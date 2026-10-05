"""Recoverable source/runtime snapshot; never copy personal profiles or change Git.

Restore into an EMPTY directory, not over a checkout. The archive is a complete
file tree of the included scope (including untracked/ignored generated graphics),
not a patch requiring the original Git worktree. External toolchains are not part
of this snapshot. --verify reads every archived entry and compares its SHA-256.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import zipfile
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[2]
SKIP_DIRS = {".git", "__pycache__", ".pytest_cache", ".aws", ".codex", ".agents"}
PERSONAL_SUFFIXES = {".sav", ".pgrsave", ".cfg", ".sa1", ".sgm", ".sg1", ".sna"}
CACHE_SUFFIXES = {".o", ".i", ".log", ".pyc"}


def exclusion(relative):
    p = Path(relative)
    parts = [part.lower() for part in p.parts]
    if parts[0] == "build" or any(part in SKIP_DIRS for part in parts):
        return "build/cache/git/private metadata"
    if any(part in {"profiles", "saves", "save_backups"} for part in parts):
        return "personal profiles/saves"
    # Also exclude backup variants such as game.sav.bak and game.pgrsave.tmp.
    suffixes = set(s.lower() for s in p.suffixes)
    if suffixes & PERSONAL_SUFFIXES or any(s.startswith(".ss") and s[3:].isdigit() for s in suffixes):
        return "personal save/configuration"
    if p.suffix.lower() in CACHE_SUFFIXES:
        return "rebuildable object/log/cache"
    return None


def git(*args):
    return subprocess.check_output(["git", "-C", str(ROOT), *args]).decode("utf-8", "replace").strip()


def inventory():
    files, excluded = [], []
    for parent, dirs, names in os.walk(ROOT, followlinks=False):
        for name in list(dirs):
            path = Path(parent) / name
            rel = path.relative_to(ROOT)
            reason = exclusion(rel)
            if path.is_symlink():
                raise ValueError(f"Symlink requires explicit review: {rel}")
            if reason:
                dirs.remove(name)
                excluded.append({"path": rel.as_posix() + "/", "reason": reason})
        for name in names:
            path = Path(parent) / name
            rel = path.relative_to(ROOT)
            reason = exclusion(rel)
            if reason:
                excluded.append({"path": rel.as_posix(), "reason": reason})
            else:
                if path.is_symlink():
                    raise ValueError(f"Symlink requires explicit review: {rel}")
                st = path.stat()
                files.append((rel.as_posix(), st.st_size, st.st_mtime_ns))
    return sorted(files), sorted(excluded, key=lambda e: e["path"])


def sha_file(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def verify(folder):
    manifest = json.loads((folder / "manifest.json").read_text(encoding="utf-8"))
    archive = folder / "project.zip"
    if sha_file(archive) != manifest["archive_sha256"]:
        raise ValueError("Archive checksum mismatch")
    with zipfile.ZipFile(archive) as package:
        if sorted(package.namelist()) != sorted(f["path"] for f in manifest["files"]):
            raise ValueError("Archive contents differ from manifest")
        for entry in manifest["files"]:
            p = entry["path"]
            if p.startswith("/") or ".." in Path(p).parts or ":" in p:
                raise ValueError(f"Unsafe archive path: {p}")
            with package.open(p) as stream:
                if hashlib.file_digest(stream, "sha256").hexdigest() != entry["sha256"]:
                    raise ValueError(f"Entry checksum mismatch: {p}")
            if package.getinfo(p).file_size != entry["size"]:
                raise ValueError(f"Entry size mismatch: {p}")
    print(json.dumps({"verified": True, "entries": len(manifest["files"]),
                      "archive_sha256": manifest["archive_sha256"]}))


def create(destination):
    destination = destination.resolve()
    if destination == ROOT or ROOT in destination.parents:
        raise ValueError("Snapshot must be outside the repository")
    destination.mkdir(parents=True, exist_ok=False)
    before_status = git("status", "--porcelain=v1", "--untracked-files=all")
    files, excluded = inventory()
    print(json.dumps({"phase": "copy", "entries": len(files), "bytes": sum(f[1] for f in files)}), flush=True)
    manifest = {"schema": 1, "created_utc": datetime.now(timezone.utc).isoformat(),
                "source": str(ROOT), "head": git("rev-parse", "HEAD"),
                "branch": git("branch", "--show-current"), "git_status": before_status,
                "exclusions": excluded, "files": [],
                "restore": "Extract project.zip into an empty directory. Never overwrite a working tree. "
                           "Saves, user configuration, build cache and external toolchains are excluded. "
                           "Git history is not included. manifest.json records the base commit and local status."}
    archive = destination / "project.zip"
    with zipfile.ZipFile(archive, "x", compression=zipfile.ZIP_DEFLATED, compresslevel=3, allowZip64=True) as package:
        for index, (rel, size, mtime) in enumerate(files):
            path = ROOT / rel
            digest = hashlib.sha256()
            with path.open("rb") as source, package.open(rel, "w", force_zip64=True) as target:
                while data := source.read(1024 * 1024):
                    digest.update(data)
                    target.write(data)
            stat = path.stat()
            if (stat.st_size, stat.st_mtime_ns) != (size, mtime):
                raise RuntimeError(f"File changed during snapshot: {rel}; snapshot incomplete")
            manifest["files"].append({"path": rel, "size": size, "mtime_ns": mtime, "sha256": digest.hexdigest()})
            if index and index % 10000 == 0:
                print(json.dumps({"copied": index}), flush=True)
    after_files, _ = inventory()
    if after_files != files or git("status", "--porcelain=v1", "--untracked-files=all") != before_status:
        raise RuntimeError("Project changed during snapshot; snapshot incomplete")
    manifest["archive_sha256"] = sha_file(archive)
    (destination / "manifest.json").write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"phase": "verify", "archive_bytes": archive.stat().st_size}), flush=True)
    verify(destination)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--destination", type=Path)
    parser.add_argument("--verify", type=Path)
    parser.add_argument("--inventory", action="store_true")
    args = parser.parse_args()
    if args.inventory:
        selected, skipped = inventory()
        print(json.dumps({"entries": len(selected), "bytes": sum(f[1] for f in selected), "excluded_entries": len(skipped)}))
    elif args.verify:
        verify(args.verify)
    elif args.destination:
        create(args.destination)
    else:
        parser.error("Choose --inventory, --destination or --verify")
