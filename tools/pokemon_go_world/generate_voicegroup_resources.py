#!/usr/bin/env python3
"""Convert the native i386 MP2K voicegroup object into relocatable PC resources."""

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
COFF_RELOCATION = struct.Struct("<IIH")
RESOURCE_HEADER = struct.Struct("<8sIIII")
RESOURCE_RELOCATION = struct.Struct("<IIQQ")

COFF_I386 = 0x014C
COFF_RELOC_DIR32 = 0x0006
MAGIC = b"PGWVOICE"
VERSION = 1
RELOCATION_VOICEGROUP = 0
RELOCATION_EXTERNAL = 1
TONE_DATA_SIZE = 12
MAX_VOICES = 128


@dataclass(frozen=True)
class Section:
    name: str
    number: int
    data: bytes
    relocation_offset: int
    relocation_count: int


@dataclass(frozen=True)
class Symbol:
    name: str
    value: int
    section_number: int


@dataclass(frozen=True)
class Relocation:
    offset: int
    kind: int
    target_hash: int
    addend: int


@dataclass(frozen=True)
class Voicegroup:
    name: str
    data: bytes
    relocations: tuple[Relocation, ...]


def fnv1a64(value: str) -> int:
    result = 0xCBF29CE484222325
    for byte in value.encode("utf-8"):
        result ^= byte
        result = (result * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return result


def normalize_symbol(name: str) -> str:
    if name.startswith("_") and not name.startswith("_."):
        return name[1:]
    return name


def read_c_string(data: bytes, offset: int) -> str:
    if offset < 0 or offset >= len(data):
        raise ValueError(f"COFF string offset is invalid: {offset}")
    end = data.find(b"\0", offset)
    if end < 0:
        raise ValueError("unterminated COFF string")
    return data[offset:end].decode("ascii")


def decode_name(raw: bytes, strings: bytes) -> str:
    zeroes, offset = struct.unpack("<II", raw)
    if zeroes == 0:
        if offset < 4:
            raise ValueError(f"invalid COFF string-table offset: {offset}")
        return read_c_string(strings, offset - 4)
    return raw.split(b"\0", 1)[0].decode("ascii")


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


def parse_coff(path: Path) -> tuple[bytes, Section, list[Symbol | None], list[tuple[int, int, int]]]:
    file_data = path.read_bytes()
    if len(file_data) < COFF_HEADER.size:
        raise ValueError(f"COFF object is too small: {path}")
    machine, section_count, _, symbol_offset, symbol_count, optional_size, _ = COFF_HEADER.unpack_from(file_data)
    if machine != COFF_I386 or optional_size != 0:
        raise ValueError(f"unsupported COFF object: {path}")
    symbol_end = symbol_offset + symbol_count * SYMBOL_ENTRY.size
    if symbol_end + 4 > len(file_data):
        raise ValueError(f"invalid COFF symbol table: {path}")
    string_size = struct.unpack_from("<I", file_data, symbol_end)[0]
    if string_size < 4 or symbol_end + string_size > len(file_data):
        raise ValueError(f"invalid COFF string table: {path}")
    strings = file_data[symbol_end + 4:symbol_end + string_size]

    sections: list[Section] = []
    for index in range(section_count):
        offset = COFF_HEADER.size + optional_size + index * SECTION_HEADER.size
        if offset + SECTION_HEADER.size > len(file_data):
            raise ValueError(f"truncated section table: {path}")
        raw_name, _, _, raw_size, raw_offset, reloc_offset, _, reloc_count, _, _ = SECTION_HEADER.unpack_from(file_data, offset)
        if raw_offset + raw_size > len(file_data):
            raise ValueError(f"section data exceeds object: {path}")
        sections.append(Section(
            decode_name(raw_name, strings), index + 1,
            file_data[raw_offset:raw_offset + raw_size], reloc_offset, reloc_count,
        ))

    symbols: list[Symbol | None] = [None] * symbol_count
    index = 0
    while index < symbol_count:
        offset = symbol_offset + index * SYMBOL_ENTRY.size
        raw_name, value, section_number, _, _, aux_count = SYMBOL_ENTRY.unpack_from(file_data, offset)
        symbols[index] = Symbol(decode_name(raw_name, strings), value, section_number)
        if index + aux_count >= symbol_count:
            raise ValueError(f"invalid auxiliary symbol count: {path}")
        index += 1 + aux_count

    rodata = next((section for section in sections if section.name == ".rodata"), None)
    if rodata is None or not rodata.data:
        raise ValueError(f"voicegroup object has no .rodata: {path}")
    relocations: list[tuple[int, int, int]] = []
    for index in range(rodata.relocation_count):
        offset = rodata.relocation_offset + index * COFF_RELOCATION.size
        if offset + COFF_RELOCATION.size > len(file_data):
            raise ValueError(f"truncated relocation table: {path}")
        relocations.append(COFF_RELOCATION.unpack_from(file_data, offset))
    return file_data, rodata, symbols, relocations


def parse_voicegroup_definitions(directory: Path) -> dict[str, int]:
    group_pattern = re.compile(r"^\s*voice_group\s+([A-Za-z0-9_]+)(?:\s*,\s*(\d+))?")
    voice_pattern = re.compile(r"^\s*voice_(?!group\b)[A-Za-z0-9_]+\b")
    definitions: dict[str, tuple[int, int, Path]] = {}
    for path in sorted(directory.rglob("*.inc")):
        current: str | None = None
        for line in path.read_text(encoding="utf-8").splitlines():
            match = group_pattern.match(line)
            if match:
                current = f"voicegroup_{match.group(1)}"
                if current in definitions:
                    raise ValueError(f"duplicate voicegroup definition: {current}")
                definitions[current] = (int(match.group(2) or 0), 0, path)
            elif current is not None and voice_pattern.match(line):
                offset, count, source = definitions[current]
                definitions[current] = (offset, count + 1, source)
    sizes: dict[str, int] = {}
    for name, (first_voice, count, source) in definitions.items():
        if count == 0 or first_voice + count > MAX_VOICES:
            raise ValueError(
                f"invalid voice range for {name} in {source}: {first_voice}+{count}"
            )
        sizes[name] = (first_voice + count) * TONE_DATA_SIZE
    return sizes


def parse_voicegroups(path: Path, definitions_dir: Path) -> tuple[list[Voicegroup], list[str]]:
    _, rodata, symbols, coff_relocations = parse_coff(path)
    sizes_by_name = parse_voicegroup_definitions(definitions_dir)
    starts_by_name = {
        normalize_symbol(symbol.name): symbol.value
        for symbol in symbols
        if symbol is not None
        and symbol.section_number == rodata.number
        and normalize_symbol(symbol.name).startswith("voicegroup_")
    }
    if not starts_by_name:
        raise ValueError("no voicegroup symbols found")
    if set(starts_by_name) != set(sizes_by_name):
        missing_definitions = sorted(set(starts_by_name) - set(sizes_by_name))
        missing_symbols = sorted(set(sizes_by_name) - set(starts_by_name))
        raise ValueError(
            f"voicegroup source/object mismatch; definitions missing={missing_definitions}, "
            f"symbols missing={missing_symbols}"
        )
    if len(set(starts_by_name.values())) != len(starts_by_name):
        raise ValueError("voicegroup aliases at the same address are unsupported")
    ranges = [
        (name, start, start + sizes_by_name[name])
        for name, start in sorted(starts_by_name.items(), key=lambda item: item[1])
    ]
    for name, start, end in ranges:
        size = end - start
        if (size <= 0 or size % TONE_DATA_SIZE != 0
                or size > MAX_VOICES * TONE_DATA_SIZE or end > len(rodata.data)):
            raise ValueError(f"invalid voicegroup size for {name}: {size}")

    names_by_start = {start: name for name, start, _ in ranges}
    external_symbols: set[str] = set()
    groups: list[Voicegroup] = []
    for owner_name, owner_start, owner_end in ranges:
        mutable = bytearray(rodata.data[owner_start:owner_end])
        relocations: list[Relocation] = []
        for virtual_address, symbol_index, relocation_type in coff_relocations:
            if not owner_start <= virtual_address < owner_end:
                continue
            if relocation_type != COFF_RELOC_DIR32:
                raise ValueError(f"unsupported COFF relocation 0x{relocation_type:04x}")
            owner_offset = virtual_address - owner_start
            if owner_offset + 4 > len(mutable):
                raise ValueError(f"relocation exceeds {owner_name}")
            if symbol_index >= len(symbols) or symbols[symbol_index] is None:
                raise ValueError(f"relocation references invalid symbol {symbol_index}")
            symbol = symbols[symbol_index]
            assert symbol is not None
            raw_addend = struct.unpack_from("<I", mutable, owner_offset)[0]
            if symbol.section_number == rodata.number:
                target = symbol.value + raw_addend
                target_name = names_by_start.get(target)
                if target_name is None:
                    raise ValueError(f"internal target 0x{target:x} is not a voicegroup label")
                relocation = Relocation(
                    owner_offset, RELOCATION_VOICEGROUP, fnv1a64(target_name), 0
                )
            elif symbol.section_number == 0:
                target_name = normalize_symbol(symbol.name)
                if target_name.startswith("voicegroup_"):
                    relocation = Relocation(
                        owner_offset, RELOCATION_VOICEGROUP,
                        fnv1a64(target_name), raw_addend,
                    )
                else:
                    external_symbols.add(target_name)
                    relocation = Relocation(
                        owner_offset, RELOCATION_EXTERNAL,
                        fnv1a64(target_name), raw_addend,
                    )
            else:
                raise ValueError(
                    f"unsupported target section {symbol.section_number} for {symbol.name}"
                )
            struct.pack_into("<I", mutable, owner_offset, 0)
            relocations.append(relocation)
        groups.append(Voicegroup(owner_name, bytes(mutable), tuple(relocations)))
    return groups, sorted(external_symbols)


def encode(group: Voicegroup) -> bytes:
    header = RESOURCE_HEADER.pack(MAGIC, VERSION, len(group.data), len(group.relocations), 0)
    relocations = b"".join(
        RESOURCE_RELOCATION.pack(item.offset, item.kind, item.target_hash, item.addend)
        for item in group.relocations
    )
    return header + relocations + group.data


def generate(root: Path, source_object: Path, definitions_dir: Path, output_dir: Path, header: Path,
             placeholders: Path, exports: Path, resource_list: Path, makefile: Path) -> None:
    groups, external_symbols = parse_voicegroups(source_object, definitions_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    resources: list[dict[str, str]] = []
    total_data = 0
    total_relocations = 0
    for group in groups:
        output = output_dir / f"{group.name}.pgwvoice"
        write_bytes_if_changed(output, encode(group))
        resources.append({
            "name": f"voicegroups/{group.name}",
            "source": output.relative_to(root).as_posix(),
        })
        total_data += len(group.data)
        total_relocations += len(group.relocations)

    placeholder_lines = [
        "@ Generated by tools/pokemon_go_world/generate_voicegroup_resources.py.",
        "@ Do not edit by hand.", "",
    ]
    for group in groups:
        placeholder_lines.extend([
            f"\t.global {group.name}", "\t.align 2", f"{group.name}:",
            f"\t.space {TONE_DATA_SIZE}", "",
        ])
    export_lines = [
        "@ Generated by tools/pokemon_go_world/generate_voicegroup_resources.py.",
        "@ Symbols referenced by external voicegroups must remain link-visible.", "",
    ]
    for symbol in external_symbols:
        export_lines.append(f"\t.global {symbol}")
    export_lines.append("")

    header_lines = [
        "// Generated by tools/pokemon_go_world/generate_voicegroup_resources.py.",
        "// Do not edit by hand.",
        "#ifndef GUARD_GENERATED_PC_VOICEGROUP_RESOURCES_H",
        "#define GUARD_GENERATED_PC_VOICEGROUP_RESOURCES_H", "",
        "#include <string.h>",
        "#include \"resource_pack.h\"", "",
    ]
    for group in groups:
        if group.name == "voicegroup_dummy":
            continue
        header_lines.append(f"extern const u8 {group.name}[];")
    header_lines.append("")
    for symbol in external_symbols:
        header_lines.append(f"extern const u8 {symbol}[];")
    header_lines.extend([
        "", "#define PC_VOICE_RELOCATION_VOICEGROUP 0",
        "#define PC_VOICE_RELOCATION_EXTERNAL 1", "",
        "struct PcVoiceResourceHeader", "{", "    u8 magic[8];", "    u32 version;",
        "    u32 dataSize;", "    u32 relocationCount;", "    u32 reserved;", "};", "",
        "struct PcVoiceRelocation", "{", "    u32 offset;", "    u32 kind;",
        "    u64 targetHash;", "    u64 addend;", "};", "",
        "STATIC_ASSERT(sizeof(struct PcVoiceResourceHeader) == 24, PcVoiceResourceHeaderSize);",
        "STATIC_ASSERT(sizeof(struct PcVoiceRelocation) == 24, PcVoiceRelocationSize);", "",
        "struct PcVoiceResource", "{", "    const void *compiledData;", "    u64 symbolHash;",
        "    u64 resourceHash;", "    struct ToneData *resolvedData;", "    bool8 attempted;",
        "    bool8 reportedInvalid;", "};", "",
        "struct PcVoiceExternalSymbol", "{", "    u64 hash;", "    const void *address;", "};", "",
        "static struct ToneData sMissingVoicegroup[128] =",
        "{",
        "    [0 ... 127] = { .type = 1, .key = 60, .squareNoiseConfig = 2 },",
        "};", "",
        "static struct PcVoiceResource sPcVoiceResources[] =", "{",
    ])
    for group in groups:
        compiled = f"&{group.name}" if group.name == "voicegroup_dummy" else group.name
        header_lines.append(
            f"    {{ {compiled}, UINT64_C({fnv1a64(group.name)}), "
            f"UINT64_C({fnv1a64(f'voicegroups/{group.name}')}) }},"
        )
    header_lines.extend(["};", "", "static const struct PcVoiceExternalSymbol sPcVoiceExternalSymbols[] =", "{"])
    for symbol in external_symbols:
        header_lines.append(f"    {{ UINT64_C({fnv1a64(symbol)}), {symbol} }},")
    header_lines.extend([
        "};", "", "static const u8 sPcVoiceMagic[8] = {'P', 'G', 'W', 'V', 'O', 'I', 'C', 'E'};", "",
        "static const void *ResolvePcVoiceExternalSymbol(u64 hash)", "{", "    u32 i;", "",
        "    for (i = 0; i < ARRAY_COUNT(sPcVoiceExternalSymbols); i++)",
        "        if (sPcVoiceExternalSymbols[i].hash == hash)",
        "            return sPcVoiceExternalSymbols[i].address;", "    return NULL;", "}", "",
        "static struct ToneData *LoadPcVoiceResource(struct PcVoiceResource *resource)", "{",
        "    u64 resourceSize = 0;", "    u8 *blob = (u8 *)ResourcePack_GetByHash(resource->resourceHash, &resourceSize);",
        "    struct PcVoiceResourceHeader resourceHeader;", "    u64 dataOffset;", "    u8 *voiceData;", "    u32 i;", "",
        "    if (blob == NULL || resourceSize < sizeof(resourceHeader))", "        return NULL;",
        "    memcpy(&resourceHeader, blob, sizeof(resourceHeader));",
        "    if (memcmp(resourceHeader.magic, sPcVoiceMagic, sizeof(sPcVoiceMagic)) != 0",
        "     || resourceHeader.version != 1 || resourceHeader.reserved != 0",
        "     || resourceHeader.relocationCount > (resourceSize - sizeof(resourceHeader)) / sizeof(struct PcVoiceRelocation))",
        "        return NULL;",
        "    dataOffset = sizeof(resourceHeader) + (u64)resourceHeader.relocationCount * sizeof(struct PcVoiceRelocation);",
        "    if (resourceHeader.dataSize > resourceSize - dataOffset || resourceHeader.dataSize == 0",
        "     || resourceHeader.dataSize % sizeof(struct ToneData) != 0",
        "     || resourceHeader.dataSize > 128 * sizeof(struct ToneData))", "        return NULL;",
        "    voiceData = blob + dataOffset;", "",
        "    for (i = 0; i < resourceHeader.relocationCount; i++)", "    {",
        "        struct PcVoiceRelocation relocation;", "        const void *target;", "        u32 pointerValue;",
        "        memcpy(&relocation, blob + sizeof(resourceHeader) + i * sizeof(relocation), sizeof(relocation));",
        "        if (relocation.offset > resourceHeader.dataSize",
        "         || resourceHeader.dataSize - relocation.offset < sizeof(pointerValue) || relocation.addend > UINT32_MAX)",
        "            return NULL;",
        "        if (relocation.kind == PC_VOICE_RELOCATION_VOICEGROUP)",
        "            target = ResolveVoicegroupByHash(relocation.targetHash);",
        "        else if (relocation.kind == PC_VOICE_RELOCATION_EXTERNAL)",
        "            target = ResolvePcVoiceExternalSymbol(relocation.targetHash);", "        else", "            return NULL;",
        "        if (target == NULL)", "            return NULL;",
        "        pointerValue = (u32)((const u8 *)target + (u32)relocation.addend);",
        "        memcpy(voiceData + relocation.offset, &pointerValue, sizeof(pointerValue));", "    }", "",
        "    resource->resolvedData = (struct ToneData *)voiceData;", "    return resource->resolvedData;", "}", "",
        "struct ToneData *ResolveVoicegroupByHash(u64 symbolHash)", "{", "    u32 i;", "",
        "    for (i = 0; i < ARRAY_COUNT(sPcVoiceResources); i++)", "    {",
        "        struct PcVoiceResource *resource = &sPcVoiceResources[i];",
        "        if (resource->symbolHash != symbolHash)", "            continue;",
        "        if (!resource->attempted)", "        {", "            resource->attempted = TRUE;",
        "            if (LoadPcVoiceResource(resource) == NULL)", "                resource->resolvedData = sMissingVoicegroup;", "        }",
        "        if (resource->resolvedData == NULL)", "            resource->resolvedData = sMissingVoicegroup;",
        "        if (resource->resolvedData == sMissingVoicegroup && !resource->reportedInvalid)", "        {",
        "            DBGPRINTF(\"Voicegroup resource: hash %llu is missing or invalid\\n\", symbolHash);",
        "            resource->reportedInvalid = TRUE;", "        }", "        return resource->resolvedData;", "    }", "",
        "    return sMissingVoicegroup;", "}", "",
        "#endif // GUARD_GENERATED_PC_VOICEGROUP_RESOURCES_H", "",
    ])

    resource_document = {"format_version": 1, "resources": resources}
    make_lines = ["# Generated by tools/pokemon_go_world/generate_voicegroup_resources.py.", "PC_VOICEGROUP_RESOURCE_ASSETS := \\"]
    for index, resource in enumerate(resources):
        continuation = " \\" if index + 1 < len(resources) else ""
        make_lines.append(f"\t{resource['source']}{continuation}")
    make_lines.append("")
    write_text_if_changed(placeholders, "\n".join(placeholder_lines))
    write_text_if_changed(exports, "\n".join(export_lines))
    write_text_if_changed(header, "\n".join(header_lines))
    write_text_if_changed(resource_list, json.dumps(resource_document, indent=2) + "\n")
    write_text_if_changed(makefile, "\n".join(make_lines))
    print(f"Generated {len(groups)} voicegroups ({total_data} bytes, {total_relocations} relocations, {len(external_symbols)} external symbols)")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--source-object", type=Path, required=True)
    parser.add_argument("--definitions-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--placeholders", type=Path, required=True)
    parser.add_argument("--exports", type=Path, required=True)
    parser.add_argument("--resource-list", type=Path, required=True)
    parser.add_argument("--makefile", type=Path, required=True)
    args = parser.parse_args()
    generate(args.root.resolve(), args.source_object.resolve(), args.definitions_dir.resolve(), args.output_dir.resolve(),
             args.header.resolve(), args.placeholders.resolve(), args.exports.resolve(),
             args.resource_list.resolve(), args.makefile.resolve())


if __name__ == "__main__":
    main()
