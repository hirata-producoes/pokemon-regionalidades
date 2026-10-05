"""Scope/verification tests: no access to real profiles or saves."""
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
import zipfile

spec = importlib.util.spec_from_file_location("snapshot", Path(__file__).parents[1] / "snapshot_ui_baseline.py")
snapshot = importlib.util.module_from_spec(spec)
spec.loader.exec_module(snapshot)


class BaselineTests(unittest.TestCase):
    def test_preserves_sources_and_runtime(self):
        for p in ("src/new_ui.c", "graphics/a.4bpp.lz", "graphics/pc_panel/art.bmp",
                  "pokemon_regionalidades.pak", "pokemon_regionalidades-pc.exe", "SDL2.dll", "config.mk"):
            self.assertIsNone(snapshot.exclusion(p), p)

    def test_excludes_personal_data_and_rebuildable_cache(self):
        for p in ("pokemon_regionalidades.sav", "game.sav.bak", "game.pgrsave.tmp",
                  "game.cfg", "profiles/main/data.json", "build/cache.bin", ".git", ".aws/credentials",
                  "tools/__pycache__/a.pyc", "src/game.o", "game.ss1"):
            self.assertIsNotNone(snapshot.exclusion(p), p)

    def test_verify_detects_entry_tampering(self):
        with tempfile.TemporaryDirectory() as name:
            folder = Path(name)
            with zipfile.ZipFile(folder / "project.zip", "w") as package:
                package.writestr("src/test.c", "original")
            manifest = {"archive_sha256": snapshot.sha_file(folder / "project.zip"),
                        "files": [{"path": "src/test.c", "size": 8,
                                   "sha256": hashlib.sha256(b"original").hexdigest()}]}
            (folder / "manifest.json").write_text(json.dumps(manifest))
            snapshot.verify(folder)
            manifest["files"][0]["sha256"] = "0" * 64
            (folder / "manifest.json").write_text(json.dumps(manifest))
            with self.assertRaises(ValueError):
                snapshot.verify(folder)


if __name__ == "__main__":
    unittest.main()
