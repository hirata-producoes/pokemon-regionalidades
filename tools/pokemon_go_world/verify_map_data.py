#!/usr/bin/env python3
"""Verify map, layout, connection, warp, and PC layout-resource integrity."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


def load_json(path: Path) -> dict[str, object]:
    with path.open("r", encoding="utf-8") as source:
        document = json.load(source)
    if not isinstance(document, dict):
        raise ValueError(f"expected a JSON object: {path}")
    return document


def aligned_size(value: int, alignment: int = 4) -> int:
    return (value + alignment - 1) & ~(alignment - 1)


def require(condition: bool, message: str, errors: list[str]) -> None:
    if not condition:
        errors.append(message)


def verify(root: Path) -> None:
    errors: list[str] = []
    warnings: list[str] = []
    layouts_path = root / "data/layouts/layouts.json"
    groups_path = root / "data/maps/map_groups.json"
    generated_resources_path = root / "build/pc-generated/map_layout_resources.json"

    layouts_document = load_json(layouts_path)
    layouts = layouts_document.get("layouts")
    if not isinstance(layouts, list):
        raise ValueError(f"layouts must be an array: {layouts_path}")

    layouts_by_id: dict[str, dict[str, object]] = {}
    layout_names: set[str] = set()
    for index, raw_layout in enumerate(layouts):
        require(isinstance(raw_layout, dict), f"layout {index} is not an object", errors)
        if not isinstance(raw_layout, dict):
            continue
        layout_id = raw_layout.get("id")
        name = raw_layout.get("name")
        width = raw_layout.get("width")
        height = raw_layout.get("height")
        require(isinstance(layout_id, str) and bool(layout_id), f"layout {index} has no id", errors)
        require(isinstance(name, str) and bool(name), f"layout {index} has no name", errors)
        require(isinstance(width, int) and width > 0, f"layout {layout_id} has invalid width", errors)
        require(isinstance(height, int) and height > 0, f"layout {layout_id} has invalid height", errors)
        if not isinstance(layout_id, str) or not isinstance(name, str):
            continue
        require(layout_id not in layouts_by_id, f"duplicate layout id: {layout_id}", errors)
        require(name not in layout_names, f"duplicate layout name: {name}", errors)
        layouts_by_id[layout_id] = raw_layout
        layout_names.add(name)

        for key in ("border_filepath", "blockdata_filepath"):
            relative_path = raw_layout.get(key)
            require(isinstance(relative_path, str), f"layout {layout_id} has no {key}", errors)
            if not isinstance(relative_path, str):
                continue
            asset = root / relative_path
            require(asset.is_file(), f"layout {layout_id} is missing {relative_path}", errors)
            if not asset.is_file():
                continue
            size = asset.stat().st_size
            require(size > 0 and size % 2 == 0, f"layout {layout_id} has invalid {key} size {size}", errors)
            if key == "blockdata_filepath" and isinstance(width, int) and isinstance(height, int):
                expected = width * height * 2
                require(
                    expected <= size <= aligned_size(expected),
                    f"layout {layout_id} blockdata size {size} does not match {width}x{height}",
                    errors,
                )

    groups_document = load_json(groups_path)
    group_order = groups_document.get("group_order")
    if not isinstance(group_order, list):
        raise ValueError(f"group_order must be an array: {groups_path}")
    require(len(group_order) == len(set(group_order)), "group_order contains duplicates", errors)

    grouped_names: list[str] = []
    for group_name in group_order:
        require(isinstance(group_name, str), f"invalid map group name: {group_name!r}", errors)
        if not isinstance(group_name, str):
            continue
        members = groups_document.get(group_name)
        require(isinstance(members, list), f"map group {group_name} is missing or not an array", errors)
        if isinstance(members, list):
            grouped_names.extend(str(member) for member in members)
    require(len(grouped_names) == len(set(grouped_names)), "a map occurs in more than one group", errors)

    disk_names = {path.parent.name for path in (root / "data/maps").glob("*/map.json")}
    grouped_name_set = set(grouped_names)
    for name in sorted(grouped_name_set - disk_names):
        errors.append(f"map group references missing map directory: {name}")
    for name in sorted(disk_names - grouped_name_set):
        if "Unused" in name:
            warnings.append(f"unused map directory is intentionally ungrouped: {name}")
        else:
            errors.append(f"map directory is absent from map_groups.json: {name}")

    maps_by_id: dict[str, dict[str, object]] = {}
    map_layouts: dict[str, dict[str, object]] = {}
    for name in grouped_names:
        map_path = root / "data/maps" / name / "map.json"
        if not map_path.is_file():
            continue
        map_document = load_json(map_path)
        map_id = map_document.get("id")
        map_name = map_document.get("name")
        layout_id = map_document.get("layout")
        require(map_name == name, f"map directory/name mismatch: {name} != {map_name!r}", errors)
        require(isinstance(map_id, str) and bool(map_id), f"map {name} has no id", errors)
        require(isinstance(layout_id, str), f"map {name} has no layout id", errors)
        if not isinstance(map_id, str) or not isinstance(layout_id, str):
            continue
        require(map_id not in maps_by_id, f"duplicate map id: {map_id}", errors)
        require(layout_id in layouts_by_id, f"map {map_id} uses unknown layout {layout_id}", errors)
        maps_by_id[map_id] = map_document
        if layout_id in layouts_by_id:
            map_layouts[map_id] = layouts_by_id[layout_id]

    allowed_directions = {"up", "down", "left", "right", "dive", "emerge"}
    off_map_events = 0
    for map_id, map_document in maps_by_id.items():
        for connection in map_document.get("connections") or []:
            if not isinstance(connection, dict):
                errors.append(f"map {map_id} contains an invalid connection")
                continue
            destination = connection.get("map")
            require(destination in maps_by_id, f"map {map_id} connects to unknown map {destination}", errors)
            require(connection.get("direction") in allowed_directions,
                    f"map {map_id} has invalid connection direction {connection.get('direction')!r}", errors)
            require(isinstance(connection.get("offset"), int),
                    f"map {map_id} has a non-integer connection offset", errors)

        for warp in map_document.get("warp_events") or []:
            if not isinstance(warp, dict):
                errors.append(f"map {map_id} contains an invalid warp")
                continue
            destination = warp.get("dest_map")
            if destination == "MAP_DYNAMIC":
                continue
            require(destination in maps_by_id, f"map {map_id} warps to unknown map {destination}", errors)
            if destination not in maps_by_id:
                continue
            try:
                destination_warp = int(str(warp.get("dest_warp_id")))
            except (TypeError, ValueError):
                errors.append(f"map {map_id} has invalid destination warp {warp.get('dest_warp_id')!r}")
                continue
            destination_warps = maps_by_id[destination].get("warp_events") or []
            require(
                isinstance(destination_warps, list)
                and 0 <= destination_warp < len(destination_warps),
                f"map {map_id} targets invalid warp {destination_warp} in {destination}",
                errors,
            )

        layout = map_layouts.get(map_id)
        if layout is not None:
            width = int(layout["width"])
            height = int(layout["height"])
            for event_list_name in ("object_events", "warp_events", "coord_events", "bg_events"):
                for event in map_document.get(event_list_name) or []:
                    if not isinstance(event, dict) or "x" not in event or "y" not in event:
                        continue
                    x = int(event["x"])
                    y = int(event["y"])
                    if not (0 <= x < width and 0 <= y < height):
                        off_map_events += 1

    generated_document = load_json(generated_resources_path)
    generated_resources = generated_document.get("resources")
    if not isinstance(generated_resources, list):
        raise ValueError(f"resources must be an array: {generated_resources_path}")
    generated_by_name: dict[str, dict[str, object]] = {}
    for raw_resource in generated_resources:
        if not isinstance(raw_resource, dict):
            errors.append("generated map resource is not an object")
            continue
        resource_name = raw_resource.get("name")
        source = raw_resource.get("source")
        require(isinstance(resource_name, str), "generated map resource has no name", errors)
        require(isinstance(source, str), f"map resource {resource_name!r} has no source", errors)
        if not isinstance(resource_name, str) or not isinstance(source, str):
            continue
        require(resource_name not in generated_by_name, f"duplicate generated map resource: {resource_name}", errors)
        require((root / source).is_file(), f"generated map resource source is missing: {source}", errors)
        generated_by_name[resource_name] = raw_resource

    external_layouts: set[str] = set()
    for resource_name in generated_by_name:
        if resource_name.endswith("_Layout_Border"):
            external_layouts.add(resource_name.removeprefix("map_layouts/").removesuffix("_Border"))
        elif resource_name.endswith("_Layout_Blockdata"):
            external_layouts.add(resource_name.removeprefix("map_layouts/").removesuffix("_Blockdata"))
        else:
            errors.append(f"unexpected generated map resource name: {resource_name}")
    for layout_name in sorted(external_layouts):
        for suffix in ("_Border", "_Blockdata"):
            resource_name = f"map_layouts/{layout_name}{suffix}"
            require(resource_name in generated_by_name,
                    f"external layout {layout_name} is missing {suffix}", errors)
        require(layout_name in layout_names, f"external resource uses unknown layout {layout_name}", errors)

    if errors:
        preview = "\n".join(f"- {error}" for error in errors[:50])
        extra = f"\n- ... and {len(errors) - 50} more" if len(errors) > 50 else ""
        raise ValueError(f"map-data verification failed with {len(errors)} error(s):\n{preview}{extra}")

    if off_map_events:
        warnings.append(
            f"{off_map_events} event coordinates intentionally or historically sit outside layout bounds"
        )
    print(
        f"Verified map data: {len(maps_by_id)} maps in {len(group_order)} groups, "
        f"{len(layouts_by_id)} layouts, {len(external_layouts)} externalized layouts, "
        f"{len(generated_by_name)} packaged layout resources."
    )
    for warning in warnings:
        print(f"Note: {warning}.")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path.cwd())
    args = parser.parse_args()
    verify(args.root.resolve())


if __name__ == "__main__":
    main()
