import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('inspect_game', Path(__file__).resolve().parents[1] / 'Tools/InspectGameBuild.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


def fixture():
    data = bytearray(0x400)
    data[:2] = b'MZ'
    struct.pack_into('<I', data, 0x3c, 0x80)
    data[0x80:0x84] = b'PE\0\0'
    struct.pack_into('<HHI', data, 0x84, 0x8664, 1, 123)
    struct.pack_into('<H', data, 0x94, 0xf0)
    struct.pack_into('<H', data, 0x98, 0x20b)
    struct.pack_into('<Q', data, 0xb0, 0x140000000)
    struct.pack_into('<II', data, 0xd0, 0x3000, 0x200)
    data[0x188:0x18d] = b'.text'
    struct.pack_into('<IIII', data, 0x190, 0x300, 0x1000, 0x200, 0x200)
    data[0x200:0x204] = b'test'
    return data


class InspectTests(unittest.TestCase):
    def test_rva_mapping(self):
        pe = module.PE(fixture())
        self.assertEqual(pe.base, 0x140000000)
        self.assertEqual(pe.bytes_at(0x1000, 4), b'test')
        self.assertEqual(pe.bytes_at(0, 2), b'MZ')

    def test_zero_fill_and_bounds(self):
        pe = module.PE(fixture())
        self.assertIsNone(pe.bytes_at(0x1200))
        self.assertIsNone(pe.bytes_at(0x9000))
        self.assertEqual(len(pe.bytes_at(0x11ff)), 1)

    def test_invalid_file(self):
        for data in [b'', b'MZ', bytes(100), fixture()[:200]]:
            with self.assertRaises(ValueError):
                module.PE(data)

    def test_truncated_sections(self):
        data = fixture()
        struct.pack_into('<I', data, 0x19c, 0x350)
        with self.assertRaises(ValueError):
            module.PE(data)

    def test_named_addresses_only(self):
        self.assertEqual(module.bindings('inline constexpr uintptr_t One=0x1000,Two=0x2000; inline constexpr uintptr_t Offset=0x1f4; inline constexpr unsigned Option=0x9999;'), {'One': 0x1000, 'Two': 0x2000})


if __name__ == '__main__':
    unittest.main()
