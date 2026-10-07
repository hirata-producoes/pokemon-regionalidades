import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location("hgss_audit", Path(__file__).parents[1] / "audit_hgss_ui_reference.py")
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


def archive(start=0, end=4):
    fat = b"BTAF" + struct.pack("<IHHII", 20, 1, 0, start, end)
    body = b"GMIF" + struct.pack("<I", 12) + b"test"
    return b"NARC" + struct.pack("<HHIHH", 0xFFFE, 0x100, 16 + len(fat + body), 16, 2) + fat + body


class ReferenceAuditTests(unittest.TestCase):
    def test_valid_member(self):
        members = audit.narc_directory(archive())
        self.assertEqual(members[0]["size"], 4)
        self.assertEqual(members[0]["sha256"], audit.sha(b"test"))

    def test_truncation_rejected(self):
        with self.assertRaises(ValueError):
            audit.narc_directory(archive()[:-1])

    def test_out_of_bounds_rejected(self):
        with self.assertRaises(ValueError):
            audit.narc_directory(archive(end=5))

    def test_reversed_bounds_rejected(self):
        with self.assertRaises(ValueError):
            audit.narc_directory(archive(start=3, end=1))


if __name__ == "__main__":
    unittest.main()
