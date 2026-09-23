"""Troca somente as rochas marcadas nos prints de Cinnabar e Route 124 por mar.

Cada marca representa uma rocha completa de 2x2 metatiles. O script recusa
mapas inesperados ou parcialmente alterados e aceita uma segunda execução.
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import struct


ROOT = Path(__file__).resolve().parents[2]

# Coordenadas do canto superior esquerdo das rochas, em blocos de mapa.
MAPS = (
    (
        "CinnabarIsland_Frlg",
        24,
        20,
        0x112B,  # METATILE_GeneralFrlg_CalmWater, elevacao de Surf
        (
            (4, 17), (6, 17), (14, 17), (16, 17), (18, 17),
            (8, 18), (10, 18), (12, 18), (20, 18), (22, 18),
        ),
        {
            (0x110, 0x111, 0x118, 0x119),
            (0x1CB, 0x1CC, 0x1D3, 0x1D4),
        },
    ),
    (
        "Route124",
        80,
        80,
        0x1170,  # METATILE_General_CalmWater, elevacao de Surf
        (
            (9, 0), (11, 0), (7, 1), (13, 2), (22, 4),
            (14, 12), (20, 14), (24, 15), (31, 16), (22, 16),
            (29, 18),
        ),
        {
            (0x150, 0x151, 0x158, 0x159),
            (0x33B, 0x33D, 0x17D, 0x17E),
        },
    ),
)


def update_map(name: str, width: int, height: int, sea: int,
               targets: tuple[tuple[int, int], ...],
               expected_rocks: set[tuple[int, int, int, int]],
               check_only: bool) -> int:
    path = ROOT / "data" / "layouts" / name / "map.bin"
    original = path.read_bytes()
    if len(original) != width * height * 2:
        raise ValueError(f"Dimensoes inesperadas: {path}")
    modified = bytearray(original)
    changed = 0
    touched: set[tuple[int, int]] = set()
    for x, y in targets:
        if not 0 <= x < width - 1 or not 0 <= y < height - 1:
            raise ValueError(f"Coordenada fora do mapa {name}: {(x, y)}")
        cells = ((x, y), (x + 1, y), (x, y + 1), (x + 1, y + 1))
        if touched.intersection(cells):
            raise ValueError(f"Rochas marcadas sobrepostas no mapa {name}: {(x, y)}")
        touched.update(cells)
        values = tuple(struct.unpack_from("<H", original, 2 * (cy * width + cx))[0]
                       for cx, cy in cells)
        ids = tuple(value & 0x03FF for value in values)
        if values == (sea,) * 4:
            continue
        if ids not in expected_rocks:
            raise ValueError(f"Rocha diferente ou parcialmente alterada em {name} {(x, y)}: {values}")
        if check_only:
            raise ValueError(f"A rocha marcada ainda existe em {name} {(x, y)}")
        for cx, cy in cells:
            struct.pack_into("<H", modified, 2 * (cy * width + cx), sea)
            changed += 1
    if changed:
        pending = path.with_name("map.bin.pending")
        if pending.exists():
            raise FileExistsError(f"Existe uma alteracao pendente: {pending}")
        pending.write_bytes(modified)
        if pending.read_bytes() != modified:
            raise IOError(f"A verificacao do mapa preparado falhou: {pending}")
        os.replace(pending, path)
    return changed


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Confere sem alterar os mapas")
    args = parser.parse_args()
    for name, width, height, sea, targets, expected_rocks in MAPS:
        count = update_map(name, width, height, sea, targets, expected_rocks, args.check)
        print(f"{name}: {count} blocos alterados; {len(targets)} rochas marcadas verificadas")


if __name__ == "__main__":
    main()
