"""Pin/read HGSS reference files and NARC directories; no game assets activated.

This is structural evidence, not a sprite decoder or a fidelity test. A checked
member exists and has a stable hash; its visual purpose still needs a source trace.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

REVISION = "9d8b7591f09b65804da2fb2dfd56f320633e0d36"
SOURCE_FILES = (
    "README.md", "INSTALL.md", "src/battle/battle_input.c", "include/battle/battle_input.h",
    "include/constants/battle_menu.h", "src/party_menu.c", "src/party_menu_sprites.c",
    "src/party_menu_items.c", "src/party_menu_list_items.c", "src/party_context_menu.c",
    "include/party_menu.h", "src/bag_view.c", "include/bag_view.h", "src/bag.c",
    "asm/overlay_15.s", "include/overlay_15.h", "asm/unk_02088288.s",
    "src/launch_application.c", "src/start_menu.c", "src/font.c", "src/font_data.c",
    "src/naming_screen.c", "src/options_app.c", "src/overlay_trainer_card.c",
    "src/touch_save_app.c", "files/graphic/plist_gra.mk", "include/filesystem.h",
)
SOURCE_DIRS = ("src/application/pokegear/main", "src/application/pokegear/map", "src/application/pokedex")
RESOURCE_DIRS = ("files/graphic/plist_gra", "files/graphic/font", "files/graphic/zukan_gra",
                 "files/application/pokegear/pgear_gra")
ARCHIVES = ("files/a/0/0/7", "files/a/0/0/8", "files/a/0/7/2")
BATTLE_MEMBERS = {28, 36, 37, 38, 39, 41, 42, 43, 173, 174, 246, 349}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def narc_directory(data):
    if len(data) < 16 or data[:4] != b"NARC":
        raise ValueError("Not a NARC archive")
    bom, version, size, header_size, count = struct.unpack_from("<HHIHH", data, 4)
    if bom != 0xFFFE or version != 0x100 or size != len(data) or header_size != 16:
        raise ValueError("Invalid/unsupported NARC header")
    blocks, offset = {}, header_size
    for _ in range(count):
        if offset + 8 > len(data):
            raise ValueError("Truncated NARC block")
        tag, length = struct.unpack_from("<4sI", data, offset)
        if length < 8 or offset + length > len(data) or tag in blocks:
            raise ValueError("Invalid/duplicate NARC block")
        blocks[tag] = data[offset + 8:offset + length]
        offset += length
    if offset != len(data) or b"BTAF" not in blocks or b"GMIF" not in blocks:
        raise ValueError("Incomplete NARC structure")
    fat, contents = blocks[b"BTAF"], blocks[b"GMIF"]
    if len(fat) < 4:
        raise ValueError("Truncated NARC directory")
    members = struct.unpack_from("<H", fat)[0]
    if len(fat) != 4 + 8 * members:
        raise ValueError("Invalid NARC directory size")
    result, last_end = [], 0
    for index in range(members):
        start, end = struct.unpack_from("<II", fat, 4 + 8 * index)
        if start < last_end or end < start or end > len(contents):
            raise ValueError("Invalid NARC member bounds")
        payload = contents[start:end]
        result.append({"id": index, "size": len(payload), "sha256": sha(payload),
                       "prefix_hex": payload[:8].hex()})
        last_end = end
    return result


def collect(reference):
    def git(*args):
        return subprocess.check_output(["git", "-C", str(reference), *args]).decode().strip()
    if git("rev-parse", "HEAD") != REVISION:
        raise ValueError("Reference revision differs from lock")
    if git("status", "--porcelain", "--untracked-files=all"):
        raise ValueError("Reference checkout must be clean")
    paths = set(SOURCE_FILES + ARCHIVES)
    for directory in SOURCE_DIRS + RESOURCE_DIRS:
        folder = reference / directory
        if not folder.is_dir():
            raise FileNotFoundError(folder)
        paths.update(p.relative_to(reference).as_posix() for p in folder.rglob("*") if p.is_file())
    records, archives = [], []
    for relative in sorted(paths):
        payload = (reference / relative).read_bytes()
        records.append({"path": relative, "size": len(payload), "sha256": sha(payload)})
        if relative in ARCHIVES:
            members = narc_directory(payload)
            if relative == "files/a/0/0/7" and max(BATTLE_MEMBERS) >= len(members):
                raise ValueError("Battle source references absent members")
            archives.append({"path": relative, "member_count": len(members), "members": members})
    return {"schema": 1, "upstream": "https://github.com/pret/pokeheartgold", "revision": REVISION,
            "status": "reference_only_not_ported", "coverage": "selected UI sources/resources; not complete UI state coverage",
            "files": records, "archives": archives}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", required=True, type=Path)
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--record", action="store_true")
    args = parser.parse_args()
    result = collect(args.reference)
    if args.record:
        args.manifest.parent.mkdir(parents=True, exist_ok=True)
        with args.manifest.open("x", encoding="utf-8") as stream:
            stream.write(json.dumps(result, ensure_ascii=False, indent=2) + "\n")
    elif json.loads(args.manifest.read_text(encoding="utf-8")) != result:
        raise ValueError("Reference files differ from recorded manifest")
    print(json.dumps({"revision": REVISION, "files": len(result["files"]), "verified": not args.record,
                      "archives": [{"path": a["path"], "members": a["member_count"]} for a in result["archives"]]}))
