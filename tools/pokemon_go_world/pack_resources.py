#!/usr/bin/env python3
"""Build or inspect the Pokemon GO World native resource pack."""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import struct
import tempfile
import zlib


MAGIC = b"PGWPACK\0"
VERSION = 1
HEADER = struct.Struct("<8sIIQQQQ")
ENTRY = struct.Struct("<QQQQII")
ALIGNMENT = 16
MAX_NAME_LENGTH = 4096


def fnv1a64(value: str) -> int:
    result = 0xCBF29CE484222325
    for byte in value.encode("utf-8"):
        result ^= byte
        result = (result * 0x100000001B3) & 0xFFFFFFFFFFFFFFFF
    return result


def align(value: int) -> int:
    return (value + ALIGNMENT - 1) & ~(ALIGNMENT - 1)


def calculate_crc32(paths: list[Path]) -> int:
    checksum = 0
    for path in paths:
        with path.open("rb") as source:
            while chunk := source.read(1024 * 1024):
                checksum = zlib.crc32(chunk, checksum)
    return checksum


def load_manifest(path: Path, root: Path) -> list[dict[str, object]]:
    document = json.loads(path.read_text(encoding="utf-8"))
    if document.get("format_version") != VERSION:
        raise ValueError(f"manifest format_version must be {VERSION}")

    documents = [document]
    for resource_list_name in document.get("resource_lists", []):
        resource_list_path = root / resource_list_name
        resource_list = json.loads(resource_list_path.read_text(encoding="utf-8"))
        if resource_list.get("format_version") != VERSION:
            raise ValueError(f"resource list {resource_list_path} format_version must be {VERSION}")
        documents.append(resource_list)

    resources: list[dict[str, object]] = []
    seen: set[str] = set()
    items = [item for current_document in documents for item in current_document.get("resources", [])]
    for item in items:
        name = item["name"]
        if ("source" in item) == ("sources" in item):
            raise ValueError(f"resource {name!r} must define exactly one of source or sources")
        source_names = [item["source"]] if "source" in item else item["sources"]
        if not isinstance(source_names, list) or not source_names:
            raise ValueError(f"resource {name!r} has no sources")
        sources = [root / source_name for source_name in source_names]
        if (not isinstance(name, str) or not name or "\\" in name
                or len(name.encode("utf-8")) > MAX_NAME_LENGTH):
            raise ValueError(f"invalid resource name: {name!r}")
        if name in seen:
            raise ValueError(f"duplicate resource name: {name}")
        for source in sources:
            if not source.is_file():
                raise FileNotFoundError(f"resource source does not exist: {source}")
        seen.add(name)
        resources.append({"name": name, "sources": sources, "hash": fnv1a64(name)})

    resources.sort(key=lambda resource: (resource["hash"], resource["name"]))
    return resources


def build(manifest: Path, output: Path, root: Path) -> None:
    resources = load_manifest(manifest, root)
    index_offset = HEADER.size
    string_offset = index_offset + len(resources) * ENTRY.size

    strings = bytearray()
    for resource in resources:
        encoded_name = str(resource["name"]).encode("utf-8")
        resource["name_offset"] = string_offset + len(strings)
        resource["name_length"] = len(encoded_name)
        strings.extend(encoded_name)

    data_offset = align(string_offset + len(strings))
    next_offset = data_offset
    for resource in resources:
        sources = [Path(source) for source in resource["sources"]]
        resource["size"] = sum(source.stat().st_size for source in sources)
        resource["checksum"] = calculate_crc32(sources)
        resource["offset"] = next_offset
        next_offset = align(next_offset + int(resource["size"]))
    file_size = next_offset

    output.parent.mkdir(parents=True, exist_ok=True)
    temp_name: str | None = None
    try:
        with tempfile.NamedTemporaryFile("w+b", dir=output.parent, delete=False) as pack:
            temp_name = pack.name
            pack.write(HEADER.pack(MAGIC, VERSION, len(resources), index_offset,
                                   string_offset, data_offset, file_size))
            for resource in resources:
                pack.write(ENTRY.pack(int(resource["hash"]), int(resource["offset"]),
                                      int(resource["size"]), int(resource["name_offset"]),
                                      int(resource["name_length"]), int(resource["checksum"])))
            pack.write(strings)
            pack.write(b"\0" * (data_offset - pack.tell()))
            for resource in resources:
                desired_offset = int(resource["offset"])
                pack.write(b"\0" * (desired_offset - pack.tell()))
                for source_path in resource["sources"]:
                    with Path(source_path).open("rb") as source:
                        while chunk := source.read(1024 * 1024):
                            pack.write(chunk)
            pack.write(b"\0" * (file_size - pack.tell()))
        os.replace(temp_name, output)
        temp_name = None
    finally:
        if temp_name is not None:
            Path(temp_name).unlink(missing_ok=True)

    print(f"Built {output} with {len(resources)} resources ({file_size} bytes)")


def inspect(path: Path) -> None:
    with path.open("rb") as pack:
        raw_header = pack.read(HEADER.size)
        if len(raw_header) != HEADER.size:
            raise ValueError("truncated resource pack header")
        magic, version, count, index_offset, string_offset, data_offset, file_size = HEADER.unpack(raw_header)
        if magic != MAGIC or version != VERSION or file_size != path.stat().st_size:
            raise ValueError("invalid resource pack header")
        for index in range(count):
            pack.seek(index_offset + index * ENTRY.size)
            hash_value, offset, size, name_offset, name_length, checksum = ENTRY.unpack(pack.read(ENTRY.size))
            pack.seek(name_offset)
            name = pack.read(name_length).decode("utf-8")
            print(f"{name}\t{size}\t@{offset}\thash={hash_value:016x}\tcrc32={checksum:08x}")
        print(f"{count} resources; data starts at {data_offset}; file size {file_size}")


def main() -> None:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)

    build_parser = subparsers.add_parser("build")
    build_parser.add_argument("--manifest", type=Path, required=True)
    build_parser.add_argument("--output", type=Path, required=True)
    build_parser.add_argument("--root", type=Path, default=Path.cwd())

    inspect_parser = subparsers.add_parser("inspect")
    inspect_parser.add_argument("pack", type=Path)

    args = parser.parse_args()
    if args.command == "build":
        build(args.manifest.resolve(), args.output.resolve(), args.root.resolve())
    else:
        inspect(args.pack.resolve())


if __name__ == "__main__":
    main()
