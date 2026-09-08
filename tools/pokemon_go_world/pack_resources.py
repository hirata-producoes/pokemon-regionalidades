#!/usr/bin/env python3
"""Build or inspect the Pokemon Regionalidades native resource pack."""

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
    seen_hashes: dict[int, str] = {}
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
        hash_value = fnv1a64(name)
        if hash_value in seen_hashes:
            raise ValueError(
                f"FNV-1a hash collision: {name!r} and {seen_hashes[hash_value]!r} "
                f"both use {hash_value:016x}"
            )
        seen.add(name)
        seen_hashes[hash_value] = name
        resources.append({"name": name, "sources": sources, "hash": hash_value})

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


def read_pack(path: Path, verify_payloads: bool = False) -> tuple[list[dict[str, int | str]], int]:
    actual_file_size = path.stat().st_size
    with path.open("rb") as pack:
        raw_header = pack.read(HEADER.size)
        if len(raw_header) != HEADER.size:
            raise ValueError("truncated resource pack header")
        magic, version, count, index_offset, string_offset, data_offset, file_size = HEADER.unpack(raw_header)
        if magic != MAGIC or version != VERSION or file_size != actual_file_size:
            raise ValueError("invalid resource pack header")
        if index_offset != HEADER.size:
            raise ValueError(f"invalid index offset: {index_offset}")
        expected_string_offset = index_offset + count * ENTRY.size
        if string_offset != expected_string_offset:
            raise ValueError(
                f"invalid string-table offset: {string_offset} != {expected_string_offset}"
            )
        if not string_offset <= data_offset <= file_size or data_offset % ALIGNMENT != 0:
            raise ValueError("invalid string-table or data offset")

        entries: list[dict[str, int | str]] = []
        for index in range(count):
            pack.seek(index_offset + index * ENTRY.size)
            raw_entry = pack.read(ENTRY.size)
            if len(raw_entry) != ENTRY.size:
                raise ValueError(f"truncated resource entry {index}")
            hash_value, offset, size, name_offset, name_length, checksum = ENTRY.unpack(raw_entry)
            if name_length == 0 or name_length > MAX_NAME_LENGTH:
                raise ValueError(f"invalid name length in resource entry {index}")
            if name_offset < string_offset or name_offset + name_length > data_offset:
                raise ValueError(f"resource entry {index} points outside the string table")
            if offset < data_offset or offset > file_size or size > file_size - offset:
                raise ValueError(f"resource entry {index} points outside the payload area")
            if offset % ALIGNMENT != 0:
                raise ValueError(f"resource entry {index} has an unaligned payload")
            pack.seek(name_offset)
            encoded_name = pack.read(name_length)
            if len(encoded_name) != name_length:
                raise ValueError(f"truncated name in resource entry {index}")
            name = encoded_name.decode("utf-8")
            if not name or "\\" in name or fnv1a64(name) != hash_value:
                raise ValueError(f"invalid name or hash in resource entry {index}: {name!r}")
            entry: dict[str, int | str] = {
                "name": name,
                "hash": hash_value,
                "offset": offset,
                "size": size,
                "checksum": checksum,
            }
            entries.append(entry)

        ordering = [(int(entry["hash"]), str(entry["name"])) for entry in entries]
        if ordering != sorted(ordering):
            raise ValueError("resource index is not sorted by hash and name")
        names = [str(entry["name"]) for entry in entries]
        if len(names) != len(set(names)):
            raise ValueError("resource index contains duplicate names")

        if verify_payloads:
            buffer_size = 1024 * 1024
            for index, entry in enumerate(entries):
                remaining = int(entry["size"])
                checksum = 0
                pack.seek(int(entry["offset"]))
                while remaining:
                    chunk = pack.read(min(buffer_size, remaining))
                    if not chunk:
                        raise ValueError(f"truncated payload for resource {entry['name']!r}")
                    checksum = zlib.crc32(chunk, checksum)
                    remaining -= len(chunk)
                if checksum != int(entry["checksum"]):
                    raise ValueError(
                        f"checksum mismatch for resource {entry['name']!r}: "
                        f"{checksum:08x} != {int(entry['checksum']):08x}"
                    )

    return entries, data_offset


def inspect(path: Path) -> None:
    entries, data_offset = read_pack(path)
    for entry in entries:
        print(
            f"{entry['name']}\t{entry['size']}\t@{entry['offset']}\t"
            f"hash={int(entry['hash']):016x}\tcrc32={int(entry['checksum']):08x}"
        )
    print(f"{len(entries)} resources; data starts at {data_offset}; file size {path.stat().st_size}")


def verify(path: Path, manifest: Path | None, root: Path) -> None:
    entries, _ = read_pack(path, verify_payloads=True)
    actual = {str(entry["name"]): entry for entry in entries}

    if manifest is not None:
        expected_resources = load_manifest(manifest, root)
        expected_names = {str(resource["name"]) for resource in expected_resources}
        actual_names = set(actual)
        missing = sorted(expected_names - actual_names)
        unexpected = sorted(actual_names - expected_names)
        if missing or unexpected:
            details = []
            if missing:
                details.append(f"missing={missing[:10]!r}")
            if unexpected:
                details.append(f"unexpected={unexpected[:10]!r}")
            raise ValueError("resource pack differs from manifest: " + ", ".join(details))

        for resource in expected_resources:
            name = str(resource["name"])
            sources = [Path(source) for source in resource["sources"]]
            expected_size = sum(source.stat().st_size for source in sources)
            expected_checksum = calculate_crc32(sources)
            entry = actual[name]
            if int(entry["size"]) != expected_size:
                raise ValueError(
                    f"size mismatch for resource {name!r}: "
                    f"{entry['size']} != {expected_size}"
                )
            if int(entry["checksum"]) != expected_checksum:
                raise ValueError(
                    f"manifest checksum mismatch for resource {name!r}: "
                    f"{int(entry['checksum']):08x} != {expected_checksum:08x}"
                )

    manifest_note = f" and {manifest}" if manifest is not None else ""
    print(
        f"Verified {path}: {len(entries)} resources, all payload checksums valid"
        f"{manifest_note}."
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)

    build_parser = subparsers.add_parser("build")
    build_parser.add_argument("--manifest", type=Path, required=True)
    build_parser.add_argument("--output", type=Path, required=True)
    build_parser.add_argument("--root", type=Path, default=Path.cwd())

    inspect_parser = subparsers.add_parser("inspect")
    inspect_parser.add_argument("pack", type=Path)

    verify_parser = subparsers.add_parser("verify")
    verify_parser.add_argument("pack", type=Path)
    verify_parser.add_argument("--manifest", type=Path)
    verify_parser.add_argument("--root", type=Path, default=Path.cwd())

    args = parser.parse_args()
    if args.command == "build":
        build(args.manifest.resolve(), args.output.resolve(), args.root.resolve())
    elif args.command == "inspect":
        inspect(args.pack.resolve())
    else:
        verify(
            args.pack.resolve(),
            args.manifest.resolve() if args.manifest is not None else None,
            args.root.resolve(),
        )


if __name__ == "__main__":
    main()
