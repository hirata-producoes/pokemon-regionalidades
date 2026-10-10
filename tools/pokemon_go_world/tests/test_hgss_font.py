import importlib.util
from pathlib import Path
import struct
import sys
import unittest

TOOLS = Path(__file__).parents[1]
sys.path.insert(0, str(TOOLS))
spec = importlib.util.spec_from_file_location('hgss_font', TOOLS / 'audit_hgss_font.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


def font():
    return struct.pack('<III4B', 16, 48, 2, 8, 8, 1, 1) + bytes([1] * 16 + [2] * 16) + bytes([3, 7])


class FontReferenceTests(unittest.TestCase):
    def test_one_based_ids_have_distinct_glyphs(self):
        data = font()
        header = audit.font_header(data)
        first = audit.glyph_metadata(data, header, 1)
        second = audit.glyph_metadata(data, header, 2)
        self.assertEqual((first['advance'], second['advance']), (3, 7))
        self.assertNotEqual(first['packed_sha256'], second['packed_sha256'])

    def test_zero_and_out_of_range_rejected(self):
        for index in (0, -1, 3):
            with self.subTest(index=index), self.assertRaises(ValueError):
                audit.glyph_metadata(font(), audit.font_header(font()), index)

    def test_bounds_and_dimensions_rejected(self):
        for offset, value, fmt in ((0, 0, '<I'), (4, 32, '<I'), (8, 99, '<I'), (14, 0, '<B'), (15, 3, '<B')):
            data = bytearray(font())
            struct.pack_into(fmt, data, offset, value)
            with self.subTest(offset=offset), self.assertRaises(ValueError): audit.font_header(data)

    def test_truncation_rejected(self):
        for length in (0, 15, 48, 49):
            with self.subTest(length=length), self.assertRaises(ValueError): audit.font_header(font()[:length])


if __name__ == '__main__': unittest.main()
