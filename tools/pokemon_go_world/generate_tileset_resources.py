#!/usr/bin/env python3
"""Generate native PC map tileset identifiers and resource maps."""

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
#define EMERALD 1
#include "config/general.h"
#include "config/overworld.h"
#include "data/tilesets/graphics.h"
#include "data/tilesets/metatiles.h"

// The Emerald primary General graphics remain in src/graphics.c upstream.
// Repeat their declarations here so the PC generator externalizes the full
// active tileset set rather than leaving these two arrays in the executable.
#if !IS_FRLG
const u16 ALIGNED(4) gTilesetPalettes_General[][16] =
{
    INCGFX_U16("data/tilesets/primary/general/palettes/00.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/01.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/02.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/03.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/04.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/05.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/06.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/07.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/08.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/09.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/10.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/11.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/12.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/13.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/14.pal", ".gbapal"),
    INCGFX_U16("data/tilesets/primary/general/palettes/15.pal", ".gbapal"),
};
const u32 gTilesetTiles_General[] =
    INCGFX_U32("data/tilesets/primary/general/tiles.png", ".4bpp.smol");
#endif
"""

TILE_DECLARATION = re.compile(
    r"const\s+u32\s+(gTilesetTiles_[A-Za-z0-9_]+)\[\]\s*=\s*"
    r"INCGFX_U32\(\"([^\"]+)\",\s*\"([^\"]+)\"(?:,\s*\"([^\"]*)\")?\);"
)
PALETTE_DECLARATION = re.compile(
    r"const\s+u16\s+(?:ALIGNED\(4\)\s+)?(gTilesetPalettes_[A-Za-z0-9_]+)"
    r"\[\]\[16\]\s*=\s*\{(.*?)\};",
    re.DOTALL,
)
PALETTE_SOURCE = re.compile(
    r"INCGFX_U16\(\"([^\"]+)\",\s*\"([^\"]+)\"(?:,\s*\"([^\"]*)\")?\)"
)
METATILE_DECLARATION = re.compile(
    r"const\s+u16\s+(gMetatiles_[A-Za-z0-9_]+)\[\]\s*=\s*"
    r"INCBIN_U16\(\"([^\"]+)\"\);"
)
ATTRIBUTE_DECLARATION = re.compile(
    r"const\s+u16\s+(gMetatileAttributes_[A-Za-z0-9_]+)\[\]\s*=\s*"
    r"INCBIN_U16\(\"([^\"]+)\"\);"
)


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
        f"-I{root}",
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


def add_unique(mapping: dict[str, object], symbol: str, value: object) -> None:
    previous = mapping.get(symbol)
    if previous is not None and previous != value:
        raise ValueError(f"conflicting active definitions for {symbol}")
    mapping[symbol] = value


def arguments_as_path(arguments: str) -> str:
    return "".join(character if character.isalnum() else "_" for character in arguments)


def generated_asset(source: str, extension: str, arguments: str) -> str:
    return f"build/assets/{source}{arguments_as_path(arguments)}{extension}"


def generate(root: Path, cpp: Path, header: Path, resource_list: Path, makefile: Path) -> None:
    source_text = preprocess(root, cpp)
    tiles: dict[str, tuple[str, str, str]] = {}
    palettes: dict[str, tuple[tuple[str, str, str], ...]] = {}
    metatiles: dict[str, str] = {}
    attributes: dict[str, str] = {}

    for match in TILE_DECLARATION.finditer(source_text):
        symbol, source, extension, arguments = match.groups()
        add_unique(tiles, symbol, (source, extension, arguments or ""))
    for match in PALETTE_DECLARATION.finditer(source_text):
        symbol, body = match.groups()
        sources = tuple(PALETTE_SOURCE.findall(body))
        if not sources:
            raise ValueError(f"tileset palette has no sources: {symbol}")
        add_unique(palettes, symbol, sources)
    for match in METATILE_DECLARATION.finditer(source_text):
        add_unique(metatiles, match.group(1), match.group(2))
    for match in ATTRIBUTE_DECLARATION.finditer(source_text):
        add_unique(attributes, match.group(1), match.group(2))

    if not tiles or not palettes or not metatiles or not attributes:
        raise ValueError("one or more active tileset resource families are empty")

    resources: list[dict[str, object]] = []
    assets: set[str] = set()
    tables: dict[str, list[tuple[str, str, int]]] = {
        "tiles": [],
        "palettes": [],
        "metatiles": [],
        "attributes": [],
    }

    for symbol, (source, extension, arguments) in sorted(tiles.items()):
        if not (root / source).is_file():
            raise FileNotFoundError(f"tileset tile source does not exist: {source}")
        asset = generated_asset(source, extension, arguments)
        name = f"tilesets/tiles/{symbol}"
        resources.append({"name": name, "source": asset})
        tables["tiles"].append((symbol, name, 4))
        assets.add(asset)

    for symbol, sources in sorted(palettes.items()):
        generated_sources: list[str] = []
        for source, extension, arguments in sources:
            if not (root / source).is_file():
                raise FileNotFoundError(f"tileset palette source does not exist: {source}")
            asset = generated_asset(source, extension, arguments)
            generated_sources.append(asset)
            assets.add(asset)
        name = f"tilesets/palettes/{symbol}"
        resources.append({"name": name, "sources": generated_sources})
        tables["palettes"].append((symbol, name, len(generated_sources) * 32))

    for family, declarations in (("metatiles", metatiles), ("attributes", attributes)):
        for symbol, source in sorted(declarations.items()):
            path = root / source
            if not path.is_file():
                raise FileNotFoundError(f"tileset {family} source does not exist: {source}")
            name = f"tilesets/{family}/{symbol}"
            resources.append({"name": name, "source": source})
            tables[family].append((symbol, name, path.stat().st_size))
            assets.add(source)

    lines = [
        "// Generated by tools/pokemon_go_world/generate_tileset_resources.py.",
        "// Do not edit by hand.",
        "#ifndef GUARD_GENERATED_PC_TILESET_RESOURCES_H",
        "#define GUARD_GENERATED_PC_TILESET_RESOURCES_H",
        "",
        "#include \"fieldmap.h\"",
        "#include \"resource_pack.h\"",
        "#include \"tileset_resources.h\"",
        "",
    ]
    for symbol in sorted(tiles):
        lines.append(f"const u32 {symbol}[1] = {{0}};")
    lines.append("")
    for symbol in sorted(palettes):
        lines.append(f"const u16 {symbol}[1][16] = {{{{0}}}};")
    lines.append("")
    for symbol in sorted(metatiles):
        lines.append(f"const u16 {symbol}[1] = {{0}};")
    lines.append("")
    for symbol in sorted(attributes):
        lines.append(f"const u16 {symbol}[1] = {{0}};")
    lines.extend([
        "",
        "struct PcTilesetResource",
        "{",
        "    const void *compiledData;",
        "    u64 hash;",
        "    u32 minimumSize;",
        "    const void *resolvedData;",
        "    bool8 attempted;",
        "};",
        "",
    ])

    table_names = {
        "tiles": "sPcTilesetTileResources",
        "palettes": "sPcTilesetPaletteResources",
        "metatiles": "sPcTilesetMetatileResources",
        "attributes": "sPcTilesetAttributeResources",
    }
    for family in ("tiles", "palettes", "metatiles", "attributes"):
        lines.append(f"static struct PcTilesetResource {table_names[family]}[] =")
        lines.append("{")
        for symbol, name, minimum_size in tables[family]:
            lines.append(
                f"    {{ {symbol}, UINT64_C({fnv1a64(name)}), {minimum_size} }},"
            )
        lines.extend(["};", ""])

    lines.extend([
        "static const u16 sMissingTilesetMetatiles[NUM_METATILES_TOTAL * NUM_TILES_PER_METATILE] = {0};",
        "static const u32 sMissingTilesetAttributes[NUM_METATILES_TOTAL] = {0};",
        "",
        "static const void *ResolveTilesetResource(",
        "    struct PcTilesetResource *resources,",
        "    u32 resourceCount,",
        "    const void *compiledData,",
        "    const void *missingData)",
        "{",
        "    u32 i;",
        "",
        "    for (i = 0; i < resourceCount; i++)",
        "    {",
        "        struct PcTilesetResource *resource = &resources[i];",
        "        const void *data;",
        "        u64 size = 0;",
        "",
        "        if (resource->compiledData != compiledData)",
        "            continue;",
        "        if (resource->attempted)",
        "            return resource->resolvedData;",
        "        data = ResourcePack_GetByHash(resource->hash, &size);",
        "        if (data != NULL && size >= resource->minimumSize)",
        "            resource->resolvedData = data;",
        "        else",
        "            resource->resolvedData = missingData;",
        "        resource->attempted = TRUE;",
        "        if (resource->resolvedData != missingData)",
        "            return resource->resolvedData;",
        "        DBGPRINTF(\"Tileset resource: identifier %p is missing or invalid\\n\", compiledData);",
        "        return resource->resolvedData;",
        "    }",
        "",
        "    DBGPRINTF(\"Tileset resource: unknown compiled identifier %p\\n\", compiledData);",
        "    return missingData;",
        "}",
        "",
        "const u32 *ResolveTilesetTiles(const u32 *compiledData)",
        "{",
        "    return ResolveTilesetResource(sPcTilesetTileResources,",
        "        ARRAY_COUNT(sPcTilesetTileResources), compiledData, NULL);",
        "}",
        "",
        "const u16 (*ResolveTilesetPalettes(const u16 (*compiledData)[16]))[16]",
        "{",
        "    return ResolveTilesetResource(sPcTilesetPaletteResources,",
        "        ARRAY_COUNT(sPcTilesetPaletteResources), compiledData, NULL);",
        "}",
        "",
        "const u16 *ResolveTilesetMetatiles(const u16 *compiledData)",
        "{",
        "    return ResolveTilesetResource(sPcTilesetMetatileResources,",
        "        ARRAY_COUNT(sPcTilesetMetatileResources), compiledData, sMissingTilesetMetatiles);",
        "}",
        "",
        "const u16 *ResolveTilesetAttributes(const u16 *compiledData)",
        "{",
        "    return ResolveTilesetResource(sPcTilesetAttributeResources,",
        "        ARRAY_COUNT(sPcTilesetAttributeResources), compiledData, sMissingTilesetAttributes);",
        "}",
        "",
        "#endif // GUARD_GENERATED_PC_TILESET_RESOURCES_H",
        "",
    ])

    resource_document = {"format_version": 1, "resources": resources}
    make_lines = [
        "# Generated by tools/pokemon_go_world/generate_tileset_resources.py.",
        "PC_TILESET_RESOURCE_ASSETS := \\",
    ]
    sorted_assets = sorted(assets)
    for index, path in enumerate(sorted_assets):
        continuation = " \\" if index != len(sorted_assets) - 1 else ""
        make_lines.append(f"\t{path}{continuation}")
    make_lines.append("")

    write_if_changed(header, "\n".join(lines))
    write_if_changed(resource_list, json.dumps(resource_document, indent=2) + "\n")
    write_if_changed(makefile, "\n".join(make_lines))
    print(
        f"Generated {len(tiles)} tiles, {len(palettes)} palettes, "
        f"{len(metatiles)} metatiles and {len(attributes)} attribute tables"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--cpp", type=Path, required=True)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--resource-list", type=Path, required=True)
    parser.add_argument("--makefile", type=Path, required=True)
    args = parser.parse_args()
    generate(
        args.root.resolve(),
        args.cpp.resolve(),
        args.header.resolve(),
        args.resource_list.resolve(),
        args.makefile.resolve(),
    )


if __name__ == "__main__":
    main()
