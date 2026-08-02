#!/usr/bin/env python3
"""Convert native i386 COFF MP2K song objects into relocatable PC resources."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
from pathlib import Path
import re
import struct


COFF_HEADER = struct.Struct("<HHIIIHH")
SECTION_HEADER = struct.Struct("<8sIIIIIIHHI")
SYMBOL_ENTRY = struct.Struct("<8sIhHBB")
RELOCATION = struct.Struct("<IIH")

COFF_I386 = 0x014C
COFF_RELOC_DIR32 = 0x0006
SONG_MAGIC = b"PGWSONG\0"
SONG_VERSION = 1
SONG_HEADER = struct.Struct("<8sIIII")
SONG_RELOCATION = struct.Struct("<IIQ")
RELOCATION_INTERNAL = 0
RELOCATION_EXTERNAL = 1


@dataclass(frozen=True)
class CoffSection:
    name: str
    number: int
    data: bytes
    relocation_offset: int
    relocation_count: int


@dataclass(frozen=True)
class CoffSymbol:
    name: str
    value: int
    section_number: int


@dataclass(frozen=True)
class SongRelocation:
    offset: int
    kind: int
    target: int


@dataclass(frozen=True)
class SongResource:
    label: str
    header_offset: int
    data: bytes
    relocations: tuple[SongRelocation, ...]
    external_symbols: tuple[str, ...]


def fnv1a64(value: str) -> int:
    result = 0xCBF29CE484222325
    for byte in value.encode("utf-8"):
        result ^= byte
        result = (result * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return result


def write_text_if_changed(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and path.read_text(encoding="utf-8") == content:
        path.touch()
        return
    path.write_text(content, encoding="utf-8", newline="\n")


def write_bytes_if_changed(path: Path, content: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and path.read_bytes() == content:
        path.touch()
        return
    path.write_bytes(content)


def read_c_string(data: bytes, offset: int) -> str:
    if offset < 0 or offset >= len(data):
        raise ValueError(f"string offset is outside COFF table: {offset}")
    end = data.find(b"\0", offset)
    if end < 0:
        raise ValueError("unterminated COFF string")
    return data[offset:end].decode("ascii")


def decode_coff_name(raw_name: bytes, string_table: bytes) -> str:
    zeroes, offset = struct.unpack("<II", raw_name)
    if zeroes == 0:
        if offset < 4:
            raise ValueError(f"invalid COFF string-table offset: {offset}")
        return read_c_string(string_table, offset - 4)
    return raw_name.split(b"\0", 1)[0].decode("ascii")


def normalize_symbol(name: str) -> str:
    if name.startswith("_") and not name.startswith("_."):
        return name[1:]
    return name


def parse_song_object(path: Path) -> SongResource:
    file_data = path.read_bytes()
    if len(file_data) < COFF_HEADER.size:
        raise ValueError(f"COFF file is too small: {path}")
    (
        machine,
        section_count,
        _,
        symbol_table_offset,
        symbol_count,
        optional_header_size,
        _,
    ) = COFF_HEADER.unpack_from(file_data)
    if machine != COFF_I386:
        raise ValueError(f"unsupported COFF machine 0x{machine:04x}: {path}")
    if optional_header_size != 0:
        raise ValueError(f"unexpected optional header in object: {path}")

    symbol_table_end = symbol_table_offset + symbol_count * SYMBOL_ENTRY.size
    if symbol_table_end + 4 > len(file_data):
        raise ValueError(f"invalid COFF symbol table: {path}")
    string_table_size = struct.unpack_from("<I", file_data, symbol_table_end)[0]
    if string_table_size < 4 or symbol_table_end + string_table_size > len(file_data):
        raise ValueError(f"invalid COFF string table: {path}")
    string_table = file_data[symbol_table_end + 4:symbol_table_end + string_table_size]

    sections: list[CoffSection] = []
    section_headers_offset = COFF_HEADER.size + optional_header_size
    for index in range(section_count):
        offset = section_headers_offset + index * SECTION_HEADER.size
        if offset + SECTION_HEADER.size > len(file_data):
            raise ValueError(f"truncated COFF section table: {path}")
        (
            raw_name,
            _,
            _,
            raw_size,
            raw_offset,
            relocation_offset,
            _,
            relocation_count,
            _,
            _,
        ) = SECTION_HEADER.unpack_from(file_data, offset)
        name = decode_coff_name(raw_name, string_table)
        if raw_offset + raw_size > len(file_data):
            raise ValueError(f"section {name} exceeds COFF file: {path}")
        sections.append(CoffSection(
            name=name,
            number=index + 1,
            data=file_data[raw_offset:raw_offset + raw_size],
            relocation_offset=relocation_offset,
            relocation_count=relocation_count,
        ))

    symbols: list[CoffSymbol | None] = [None] * symbol_count
    index = 0
    while index < symbol_count:
        offset = symbol_table_offset + index * SYMBOL_ENTRY.size
        raw_name, value, section_number, _, _, aux_count = SYMBOL_ENTRY.unpack_from(file_data, offset)
        symbols[index] = CoffSymbol(
            name=decode_coff_name(raw_name, string_table),
            value=value,
            section_number=section_number,
        )
        if index + aux_count >= symbol_count:
            raise ValueError(f"invalid auxiliary symbol count in {path}")
        index += 1 + aux_count

    rodata = next((section for section in sections if section.name == ".rodata"), None)
    if rodata is None or not rodata.data:
        raise ValueError(f"song object has no .rodata: {path}")

    label = path.stem
    expected_symbol = f"_{label}"
    header_symbol = next(
        (
            symbol for symbol in symbols
            if symbol is not None
            and symbol.name == expected_symbol
            and symbol.section_number == rodata.number
        ),
        None,
    )
    if header_symbol is None:
        raise ValueError(f"song header symbol {expected_symbol} not found in {path}")
    if header_symbol.value + 8 > len(rodata.data):
        raise ValueError(f"song header is outside .rodata: {path}")

    mutable_data = bytearray(rodata.data)
    relocations: list[SongRelocation] = []
    external_symbols: set[str] = set()
    for relocation_index in range(rodata.relocation_count):
        offset = rodata.relocation_offset + relocation_index * RELOCATION.size
        if offset + RELOCATION.size > len(file_data):
            raise ValueError(f"truncated relocation table: {path}")
        virtual_address, symbol_index, relocation_type = RELOCATION.unpack_from(file_data, offset)
        if relocation_type != COFF_RELOC_DIR32:
            raise ValueError(
                f"unsupported relocation type 0x{relocation_type:04x} at "
                f"0x{virtual_address:x}: {path}"
            )
        if virtual_address + 4 > len(mutable_data):
            raise ValueError(f"relocation is outside .rodata: {path}")
        if symbol_index >= len(symbols) or symbols[symbol_index] is None:
            raise ValueError(f"relocation references invalid symbol {symbol_index}: {path}")
        symbol = symbols[symbol_index]
        assert symbol is not None
        addend = struct.unpack_from("<I", mutable_data, virtual_address)[0]
        if symbol.section_number == rodata.number:
            target = symbol.value + addend
            if target >= len(mutable_data):
                raise ValueError(f"internal relocation target is outside .rodata: {path}")
            relocation = SongRelocation(virtual_address, RELOCATION_INTERNAL, target)
        elif symbol.section_number == 0:
            external_name = normalize_symbol(symbol.name)
            if not external_name.startswith("voicegroup_"):
                raise ValueError(f"unsupported external song symbol {symbol.name}: {path}")
            if addend != 0:
                raise ValueError(f"external song relocation has nonzero addend: {path}")
            external_symbols.add(external_name)
            relocation = SongRelocation(
                virtual_address,
                RELOCATION_EXTERNAL,
                fnv1a64(external_name),
            )
        else:
            raise ValueError(
                f"relocation references unsupported section {symbol.section_number}: {path}"
            )
        struct.pack_into("<I", mutable_data, virtual_address, 0)
        relocations.append(relocation)

    track_count = mutable_data[header_symbol.value]
    if track_count > 16:
        raise ValueError(f"song declares {track_count} tracks: {path}")
    if header_symbol.value + 8 + track_count * 4 > len(mutable_data):
        raise ValueError(f"song header track table exceeds .rodata: {path}")

    return SongResource(
        label=label,
        header_offset=header_symbol.value,
        data=bytes(mutable_data),
        relocations=tuple(relocations),
        external_symbols=tuple(sorted(external_symbols)),
    )


def encode_song_resource(song: SongResource) -> bytes:
    header = SONG_HEADER.pack(
        SONG_MAGIC,
        SONG_VERSION,
        len(song.data),
        song.header_offset,
        len(song.relocations),
    )
    relocation_data = b"".join(
        SONG_RELOCATION.pack(relocation.offset, relocation.kind, relocation.target)
        for relocation in song.relocations
    )
    return header + relocation_data + song.data


def generate(
    root: Path,
    object_dir: Path,
    output_dir: Path,
    header: Path,
    placeholders: Path,
    resource_list: Path,
    makefile: Path,
) -> None:
    object_paths = sorted(object_dir.glob("*.o"))
    if not object_paths:
        raise ValueError(f"no song objects found in {object_dir}")
    songs = [parse_song_object(path) for path in object_paths]
    labels = [song.label for song in songs]
    if len(labels) != len(set(labels)):
        raise ValueError("duplicate song labels")

    all_external_symbols = sorted({
        symbol
        for song in songs
        for symbol in song.external_symbols
    })
    output_dir.mkdir(parents=True, exist_ok=True)
    resources: list[dict[str, str]] = []
    total_data_size = 0
    total_relocations = 0
    for song in songs:
        output_path = output_dir / f"{song.label}.pgwsong"
        encoded = encode_song_resource(song)
        write_bytes_if_changed(output_path, encoded)
        relative_output = output_path.relative_to(root).as_posix()
        resources.append({
            "name": f"songs/{song.label}",
            "source": relative_output,
        })
        total_data_size += len(song.data)
        total_relocations += len(song.relocations)

    placeholder_lines = [
        "# Generated by tools/pokemon_go_world/generate_song_resources.py.",
        "# Do not edit by hand.",
        "",
        "\t.section .rodata",
        "",
    ]
    for song in songs:
        placeholder_lines.extend([
            f"\t.global {song.label}",
            "\t.align 2",
            f"{song.label}:",
            "\t.space 8",
            "",
        ])

    header_lines = [
        "// Generated by tools/pokemon_go_world/generate_song_resources.py.",
        "// Do not edit by hand.",
        "#ifndef GUARD_GENERATED_PC_SONG_RESOURCES_H",
        "#define GUARD_GENERATED_PC_SONG_RESOURCES_H",
        "",
        "#include <string.h>",
        "#include \"resource_pack.h\"",
        "",
    ]
    for song in songs:
        header_lines.append(f"extern const u8 {song.label}[];")
    header_lines.append("")
    for symbol in all_external_symbols:
        if symbol == "voicegroup_dummy":
            # m4a_internal.h exposes this single voicegroup as a ToneData object.
            # All other voicegroups are assembler byte arrays without C declarations.
            continue
        header_lines.append(f"extern const u8 {symbol}[];")
    header_lines.extend([
        "",
        "#define PC_SONG_RELOCATION_INTERNAL 0",
        "#define PC_SONG_RELOCATION_EXTERNAL 1",
        "",
        "struct PcSongResourceHeader",
        "{",
        "    u8 magic[8];",
        "    u32 version;",
        "    u32 dataSize;",
        "    u32 songHeaderOffset;",
        "    u32 relocationCount;",
        "};",
        "",
        "struct PcSongRelocation",
        "{",
        "    u32 offset;",
        "    u32 kind;",
        "    u64 target;",
        "};",
        "",
        "STATIC_ASSERT(sizeof(struct PcSongResourceHeader) == 24, PcSongResourceHeaderSize);",
        "STATIC_ASSERT(sizeof(struct PcSongRelocation) == 16, PcSongRelocationSize);",
        "",
        "struct PcSongResource",
        "{",
        "    const struct SongHeader *compiledData;",
        "    u64 hash;",
        "    struct SongHeader *resolvedData;",
        "    struct SongHeader fallback;",
        "    bool8 attempted;",
        "    bool8 reportedInvalid;",
        "};",
        "",
        "struct PcSongExternalSymbol",
        "{",
        "    u64 hash;",
        "    const void *address;",
        "};",
        "",
        "static struct PcSongResource sPcSongResources[] =",
        "{",
    ])
    for song in songs:
        header_lines.append(
            f"    {{ (const struct SongHeader *){song.label}, "
            f"UINT64_C({fnv1a64(f'songs/{song.label}')}) }},"
        )
    header_lines.extend([
        "};",
        "",
        "static const struct PcSongExternalSymbol sPcSongExternalSymbols[] =",
        "{",
    ])
    for symbol in all_external_symbols:
        address = f"&{symbol}" if symbol == "voicegroup_dummy" else symbol
        header_lines.append(
            f"    {{ UINT64_C({fnv1a64(symbol)}), {address} }},"
        )
    header_lines.extend([
        "};",
        "",
        "static const u8 sPcSongMagic[8] = {'P', 'G', 'W', 'S', 'O', 'N', 'G', 0};",
        "",
        "static const void *ResolvePcSongExternalSymbol(u64 hash)",
        "{",
        "    return ResolveVoicegroupByHash(hash);",
        "}",
        "",
        "static struct SongHeader *LoadPcSongResource(struct PcSongResource *resource)",
        "{",
        "    u64 resourceSize = 0;",
        "    u8 *blob = (u8 *)ResourcePack_GetByHash(resource->hash, &resourceSize);",
        "    struct PcSongResourceHeader resourceHeader;",
        "    u64 dataOffset;",
        "    u8 *songData;",
        "    u32 i;",
        "",
        "    if (blob == NULL || resourceSize < sizeof(resourceHeader))",
        "        return NULL;",
        "    memcpy(&resourceHeader, blob, sizeof(resourceHeader));",
        "    if (memcmp(resourceHeader.magic, sPcSongMagic, sizeof(sPcSongMagic)) != 0",
        "     || resourceHeader.version != 1",
        "     || resourceHeader.relocationCount > (resourceSize - sizeof(resourceHeader)) / sizeof(struct PcSongRelocation))",
        "        return NULL;",
        "    dataOffset = sizeof(resourceHeader)",
        "        + (u64)resourceHeader.relocationCount * sizeof(struct PcSongRelocation);",
        "    if (resourceHeader.dataSize > resourceSize - dataOffset",
        "     || resourceHeader.songHeaderOffset > resourceHeader.dataSize",
        "     || resourceHeader.dataSize - resourceHeader.songHeaderOffset < 8)",
        "        return NULL;",
        "    songData = blob + dataOffset;",
        "",
        "    for (i = 0; i < resourceHeader.relocationCount; i++)",
        "    {",
        "        struct PcSongRelocation relocation;",
        "        const u8 *record = blob + sizeof(resourceHeader) + i * sizeof(relocation);",
        "        const void *target;",
        "        u32 pointerValue;",
        "",
        "        memcpy(&relocation, record, sizeof(relocation));",
        "        if (relocation.offset > resourceHeader.dataSize",
        "         || resourceHeader.dataSize - relocation.offset < sizeof(pointerValue))",
        "            return NULL;",
        "        if (relocation.kind == PC_SONG_RELOCATION_INTERNAL)",
        "        {",
        "            if (relocation.target >= resourceHeader.dataSize)",
        "                return NULL;",
        "            target = songData + (u32)relocation.target;",
        "        }",
        "        else if (relocation.kind == PC_SONG_RELOCATION_EXTERNAL)",
        "        {",
        "            target = ResolvePcSongExternalSymbol(relocation.target);",
        "            if (target == NULL)",
        "                return NULL;",
        "        }",
        "        else",
        "        {",
        "            return NULL;",
        "        }",
        "        pointerValue = (u32)target;",
        "        memcpy(songData + relocation.offset, &pointerValue, sizeof(pointerValue));",
        "    }",
        "",
        "    resource->resolvedData = (struct SongHeader *)(songData + resourceHeader.songHeaderOffset);",
        "    if (resource->resolvedData->trackCount > MAX_MUSICPLAYER_TRACKS",
        "     || 8 + resource->resolvedData->trackCount * sizeof(u8 *)",
        "        > resourceHeader.dataSize - resourceHeader.songHeaderOffset)",
        "        return NULL;",
        "    return resource->resolvedData;",
        "}",
        "",
        "struct SongHeader *ResolveSongHeader(struct SongHeader *compiledData)",
        "{",
        "    u32 i;",
        "",
        "    for (i = 0; i < ARRAY_COUNT(sPcSongResources); i++)",
        "    {",
        "        struct PcSongResource *resource = &sPcSongResources[i];",
        "",
        "        if (resource->resolvedData == compiledData)",
        "            return compiledData;",
        "        if (resource->compiledData != compiledData)",
        "            continue;",
        "        if (!resource->attempted)",
        "        {",
        "            resource->attempted = TRUE;",
        "            if (LoadPcSongResource(resource) == NULL)",
        "                resource->resolvedData = &resource->fallback;",
        "        }",
        "        if (resource->resolvedData == &resource->fallback && !resource->reportedInvalid)",
        "        {",
        "            DBGPRINTF(\"Song resource: identifier %p is missing or invalid\\n\", compiledData);",
        "            resource->reportedInvalid = TRUE;",
        "        }",
        "        return resource->resolvedData;",
        "    }",
        "",
        "    return compiledData;",
        "}",
        "",
        "#endif // GUARD_GENERATED_PC_SONG_RESOURCES_H",
        "",
    ])

    resource_document = {"format_version": 1, "resources": resources}
    make_lines = [
        "# Generated by tools/pokemon_go_world/generate_song_resources.py.",
        "PC_SONG_RESOURCE_ASSETS := \\",
    ]
    for index, resource in enumerate(resources):
        continuation = " \\" if index != len(resources) - 1 else ""
        make_lines.append(f"\t{resource['source']}{continuation}")
    make_lines.append("")
    write_text_if_changed(placeholders, "\n".join(placeholder_lines))
    write_text_if_changed(header, "\n".join(header_lines))
    write_text_if_changed(resource_list, json.dumps(resource_document, indent=2) + "\n")
    write_text_if_changed(makefile, "\n".join(make_lines))
    print(
        f"Generated {len(songs)} songs ({total_data_size} bytes, "
        f"{total_relocations} relocations, {len(all_external_symbols)} external symbols)"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--object-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--placeholders", type=Path, required=True)
    parser.add_argument("--resource-list", type=Path, required=True)
    parser.add_argument("--makefile", type=Path, required=True)
    args = parser.parse_args()
    generate(
        args.root.resolve(),
        args.object_dir.resolve(),
        args.output_dir.resolve(),
        args.header.resolve(),
        args.placeholders.resolve(),
        args.resource_list.resolve(),
        args.makefile.resolve(),
    )


if __name__ == "__main__":
    main()
