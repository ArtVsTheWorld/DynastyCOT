from pathlib import Path
import configparser
import json
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]


class MenuCatalogTests(unittest.TestCase):
    def test_curated_catalog_is_unique_subset_without_upgrade_markers(self):
        config = configparser.ConfigParser()
        config.optionxform = str
        config.read(ROOT / 'Source/DynamicGamedayAtmosphere.ini')
        expected = {(section, key) for section in config.sections() for key in config[section]
                    if section != 'General'}
        source = (ROOT / 'Source/SettingsMenuOptions.inc').read_text()
        pairs = re.findall(r'\{[^\n]*?L"([^"]+)",L"([^"]+)"', source)
        self.assertEqual(len(pairs), len(set(pairs)))
        self.assertTrue(set(pairs) < expected)
        self.assertEqual(len(pairs), 46)
        self.assertTrue(all(section != 'Logging' for section, key in pairs))
        self.assertNotIn(('Dynamic Gameday Soundscape', 'MaximumBoostPercent'), pairs)
        self.assertNotIn(('Dynamic Gameday Attendance', 'MaximumDeparturePerQuarter'), pairs)

    def test_audio_catalog_matches_dispatch_allowlist(self):
        data = json.loads((ROOT / 'docs/REACTION-AUDIO-CATALOG.json').read_text(encoding='utf-8'))
        source = (ROOT / 'Source/ReactionCatalog.h').read_text()
        masks = [int(mask) for mask in re.findall(r'\{"[A-Z_]+",(\d+),(?:true|false)\}', source)]
        self.assertEqual(len(masks), 28)
        heard = set()
        for family in data['families']:
            index = family['family']
            expected = [index * 4 + level for level in range(1, 4) if masks[index] & (1 << level)]
            self.assertEqual(family['audible_ids'], expected)
            heard.update(expected)
        self.assertEqual(len(heard), 80)
        self.assertTrue(heard.isdisjoint({13, 14, 15, 23}))

    def test_numeric_bounds_match_configuration_audit(self):
        audit = (ROOT / 'Tests/ConfigurationOptions.inc').read_text()
        catalog = (ROOT / 'Source/SettingsMenuOptions.inc').read_text()
        expected = {(s, k): tuple(map(float, (d, lo, hi))) for s, k, d, lo, hi in
                    re.findall(r'\{L"([^"]+)",L"([^"]+)",([\d.]+),([\d.]+),([\d.]+),', audit)}
        actual = {}
        for line in catalog.splitlines():
            name = re.search(r'L"([^"]+)",L"([^"]+)"', line)
            values = re.search(r',([\d.]+),([\d.]+),([\d.]+),Kind::(\w+)\}', line)
            if name and values and values[4] != 'Profile':
                actual[name.group(1, 2)] = tuple(map(float, values.group(1, 2, 3)))
        self.assertEqual(actual, {pair: expected[pair] for pair in actual})

    def test_three_sections_cap_and_toggle_order(self):
        catalog = (ROOT / 'Source/SettingsMenuOptions.inc').read_text()
        groups = {}
        for line in catalog.splitlines():
            row = re.match(r'\{"([^\"]+)".*Kind::(\w+)\}', line)
            if row:
                groups.setdefault(row[1], []).append(row[2])
        self.assertEqual(list(groups), ['Attendance', 'Soundscape', 'Broadcast'])
        self.assertEqual([len(x) for x in groups.values()], [14, 12, 20])
        for group, controls in groups.items():
            self.assertLessEqual(len(controls), 20 if group == "Broadcast" else 15)
            numeric_seen = False
            for kind in controls:
                if kind == 'Toggle':
                    self.assertFalse(numeric_seen, 'toggles must precede numeric/dropdown controls')
                else:
                    numeric_seen = True


if __name__ == '__main__':
    unittest.main()
