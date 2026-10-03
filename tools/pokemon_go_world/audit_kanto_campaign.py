#!/usr/bin/env python3
"""Report risks before enabling all FRLG events in the shared PC world."""

import json
import re
from collections import Counter
from pathlib import Path


def macros(path):
    return dict(re.findall(r"^#define\s+([A-Z][A-Z0-9_]+)\s+([^\s/]+)", path.read_text(encoding="utf-8"), re.M))


def main():
    root = Path(__file__).resolve().parents[2]
    groups = json.loads((root / "data/maps/map_groups.json").read_text(encoding="utf-8"))
    preview = []
    scripts = []
    for group in groups["group_order"]:
        for name in groups[group]:
            path = root / "data/maps" / name / "map.json"
            data = json.loads(path.read_text(encoding="utf-8"))
            if data.get("region") != "REGION_KANTO" or data.get("world_enabled"):
                continue
            preview.append(name)
            script = root / "data/maps" / name / "scripts.inc"
            if script.exists():
                scripts.append(script.read_text(encoding="utf-8"))
    text = "\n".join(scripts)
    families = (
        ("FLAG", macros(root / "include/constants/flags.h")),
        ("VAR", macros(root / "include/constants/vars.h")),
        ("TRAINER", macros(root / "include/constants/opponents.h")),
    )
    print(f"Mapas de Kanto em prévia sem eventos ativos: {len(preview)}")
    for prefix, defined in families:
        used = set(re.findall(rf"\b{prefix}_[A-Z0-9_]+\b", text))
        missing = sorted(used - defined.keys())
        zero = sorted(name for name in used & defined.keys() if defined[name] == "0")
        print(f"{prefix}: {len(used)} nomes usados, {len(missing)} ausentes, {len(zero)} definidos como zero")
        print("  ausentes:", ", ".join(missing[:12]))
        print("  zero:", ", ".join(zero[:12]))
    battle = Counter(re.findall(r"\btrainerbattle_\w+\s+(TRAINER_[A-Z0-9_]+)", text))
    print(f"Treinadores diferentes em scripts de prévia: {len(battle)}")
    print("Amostra:", ", ".join(name for name, _ in battle.most_common(12)))


if __name__ == "__main__":
    main()
