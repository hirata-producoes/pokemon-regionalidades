#!/usr/bin/env python3
"""Compare the planned Johto placement with Hoenn on the PC atlas grid."""

import argparse
import json
from collections import deque
from pathlib import Path


CHECK = ("AzaleaTown_hns", "Route32_hns", "Route33_hns", "Route34_hns", "IlexForest_hns", "RuinsOfAlph_Outside_hns")
HOENN_CHECK = ("Route114", "FortreeCity", "Route119", "Route120")


def read(path):
    return json.loads(path.read_text(encoding="utf-8"))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--project", type=Path, default=Path.cwd())
    args = parser.parse_args()
    dst = args.project.resolve()
    layouts = {
        layout["id"]: layout
        for layout in read(dst / "data/layouts/layouts.json")["layouts"]
    }
    maps = {}
    for group in read(dst / "data/maps/map_groups.json")["group_order"]:
        for name in read(dst / "data/maps/map_groups.json")[group]:
            data = read(dst / "data/maps" / name / "map.json")
            maps[data["id"]] = (data, layouts[data["layout"]], name)
    # A virtual atlas edge records the intended 30-tile northern move. It is
    # deliberately not a playable connection until there is real terrain.
    maps["MAP_ROUTE22"][0]["connections"].append({
        "map": "MAP_ROUTE26NORTH_HNS", "offset": -40, "direction": "left"
    })
    placements = {"MAP_CINNABAR_ISLAND": (0, 0)}
    conflicts = []
    def walk(seed):
        pending = deque((seed,))
        while pending:
            current = pending.popleft()
            data, layout, _ = maps[current]
            x, y = placements[current]
            for edge in (data.get("connections") or []) + (data.get("connections_pc") or []):
                target = edge["map"]
                if target not in maps or edge["direction"] not in ("up", "down", "left", "right"):
                    continue
                other = maps[target][1]
                offset = int(edge["offset"])
                direction = edge["direction"]
                position = {
                    "up": (x + offset, y - other["height"]),
                    "down": (x + offset, y + layout["height"]),
                    "left": (x - other["width"], y + offset),
                    "right": (x + layout["width"], y + offset),
                }[direction]
                if target not in placements:
                    placements[target] = position
                    pending.append(target)
                elif placements[target] != position:
                    conflicts.append((current, target, position, placements[target]))

    walk("MAP_CINNABAR_ISLAND")
    ecruteak_x, ecruteak_y = placements["MAP_ECRUTEAK_CITY_HNS"]
    placements["MAP_ROUTE42_HNS"] = (ecruteak_x + maps["MAP_ECRUTEAK_CITY_HNS"][1]["width"] + 37,
                                      ecruteak_y + 27)
    walk("MAP_ROUTE42_HNS")

    # Ilex Forest is entered via gates, so its atlas rectangle can be moved
    # independently of Route 34 and Azalea Town.
    route34_x, route34_y = placements["MAP_ROUTE34_HNS"]
    placements["MAP_ILEX_FOREST_HNS"] = (route34_x - 102, route34_y + 40)

    # Match the camera's ten-tile southern adjustment for every anchored
    # Johto outdoor map without changing any playable map connection.
    for map_id, (x, y) in list(placements.items()):
        if maps[map_id][0].get("region") == "REGION_JOHTO":
            placements[map_id] = (x, y + 10)

    # Temporary visual comparison: butt Route 45 against Route 46 and keep
    # Blackthorn above it. Gameplay connections remain unchanged until the
    # terrain screenshot clarifies the intended passage.
    route45 = "MAP_ROUTE45_HNS"
    route46 = "MAP_ROUTE46_HNS"
    blackthorn = "MAP_BLACKTHORN_CITY_HNS"
    delta_x = (placements[route46][0] + maps[route46][1]["width"]
               - placements[route45][0])
    for map_id in (route45, blackthorn):
        x, y = placements[map_id]
        placements[map_id] = (x + delta_x, y)

    def rect(map_id):
        x, y = placements[map_id]
        layout = maps[map_id][1]
        return x, y, x + layout["width"], y + layout["height"]

    def overlap(a, b):
        x1, y1, x2, y2 = rect(a)
        u1, v1, u2, v2 = rect(b)
        return max(0, min(x2, u2) - max(x1, u1)) * max(0, min(y2, v2) - max(y1, v1))

    print(f"Mapas posicionados: {len(placements)}; conflitos de coordenadas: {len(conflicts)}")
    unpositioned = [
        maps[map_id][2] for map_id in maps
        if maps[map_id][0].get("region") == "REGION_JOHTO"
        and maps[map_id][0].get("map_type") in ("MAP_TYPE_TOWN", "MAP_TYPE_CITY", "MAP_TYPE_ROUTE", "MAP_TYPE_OCEAN_ROUTE")
        and map_id not in placements
    ]
    print(f"Externos de Johto sem ancora: {unpositioned}")
    for name in CHECK:
        map_id = read(dst / "data/maps" / name / "map.json")["id"]
        if map_id not in placements:
            print(f"{name}: sem coordenada na grade atual")
            continue
        print(f"{name}: {rect(map_id)}")
        for hoenn_name in HOENN_CHECK:
            hoenn_id = read(dst / "data/maps" / hoenn_name / "map.json")["id"]
            if hoenn_id in placements:
                print(f"  {hoenn_name}: {overlap(map_id, hoenn_id)} tiles de sobreposicao")
            else:
                print(f"  {hoenn_name}: sem coordenada na grade atual")
    for name in ("VioletCity_hns", "Route35_hns", "Route41_hns", "TrainerHill_Courtyard_hns",
                 "EcruteakCity_hns", "Route42_hns", "Mahoganytown_hns", "Route43_hns",
                 "LakeOfRage_hns", "Route44_hns", "BlackthornCity_hns", "Route45_hns",
                 "Route46_hns"):
        map_id = read(dst / "data/maps" / name / "map.json")["id"]
        print(f"{name}: {rect(map_id)}")
    trainer = "MAP_TRAINER_HILL_COURTYARD_HNS"
    print(f"TrainerHill x Olivine: {overlap(trainer, 'MAP_OLIVINE_CITY_HNS')} tiles")
    print(f"TrainerHill x Route39: {overlap(trainer, 'MAP_ROUTE39_HNS')} tiles")
    ecruteak = "MAP_ECRUTEAK_CITY_HNS"
    route42 = "MAP_ROUTE42_HNS"
    assert placements[route42][0] - rect(ecruteak)[2] == 37
    assert "MAP_ROUTE44_HNS" in placements
    horizontal_gap = rect(route45)[0] - rect(route46)[2]
    vertical_overlap = max(0, min(rect(route45)[3], rect(route46)[3])
                           - max(rect(route45)[1], rect(route46)[1]))
    print(f"Route46 -> Route45: vazio horizontal de {horizontal_gap} tiles; "
          f"faixa vertical comum de {vertical_overlap} tiles; sem conexao caminhavel")
    print(f"Ajuste provisorio: Route45 e Blackthorn deslocados {delta_x} tiles no atlas; "
          f"Route44 x Blackthorn = {overlap('MAP_ROUTE44_HNS', blackthorn)} tiles")
    hoenn = [map_id for map_id in placements if maps[map_id][0].get("region") == "REGION_HOENN"]
    johto = [
        map_id for map_id in placements
        if maps[map_id][0].get("region") == "REGION_JOHTO"
        and maps[map_id][0].get("map_type") in ("MAP_TYPE_TOWN", "MAP_TYPE_CITY", "MAP_TYPE_ROUTE", "MAP_TYPE_OCEAN_ROUTE")
    ]
    collisions = sorted(
        (overlap(a, b), maps[a][2], maps[b][2])
        for a in johto for b in hoenn if overlap(a, b)
    )
    print(f"Sobreposicoes Johto importado x Hoenn: {len(collisions)}")
    for area, a, b in collisions[-15:]:
        print(f"  {a} x {b}: {area} tiles")
    for target in ("MAP_RUINS_OF_ALPH_OUTSIDE_HNS", "MAP_ILEX_FOREST_HNS"):
        adjacent_overlaps = sorted(
            (overlap(target, other), maps[other][2])
            for other in johto if other != target and overlap(target, other)
        )
        print(f"Sobreposicoes de {maps[target][2]} com Johto: {adjacent_overlaps[-10:]}")
    for current, target, candidate, established in conflicts[:10]:
        print(f"Conflito {current} -> {target}: {candidate} != {established}")


if __name__ == "__main__":
    main()
