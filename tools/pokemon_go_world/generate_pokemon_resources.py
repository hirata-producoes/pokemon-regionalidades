#!/usr/bin/env python3
"""Generate the PC Pokemon picture and palette resource maps."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import subprocess


WRAPPER = r"""
#define TRUE 1
#define FALSE 0
#define MODERN 1
#include "config/general.h"
#include "config/pokemon.h"
#include "config/overworld.h"
#include "data/graphics/pokemon.h"
"""

PIC_DECLARATION = re.compile(
    r"const\s+u32\s+"
    r"(gMon(?:FrontPic|BackPic|EggGfx|HatchGfx)_[A-Za-z0-9_]+)"
    r"\[\]\s*=\s*INCGFX_U32\(\"([^\"]+)\",\s*\"([^\"]+)\"\);"
)

PALETTE_DECLARATION = re.compile(
    r"const\s+u16\s+"
    r"([A-Za-z0-9_]+)"
    r"\[\]\s*=\s*INCGFX_U16\(\"([^\"]+)\",\s*\"([^\"]+)\"\);"
)

U8_DECLARATION = re.compile(
    r"const\s+u8\s+"
    r"((?:gMonIcon|gMonEggIcon|gMonFootprint)_[A-Za-z0-9_]+)"
    r"\[\]\s*=\s*INCGFX_U8\(\"([^\"]+)\",\s*\"([^\"]+)\"\);"
)

FRONT_FALLBACK = "graphics/pokemon/pics/gMonFrontPic_CircledQuestionMark"
BACK_FALLBACK = "graphics/pokemon/pics/gMonBackPic_CircledQuestionMark"
PALETTE_FALLBACK = "graphics/pokemon/palettes/gMonPalette_CircledQuestionMark"
SHINY_PALETTE_FALLBACK = "graphics/pokemon/palettes/gMonShinyPalette_CircledQuestionMark"
ICON_FALLBACK = "graphics/pokemon/icons/gMonIcon_QuestionMark"
EGG_ICON_FALLBACK = "graphics/pokemon/icons/gMonIcon_Egg"
FOOTPRINT_FALLBACK = "graphics/pokemon/footprints/gMonFootprint_QuestionMark"


def fnv1a64(value: str) -> int:
    result = 0xCBF29CE484222325
    for byte in value.encode("utf-8"):
        result ^= byte
        result = (result * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return result


def write_if_changed(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and path.read_text(encoding="utf-8") == content:
        path.touch()
        return
    path.write_text(content, encoding="utf-8", newline="\n")


def preprocess(root: Path, cpp: Path) -> str:
    command = [
        str(cpp),
        "-P",
        f"-I{root / 'include'}",
        f"-I{root / 'src'}",
        "-",
    ]
    result = subprocess.run(
        command,
        cwd=root,
        input=WRAPPER,
        text=True,
        encoding="utf-8",
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(f"C preprocessor failed ({result.returncode}):\n{result.stderr}")
    return result.stdout


def fallback_for(symbol: str) -> str | None:
    if symbol.startswith("gMonBackPic_"):
        return BACK_FALLBACK
    if symbol.startswith("gMonFrontPic_") or symbol.startswith("gMonEggGfx_"):
        return FRONT_FALLBACK
    return None


def palette_fallback_for(symbol: str) -> str:
    if "Shiny" in symbol:
        return SHINY_PALETTE_FALLBACK
    return PALETTE_FALLBACK


def u8_resource_family(symbol: str) -> str:
    if symbol.startswith("gMonFootprint_"):
        return "footprints"
    return "icons"


def u8_fallback_for(symbol: str) -> str:
    if symbol.startswith("gMonEggIcon_"):
        return EGG_ICON_FALLBACK
    if symbol.startswith("gMonFootprint_"):
        return FOOTPRINT_FALLBACK
    return ICON_FALLBACK


def generate(root: Path, cpp: Path, header: Path, resource_list: Path, makefile: Path) -> None:
    preprocessed = preprocess(root, cpp)
    pic_declarations: dict[str, tuple[str, str]] = {}
    for match in PIC_DECLARATION.finditer(preprocessed):
        symbol, source, extension = match.groups()
        value = (source, extension)
        if symbol in pic_declarations and pic_declarations[symbol] != value:
            raise ValueError(f"conflicting active definitions for {symbol}")
        pic_declarations[symbol] = value

    palette_declarations: dict[str, tuple[str, str]] = {}
    for match in PALETTE_DECLARATION.finditer(preprocessed):
        symbol, source, extension = match.groups()
        value = (source, extension)
        if symbol in palette_declarations and palette_declarations[symbol] != value:
            raise ValueError(f"conflicting active definitions for {symbol}")
        palette_declarations[symbol] = value

    u8_declarations: dict[str, tuple[str, str]] = {}
    for match in U8_DECLARATION.finditer(preprocessed):
        symbol, source, extension = match.groups()
        value = (source, extension)
        if symbol in u8_declarations and u8_declarations[symbol] != value:
            raise ValueError(f"conflicting active definitions for {symbol}")
        u8_declarations[symbol] = value

    if FRONT_FALLBACK.rsplit("/", 1)[1] not in pic_declarations:
        raise ValueError("front picture fallback is missing from active graphics")
    if BACK_FALLBACK.rsplit("/", 1)[1] not in pic_declarations:
        raise ValueError("back picture fallback is missing from active graphics")
    if PALETTE_FALLBACK.rsplit("/", 1)[1] not in palette_declarations:
        raise ValueError("normal palette fallback is missing from active graphics")
    if SHINY_PALETTE_FALLBACK.rsplit("/", 1)[1] not in palette_declarations:
        raise ValueError("shiny palette fallback is missing from active graphics")
    for fallback in (ICON_FALLBACK, EGG_ICON_FALLBACK, FOOTPRINT_FALLBACK):
        if fallback.rsplit("/", 1)[1] not in u8_declarations:
            raise ValueError(f"u8 fallback is missing from active graphics: {fallback}")

    resources: list[dict[str, str]] = []
    pic_entries: list[tuple[str, str, str | None]] = []
    palette_entries: list[tuple[str, str, str]] = []
    icon_entries: list[tuple[str, str, str]] = []
    footprint_entries: list[tuple[str, str, str]] = []
    asset_paths: set[str] = set()
    for symbol, (source, extension) in sorted(pic_declarations.items()):
        name = f"graphics/pokemon/pics/{symbol}"
        generated_source = f"build/assets/{source}{extension}"
        if not (root / source).is_file():
            raise FileNotFoundError(f"Pokemon picture source does not exist: {root / source}")
        resources.append({"name": name, "source": generated_source})
        pic_entries.append((symbol, name, fallback_for(symbol)))
        asset_paths.add(generated_source)

    for symbol, (source, extension) in sorted(palette_declarations.items()):
        name = f"graphics/pokemon/palettes/{symbol}"
        generated_source = f"build/assets/{source}{extension}"
        if not (root / source).is_file():
            raise FileNotFoundError(f"Pokemon palette source does not exist: {root / source}")
        resources.append({"name": name, "source": generated_source})
        palette_entries.append((symbol, name, palette_fallback_for(symbol)))
        asset_paths.add(generated_source)

    for symbol, (source, extension) in sorted(u8_declarations.items()):
        family = u8_resource_family(symbol)
        name = f"graphics/pokemon/{family}/{symbol}"
        generated_source = f"build/assets/{source}{extension}"
        if not (root / source).is_file():
            raise FileNotFoundError(f"Pokemon {family} source does not exist: {root / source}")
        resources.append({"name": name, "source": generated_source})
        entry = (symbol, name, u8_fallback_for(symbol))
        if family == "icons":
            icon_entries.append(entry)
        else:
            footprint_entries.append(entry)
        asset_paths.add(generated_source)

    header_lines = [
        "// Generated by tools/pokemon_go_world/generate_pokemon_resources.py.",
        "// Do not edit by hand.",
        "#ifndef GUARD_GENERATED_PC_POKEMON_RESOURCES_H",
        "#define GUARD_GENERATED_PC_POKEMON_RESOURCES_H",
        "",
        "#include \"pokemon_resources.h\"",
        "#include \"resource_pack.h\"",
        "",
        "struct PcPokemonPicResource",
        "{",
        "    const void *compiledData;",
        "    u64 hash;",
        "    u64 fallbackHash;",
        "};",
        "",
        "static const struct PcPokemonPicResource sPcPokemonPicResources[] =",
        "{",
    ]
    for symbol, name, fallback in pic_entries:
        fallback_hash = fnv1a64(fallback) if fallback is not None else 0
        header_lines.append(
            f"    {{ {symbol}, UINT64_C({fnv1a64(name)}), UINT64_C({fallback_hash}) }},"
        )
    header_lines.extend([
        "};",
        "",
        "struct PcPokemonCachedResource",
        "{",
        "    const void *compiledData;",
        "    u64 hash;",
        "    u64 fallbackHash;",
        "};",
        "",
        "static const struct PcPokemonCachedResource sPcPokemonPaletteResources[] =",
        "{",
    ])
    for symbol, name, fallback in palette_entries:
        header_lines.append(
            f"    {{ {symbol}, UINT64_C({fnv1a64(name)}), UINT64_C({fnv1a64(fallback)}) }},"
        )
    header_lines.extend([
        "};",
        "",
        "static const struct PcPokemonCachedResource sPcPokemonIconResources[] =",
        "{",
    ])
    for symbol, name, fallback in icon_entries:
        header_lines.append(
            f"    {{ {symbol}, UINT64_C({fnv1a64(name)}), UINT64_C({fnv1a64(fallback)}) }},"
        )
    header_lines.extend([
        "};",
        "",
        "static const struct PcPokemonCachedResource sPcPokemonFootprintResources[] =",
        "{",
    ])
    for symbol, name, fallback in footprint_entries:
        header_lines.append(
            f"    {{ {symbol}, UINT64_C({fnv1a64(name)}), UINT64_C({fnv1a64(fallback)}) }},"
        )
    header_lines.extend([
        "};",
        "",
        "static const u16 sMissingPokemonPalette[16] = {0};",
        "static const u8 sMissingPokemonIcon[0x400] = {0};",
        "static const u8 sMissingPokemonFootprint[32] = {0};",
        "",
        "void *LoadExternalPokemonPic(const void *compiledData, u64 *sizeOut)",
        "{",
        "    u32 i;",
        "",
        "    for (i = 0; i < ARRAY_COUNT(sPcPokemonPicResources); i++)",
        "    {",
        "        const struct PcPokemonPicResource *resource = &sPcPokemonPicResources[i];",
        "        void *data;",
        "",
        "        if (resource->compiledData != compiledData)",
        "            continue;",
        "        data = ResourcePack_LoadByHash(resource->hash, sizeOut);",
        "        if (data == NULL && resource->fallbackHash != 0",
        "         && resource->hash != resource->fallbackHash)",
        "            data = ResourcePack_LoadByHash(resource->fallbackHash, sizeOut);",
        "        return data;",
        "    }",
        "    if (sizeOut != NULL)",
        "        *sizeOut = 0;",
        "    DBGPRINTF(\"Pokemon resource: unknown compiled picture identifier %p\\n\", compiledData);",
        "    return NULL;",
        "}",
        "",
        "static const void *GetExternalPokemonCachedResource(",
        "    const struct PcPokemonCachedResource *resources,",
        "    u32 resourceCount,",
        "    const void *compiledData,",
        "    u64 minimumSize,",
        "    const void *missingData)",
        "{",
        "    u32 i;",
        "",
        "    for (i = 0; i < resourceCount; i++)",
        "    {",
        "        const struct PcPokemonCachedResource *resource = &resources[i];",
        "        const void *data;",
        "        u64 size = 0;",
        "",
        "        if (resource->compiledData != compiledData)",
        "            continue;",
        "        data = ResourcePack_GetByHash(resource->hash, &size);",
        "        if ((data == NULL || size < minimumSize)",
        "         && resource->hash != resource->fallbackHash)",
        "            data = ResourcePack_GetByHash(resource->fallbackHash, &size);",
        "        if (data != NULL && size >= minimumSize)",
        "            return data;",
        "        return missingData;",
        "    }",
        "    DBGPRINTF(\"Pokemon resource: unknown cached identifier %p\\n\", compiledData);",
        "    return missingData;",
        "}",
        "",
        "const u16 *GetExternalPokemonPalette(const void *compiledData)",
        "{",
        "    return GetExternalPokemonCachedResource(",
        "        sPcPokemonPaletteResources, ARRAY_COUNT(sPcPokemonPaletteResources),",
        "        compiledData, sizeof(sMissingPokemonPalette), sMissingPokemonPalette);",
        "}",
        "",
        "const u8 *GetExternalPokemonIcon(const void *compiledData)",
        "{",
        "    return GetExternalPokemonCachedResource(",
        "        sPcPokemonIconResources, ARRAY_COUNT(sPcPokemonIconResources),",
        "        compiledData, sizeof(sMissingPokemonIcon), sMissingPokemonIcon);",
        "}",
        "",
        "const u8 *GetExternalPokemonFootprint(const void *compiledData)",
        "{",
        "    return GetExternalPokemonCachedResource(",
        "        sPcPokemonFootprintResources, ARRAY_COUNT(sPcPokemonFootprintResources),",
        "        compiledData, sizeof(sMissingPokemonFootprint), sMissingPokemonFootprint);",
        "}",
        "",
        "#endif // GUARD_GENERATED_PC_POKEMON_RESOURCES_H",
        "",
    ])

    resource_document = {
        "format_version": 1,
        "resources": resources,
    }
    make_lines = [
        "# Generated by tools/pokemon_go_world/generate_pokemon_resources.py.",
        "PC_POKEMON_RESOURCE_ASSETS := \\",
    ]
    sorted_asset_paths = sorted(asset_paths)
    for index, path in enumerate(sorted_asset_paths):
        continuation = " \\" if index != len(sorted_asset_paths) - 1 else ""
        make_lines.append(f"\t{path}{continuation}")
    make_lines.append("")

    write_if_changed(header, "\n".join(header_lines))
    write_if_changed(resource_list, json.dumps(resource_document, indent=2) + "\n")
    write_if_changed(makefile, "\n".join(make_lines))
    print(
        f"Generated {len(pic_entries)} active Pokemon pictures and "
        f"{len(palette_entries)} palettes, {len(icon_entries)} icons and "
        f"{len(footprint_entries)} footprints"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--cpp", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--resource-list", type=Path, required=True)
    parser.add_argument("--makefile", type=Path, required=True)
    args = parser.parse_args()
    root = args.root.resolve()
    generate(root, args.cpp.resolve(), args.header.resolve(),
             args.resource_list.resolve(), args.makefile.resolve())


if __name__ == "__main__":
    main()
