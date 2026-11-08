from pathlib import Path
import importlib.util
import struct
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('audit_mapped_game', ROOT / 'Tools/AuditMappedGame.py')
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


def image(base=0x140000000):
    data = bytearray(0x2000)
    data[:2] = b'MZ'
    struct.pack_into('<I', data, 0x3c, 0x80)
    data[0x80:0x84] = b'PE\0\0'
    struct.pack_into('<H', data, 0x84, 0x8664)
    optional = 0x80 + 24
    struct.pack_into('<H', data, optional, 0x20b)
    struct.pack_into('<Q', data, optional + 24, 0x140000000)
    struct.pack_into('<I', data, optional + 56, len(data))
    data[0x1000:0x1004] = b'\x48\x83\xec\x28'
    struct.pack_into('<Q', data, 0x1100, base + 0x1200)
    return data


CHECKS = [dict(kind='bytes', source='fixture', rva=0x1000, expected='4883ec28'),
          dict(kind='pointer', source='fixture', rva=0x1100, target=0x1200)]


class MappedAuditTests(unittest.TestCase):
    def test_code_and_pointer_guards(self):
        self.assertEqual(MODULE.audit(image(), CHECKS)['passed'], 2)
        data = image()
        data[0x1000] = 0x90
        report = MODULE.audit(data, CHECKS)
        self.assertEqual(report['passed'], 1)
        self.assertEqual(report['failures'][0]['rva'], 0x1000)
        self.assertEqual(report['failures'][0]['actual'], '9083ec28')

    def test_loaded_base_override(self):
        relocated = image(0x150000000)
        self.assertEqual(MODULE.audit(relocated, CHECKS)['passed'], 1)
        self.assertEqual(MODULE.audit(relocated, CHECKS, 0x150000000)['passed'], 2)

    def test_wrong_image_or_architecture_rejected(self):
        for cause in ('truncated', 'machine', 'magic', 'header'):
            data = image()
            if cause == 'truncated':
                data = data[:-1]
            elif cause == 'machine':
                struct.pack_into('<H', data, 0x84, 0xaa64)
            elif cause == 'magic':
                struct.pack_into('<H', data, 0x98, 0x10b)
            else:
                data[:2] = b'??'
            with self.assertRaises(ValueError):
                MODULE.audit(data, CHECKS)

    def test_address_parser_resolves_only_symbols_and_integer_addition(self):
        self.assertEqual(MODULE.address('attendance::bindings::Example+0x28', {'Example': 0x1234}), 0x125c)
        for expression in ['missing', 'Example-1', 'Example + run()', 'Example+']:
            with self.assertRaises(ValueError):
                MODULE.address(expression, {'Example': 0x1234})

    def test_current_manifest_includes_all_menu_slots_and_hook_prefixes(self):
        checks = MODULE.specifications()
        slots = [check for check in checks if check['source'].startswith('NativeSettingsMenu.h/slot')]
        self.assertEqual(len(slots), 42)
        for file in ('ReplayUpdateHook.h/Prefix', 'StartingAttendanceLaunchHook.h/Prefix', 'PauseMenu.h/Prefix'):
            self.assertTrue(any(check['source'] == file for check in checks))
        self.assertTrue(all(check['rva'] > 0 for check in checks))


if __name__ == '__main__':
    unittest.main()
