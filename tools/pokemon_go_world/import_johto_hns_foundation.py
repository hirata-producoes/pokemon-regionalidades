#!/usr/bin/env python3
"""Import Johto terrain, regional encounters, and selected gate interiors."""
import argparse, json, re, shutil
from pathlib import Path

MAPS = (
    "NewBarkTown_hns",
    "CherrygroveCity_hns",
    "VioletCity_hns",
    "AzaleaTown_hns",
    "GoldenrodCity_hns",
    "EcruteakCity_hns",
    "OlivineCity_hns",
    "CianwoodCity_hns",
    "SafariZoneGate_hns",
    "Mahoganytown_hns",
    "BlackthornCity_hns",
    "Route29_hns",
    "Route30_hns",
    "Route31_hns",
    "Route32_hns",
    "Route33_hns",
    "Route34_hns",
    "Route35_hns",
    "Route36_hns",
    "Route37_hns",
    "Route38_hns",
    "Route39_hns",
    "Route40_hns",
    "Route41_hns",
    "Route42_hns",
    "Route43_hns",
    "Route44_hns",
    "Route45_hns",
    "Route46_hns",
    "Route47_hns",
    "Route48_hns",
    "Route27_hns",
    "TohjoFalls_Cavern_hns",
    "Route26_hns",
    "Route26North_hns",
    "Route28_hns",
    "RuinsOfAlph_Outside_hns",
    "IlexForest_hns",
    "NationalPark_Normal_hns",
    "LakeOfRage_hns",
    "LakeOfRageLowTide_hns",
    "MtSilver_Outside_hns",
    "MtSilver_MountainSide_hns",
    "MtSilver_Snow_hns",
    "MtSilver_SummitDay_hns",
    "MtSilver_SummitNight_hns",
    "CliffEdgeGate_hns",
    "CliffEdgeCave_hns",
    "TrainerHill_Courtyard_hns",
    "SafariZone_Top_Left_hns",
    "SafariZone_Top_Mid_hns",
    "SafariZone_Top_Right_hns",
    "SafariZone_Low_Left_hns",
    "SafariZone_Low_Mid_hns",
    "SafariZone_Low_Right_hns",
    "SafariZone_Enterance_hns",
    "Gate_IlexForest_Route34_hns",
    "Gate_AzaleaTown_IlexForest_hns",
    "Gate_RuinsOfAlph_Route32_hns",
    "Gate_RuinsOfAlph_Route36_hns",
)
MUSIC = {
    "CherrygroveCity_hns": "MUS_OLDALE",
    "Route29_hns": "MUS_ROUTE101",
    "NewBarkTown_hns": "MUS_LITTLEROOT",
    "Route27_hns": "MUS_RG_ROUTE3",
    "TohjoFalls_Cavern_hns": "MUS_RG_MT_MOON",
    "Route26_hns": "MUS_RG_ROUTE3",
    "Route26North_hns": "MUS_RG_ROUTE3",
    "Route28_hns": "MUS_RG_ROUTE3",
    "MtSilver_Outside_hns": "MUS_RG_MT_MOON",
}
GROUP = "gMapGroup_JohtoFoundation_Hns"
KANTO_ROUTE22 = "MAP_ROUTE22"
SOURCE_ROUTE22 = "MAP_ROUTE22_HNS"

def load(p): return json.loads(Path(p).read_text(encoding="utf-8"))
def save(p, value):
    with Path(p).open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(json.dumps(value, indent=2, ensure_ascii=False) + "\n")
def entries(value): return value if isinstance(value, list) else []
def block(text, pattern, label):
    found = re.search(pattern, text, re.S)
    if not found: raise RuntimeError(f"Bloco ausente: {label}")
    return found.group(0).strip()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("source", type=Path)
    ap.add_argument("--project", type=Path, default=Path.cwd())
    a = ap.parse_args(); src = a.source.resolve(); dst = a.project.resolve()
    source_layouts = {x["id"]: x for x in load(src / "data/layouts/layouts.json")["layouts"]}
    source_maps = {name: load(src / "data/maps" / name / "map.json") for name in MAPS}
    map_ids = {data["id"] for data in source_maps.values()}
    source_by_id = {data["id"]: data for data in source_maps.values()}
    # Removing warps to excluded maps changes the numerical indices used by
    # remaining warp destinations. Retain only complete pairs and renumber.
    def numeric_warp_id(warp):
        value = str(warp["dest_warp_id"])
        return int(value) if value.isdecimal() else None

    retained_warps = {
        data["id"]: {
            index for index, warp in enumerate(entries(data.get("warp_events")))
            if warp["dest_map"] in map_ids and numeric_warp_id(warp) is not None
        }
        for data in source_maps.values()
    }
    changed = True
    while changed:
        changed = False
        for map_id, indices in retained_warps.items():
            warps = entries(source_by_id[map_id].get("warp_events"))
            valid = {
                index for index in indices
                if numeric_warp_id(warps[index]) in retained_warps[warps[index]["dest_map"]]
            }
            if valid != indices:
                retained_warps[map_id] = valid
                changed = True
    warp_number = {
        map_id: {old: new for new, old in enumerate(sorted(indices))}
        for map_id, indices in retained_warps.items()
    }
    layouts, tilesets = [], set()
    for name in MAPS:
        data = source_maps[name]
        data["music"] = MUSIC.get(name, "MUS_ROUTE101" if "Route" in name else "MUS_OLDALE")
        data.pop("game_version", None); data["region"] = "REGION_JOHTO"
        data["world_enabled"] = True
        for connection in entries(data.get("connections")):
            if connection["map"] == SOURCE_ROUTE22:
                connection["map"] = KANTO_ROUTE22
                # O deslocamento da Route 22 da hack deixa as bordas sem area
                # compartilhada quando usamos a Route 22 atual (48 x 24).
                connection["offset"] = 10
        data["connections"] = [
            connection for connection in entries(data.get("connections"))
            if connection["map"] in map_ids
        ]
        # Johto's 30-tile northern shift needs a future transition map. Do not
        # keep the old direct Kanto edge, which would have no shared walkable row.
        if name == "RuinsOfAlph_Outside_hns":
            data["connections"] = [c for c in data["connections"] if c["map"] != "MAP_ROUTE32_HNS"]
            for connection in data["connections"]:
                if connection["map"] == "MAP_ROUTE36_HNS":
                    connection["offset"] = -24
        elif name == "Route36_hns":
            for connection in data["connections"]:
                if connection["map"] == "MAP_RUINS_OF_ALPH_OUTSIDE_HNS":
                    connection["offset"] = 24
        elif name == "Route40_hns":
            for connection in data["connections"]:
                if connection["map"] == "MAP_TRAINER_HILL_COURTYARD_HNS":
                    connection["offset"] = -30
        elif name == "TrainerHill_Courtyard_hns":
            for connection in data["connections"]:
                if connection["map"] == "MAP_ROUTE40_HNS":
                    connection["offset"] = 30
        elif name == "EcruteakCity_hns":
            data["connections"] = [c for c in data["connections"] if c["map"] != "MAP_ROUTE42_HNS"]
        elif name == "Route42_hns":
            data["connections"] = [c for c in data["connections"] if c["map"] != "MAP_ECRUTEAK_CITY_HNS"]
        elif name == "Route45_hns":
            data["connections"] = [c for c in data["connections"] if c["map"] != "MAP_ROUTE46_HNS"]
        elif name == "Route46_hns":
            data["connections"] = [c for c in data["connections"] if c["map"] != "MAP_ROUTE45_HNS"]
        elif name == "Route32_hns":
            data["connections"] = [c for c in data["connections"] if c["map"] != "MAP_RUINS_OF_ALPH_OUTSIDE_HNS"]
        elif name in ("IlexForest_hns", "Route34_hns"):
            blocked = "MAP_ROUTE34_HNS" if name == "IlexForest_hns" else "MAP_ILEX_FOREST_HNS"
            data["connections"] = [c for c in data["connections"] if c["map"] != blocked]
        data["warp_events"] = [
            {**warp, "dest_warp_id": str(warp_number[warp["dest_map"]][numeric_warp_id(warp)])}
            for index, warp in enumerate(entries(data.get("warp_events")))
            if index in retained_warps[data["id"]]
        ]
        for key in ("object_events", "coord_events", "bg_events"): data[key] = []
        target = dst / "data/maps" / name; target.mkdir(parents=True, exist_ok=True); save(target / "map.json", data)
        (target / "scripts.inc").write_text(f"{name}_MapScripts::\n\t.byte 0\n", encoding="utf-8")
        layout = dict(source_layouts[data["layout"]]); layout.pop("game_version", None)
        # HnS usa a mesma divisao de recursos de FRLG: 640 tiles e 7 paletas
        # primarias. Tratar estes layouts como Emerald desloca os metatiles e pode
        # fazer a camera global ler recursos fora do bloco correto.
        layout["layout_version"] = "frlg"
        layout["border_width"] = 2
        layout["border_height"] = 2
        layouts.append(layout); tilesets.update((layout["primary_tileset"], layout["secondary_tileset"]))
        folder = Path(layout["blockdata_filepath"]).parent
        shutil.copytree(src / folder, dst / folder, dirs_exist_ok=True)

    (dst / "data/maps/johto_hns_scripts.inc").write_text(
        "".join(f'\t.include "data/maps/{name}/scripts.inc"\n' for name in MAPS),
        encoding="utf-8",
    )

    # Keep existing Kanto and Johto terrain distinct until an actual transition
    # map spans the new 30-tile vertical gap; never import Kanto duplicates.
    p = dst / "data/maps/Route22_Frlg/map.json"; route22 = load(p)
    route22["connections"] = [
        connection for connection in route22["connections"]
        if connection["map"] != "MAP_ROUTE26NORTH_HNS"
    ]
    save(p, route22)
    p = dst / "data/maps/map_groups.json"; root = load(p)
    if GROUP not in root["group_order"]: root["group_order"].append(GROUP)
    root[GROUP] = list(MAPS); save(p, root)
    p = dst / "data/layouts/layouts.json"; root = load(p); ids = {x["id"] for x in layouts}
    root["layouts"] = [x for x in root["layouts"] if x["id"] not in ids] + layouts; save(p, root)
    p = dst / "src/data/region_map/region_map_sections.json"; root = load(p)
    source_section_data = load(src / "src/data/region_map/region_map_sections.json")
    source_sections = {
        section["id"]: section
        for section in source_section_data["map_sections"] + source_section_data["hns_map_sections"]
    }
    wanted = {load(src / "data/maps" / n / "map.json")["region_map_section"] for n in MAPS}
    missing_sections = wanted - source_sections.keys()
    if missing_sections: raise RuntimeError(f"Secoes de mapa ausentes: {sorted(missing_sections)}")
    root["map_sections"] = [x for x in root["map_sections"] if x["id"] not in wanted] + [source_sections[section_id] for section_id in sorted(wanted)]; save(p, root)
    p = dst / "src/data/wild_encounters.json"; root = load(p); source_wild = load(src / "src/data/wild_encounters.json")
    target_group = next(x for x in root["wild_encounter_groups"] if x["label"] == "gWildMonHeaders")
    source_group = next(x for x in source_wild["wild_encounter_groups"] if x["label"] == "gWildMonHeaders")
    target_group["encounters"] = [x for x in target_group["encounters"] if x["map"] not in map_ids] + [x for x in source_group["encounters"] if x["map"] in map_ids]; save(p, root)
    hs = (src / "src/data/tilesets/headers.h").read_text(); gs = (src / "src/data/tilesets/graphics.h").read_text(); ms = (src / "src/data/tilesets/metatiles.h").read_text()
    headers=[]; graphics=[]; metatiles=[]
    for symbol in sorted(tilesets):
        suffix=symbol.removeprefix("gTileset_")
        h=block(hs, rf"const struct Tileset {re.escape(symbol)}\s*=\s*\{{.*?\n\}};", symbol)
        headers.append(re.sub(r"\.callback\s*=\s*[^,]+,", ".callback = NULL,", h))
        tile=block(gs, rf"const u32 gTilesetTiles_{re.escape(suffix)}\[\]\s*=\s*INCBIN_U32\([^;]+;", symbol)
        palette=block(gs, rf"const u16 gTilesetPalettes_{re.escape(suffix)}\[\]\[16\]\s*=\s*\{{.*?\n\}};", symbol)
        g=tile+"\n"+palette
        g=re.sub(r'INCBIN_U32\("([^\"]+)/tiles\.4bpp\.(fastSmol|lz)"\)', r'INCGFX_U32("\1/tiles.png", ".4bpp.\2")', g)
        g=re.sub(r'INCBIN_U16\("([^\"]+)\.gbapal"\)', r'INCGFX_U16("\1.pal", ".gbapal")', g); graphics.append(g)
        metatiles.append(block(ms, rf"const u16 gMetatiles_{re.escape(suffix)}\[\].*?;\s*\nconst u16 gMetatileAttributes_{re.escape(suffix)}\[\].*?;", symbol))
        for asset in set(re.findall(r'data/tilesets/(?:primary|secondary)/[^/\"]+', g)): shutil.copytree(src / asset, dst / asset, dirs_exist_ok=True)
    out=dst/"src/data/tilesets"
    (out/"johto_hns_graphics.h").write_text("\n\n".join(graphics)+"\n"); (out/"johto_hns_metatiles.h").write_text("\n\n".join(metatiles)+"\n"); (out/"johto_hns_headers.h").write_text("\n\n".join(headers)+"\n")
    (dst/"include/tilesets_johto_hns.h").write_text("\n".join(f"extern const struct Tileset {x};" for x in sorted(tilesets))+"\n")
    print(f"Importados {len(MAPS)} mapas, {len(layouts)} layouts e {len(tilesets)} tilesets.")
if __name__ == "__main__": main()
