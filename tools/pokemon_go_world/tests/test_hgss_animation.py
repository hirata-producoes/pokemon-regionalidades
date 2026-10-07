import importlib.util
from pathlib import Path
import struct
import sys
import unittest

TOOLS = Path(__file__).parents[1]
sys.path.insert(0, str(TOOLS))
spec = importlib.util.spec_from_file_location('hgss_animation', TOOLS / 'audit_hgss_animation.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


def animation():
    bank = struct.pack('<HHIIIII', 1, 2, 24, 40, 56, 0, 0)
    bank += struct.pack('<HHIII', 2, 0, 0x10002, 2, 0)
    bank += struct.pack('<IHHIHH', 0, 6, 0, 0, 6, 0) + b'\0\0'
    block = b'KNBA' + struct.pack('<I', len(bank) + 8) + bank
    return b'RNAN' + struct.pack('<HHIHH', 0xFEFF, 0x100, len(block) + 16, 16, 1) + block


class AnimationTests(unittest.TestCase):
    def test_lz_literals(self):
        self.assertEqual(audit.lz10(b'\x10\x03\0\0\0abc'), b'abc')

    def test_lz_overlap(self):
        self.assertEqual(audit.lz10(b'\x10\x06\0\0\x40a\x20\0'), b'aaaaaa')

    def test_lz_truncated(self):
        for data in (b'', b'\x10\x01\0\0', b'\x10\x01\0\0\0', b'\x10\x03\0\0\x80\0'):
            with self.subTest(data=data), self.assertRaises(ValueError): audit.lz10(data)

    def test_lz_invalid_reference_or_size(self):
        for data in (b'\x10\x03\0\0\x80\0\0', b'\x10\x02\0\0\x40a\0\0', b'\x10\0\0\0'):
            with self.subTest(data=data), self.assertRaises(ValueError): audit.lz10(data)
        with self.assertRaises(ValueError): audit.lz10(b'\x10\x10\0\0', limit=8)

    def test_nanr_sequences(self):
        result = audit.nanr_sequences(animation())
        self.assertEqual(result['sequences'][0]['durations_ticks'], [6, 6])
        self.assertEqual(result['sequences'][0]['forward_duration_ticks'], 12)

    def test_nanr_truncated(self):
        for end in (0, 15, 22, len(animation()) - 1):
            with self.subTest(end=end), self.assertRaises(ValueError): audit.nanr_sequences(animation()[:end])

    def test_nanr_bad_sections(self):
        for offset, value in ((28, 0), (32, 900), (36, 900)):
            data = bytearray(animation())
            struct.pack_into('<I', data, offset, value)
            with self.subTest(offset=offset), self.assertRaises(ValueError): audit.nanr_sequences(data)

    def test_nanr_bad_sequence(self):
        for offset, fmt, value in ((50, '<H', 2), (56, '<I', 0), (60, '<I', 1), (64, '<I', 999)):
            data = bytearray(animation())
            struct.pack_into(fmt, data, offset, value)
            with self.subTest(offset=offset), self.assertRaises(ValueError): audit.nanr_sequences(data)


if __name__ == '__main__': unittest.main()
