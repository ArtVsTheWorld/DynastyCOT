"""Stats' verified skill producer, position changes and recycled Attributes cards."""
from pathlib import Path
from dataclasses import replace
import copy
import importlib.util
import json
import sys
import time

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'Tools'))
from build_recruit_notes_ui import Assembler, NativeFrameAssembler, fnv
from build_player_card_ui import legacy_skill_owner_suffix, attribute_slots_suffix
from build_player_ingame_ui import stats_module_suffix, STATS_SPECS
from grade_scales import label
from recruit_notes_vm import Machine, UNDEFINED


def compile_(suffix, scale=0, fog=True, colors=False):
    a = Assembler(); suffix(a); p = a.finish(); code = bytearray(p.code)
    for offsets, value in [(p.scale_offsets, scale), (p.fog_offsets, int(fog)),
                           (p.color_offsets, int(colors))]:
        for at in offsets: code[at] = value
    return replace(p, code=bytes(code))


def widget(value):
    w = {'mData': {'display': value, 'ExtraInfo': {'fontColor': 0xFFFFFF}}}
    w['SetLabel'] = lambda v: w['mData'].update(display=v)
    w['SetExtraInfo'] = lambda patch: w['mData']['ExtraInfo'].update(patch)
    return w


class NativeSkills(dict):
    def get(self, key, default=None):
        assert key != 'length', 'Native producer is not an AS2 Array'
        assert not isinstance(key, int) or 1 <= key <= 6, 'Six-slot bound'
        return super().get(key, default)


def context(names, values, player=7):
    skills = NativeSkills({i+1: {'skillLabel': n, 'skillValue': v}
                           for i, (n, v) in enumerate(zip(names, values))})
    attrs = {'mData': {'ExtraInfo': {
        f'AttributesData{i}': {'ratingName': n, 'ratingValue': v, 'isFullyScouted': True}
        for i, (n, v) in enumerate(zip(names, values))}},
        **{f'mcValueSlotText{i}': widget(v) for i, v in enumerate(values)}}
    info = {'PlayerId': player, 'attributesSubpanelInfo': skills, 'roster': False,
            'overall': 'D', 'injuryRisk': 0, 'SeasonHealth': 88}
    module = {'_visible': True, 'mData': {'ExtraInfo': info}, 'mcInfoSlotAttributes': attrs}
    stats = {'mCurrentPlayerKey': player, 'mcPlayerModule': module,
             'statistics': {'rating': 114.2, 'yards': 86, 'completions': 6,
                            'attempts': 9, 'average': 5.6, 'touchdowns': 1}}
    return stats, module, attrs, skills


GLOBALS = {'art_styleguide': {'Font': {'RESIZE_SCALEDOWN': 'scaledown'}}}
producer = compile_(stats_module_suffix)
owner = compile_(legacy_skill_owner_suffix)
page_asm = NativeFrameAssembler(3); legacy_skill_owner_suffix(page_asm); page = page_asm.finish()
assert page_asm.max_register < 3

# Exact reported receiving card, not the previous test's different sixth label.
WR = ['Speed', 'Agility', 'Awareness', 'Medium Route Running', 'Catching', 'Spec Catch']
WR_VALUES = [86, 88, 74, 65, 79, 61]

# Optional private baseline comparison uses the previous source-built candidate,
# never installed files or a live game. It reproduces the all-six rejection.
if '--baseline' in sys.argv:
    p = Path(sys.argv[sys.argv.index('--baseline') + 1])
    spec = importlib.util.spec_from_file_location('baseline_card', p)
    old = importlib.util.module_from_spec(spec); spec.loader.exec_module(old)
    stats, module, attrs, skills = context(WR, WR_VALUES)
    Machine(GLOBALS, []).run(compile_(old.legacy_skill_owner_suffix), {1: module})
    Machine(GLOBALS, []).run(compile_(old.attribute_slots_suffix), {1: attrs})
    assert [attrs[f'mcValueSlotText{i}']['mData']['display'] for i in range(6)] == WR_VALUES
    print('REPRODUCED: previous candidate rejects exact WR Spec Catch card; all six ratings leak.')

# Position labels beyond the reported WR/HB are synthetic alternate-caption
# fixtures. No claim these exact abbreviations are native captures: the verified
# producer must work regardless of caption/localization, without a preset list.
families = {
    'QB': ['Throw Power', 'Short Acc', 'Mid Acc', 'Deep Acc', 'Throw on Run', 'Break Sack'],
    'HB': ['Speed', 'Agility', 'Awareness', 'Carrying', 'Trucking', 'Change of Direction'],
    'WR': WR,
    'TE': ['Speed', 'Agility', 'Awareness', 'Catching', 'Catch in Traff', 'Run Block'],
    'OL': ['Strength', 'Awareness', 'Pass Blk', 'Run Blk', 'Impact Block', 'Lead Blocking'],
    'EDGE': ['Speed', 'Acceleration', 'Tackle', 'Block Shed', 'Power Move', 'Finesse Move'],
    'LB': ['Tackle', 'Hit Power', 'Pursuit', 'Play Rec', 'Man Cov', 'Zone Cov'],
    'DB': ['Speed', 'Agility', 'Awareness', 'Man Cov', 'Zone Cov', 'Press'],
    'K/P': ['Kick Power', 'Kick Acc', 'Awareness', 'Strength', 'Stamina', 'Injury'],
    'localized': ['Vitesse', 'Agilite', 'Lecture', 'Attribut quatre', 'Attribut cinq', 'Attribut six'],
}
cases = 0
for scale in range(5):
    for fog, colors in [(True, True), (True, False), (False, True), (False, False)]:
        display = compile_(attribute_slots_suffix, scale, fog, colors)
        for family, names in families.items():
            for text in [False, True]:
                values = [str(v) for v in WR_VALUES] if text else WR_VALUES[:]
                stats, module, attrs, skills = context(names, values)
                original = copy.deepcopy(skills); table = copy.deepcopy(stats['statistics'])
                Machine(GLOBALS, []).run(producer, {1: stats})
                for callback in [owner, page]:
                    Machine(GLOBALS, []).run(callback, {1: module})
                    assert attrs['mData']['ExtraInfo']['DELegacyPlayerSkillRatings'] is True, family
                    Machine(GLOBALS, []).run(display, {1: attrs})
                    expected = [label(v, scale) if fog else str(v) if colors else raw
                                for v, raw in zip(WR_VALUES, values)]
                    assert [attrs[f'mcValueSlotText{i}']['mData']['display'] for i in range(6)] == expected
                    # Native Draw/page rebuild replaces the entire child data.
                    attrs['mData']['ExtraInfo'] = {f'AttributesData{i}': {
                        'ratingName': n, 'ratingValue': v, 'isFullyScouted': True}
                        for i, (n, v) in enumerate(zip(names, values))}
                    for i, v in enumerate(values): attrs[f'mcValueSlotText{i}']['mData']['display'] = v
                assert skills == original and stats['statistics'] == table
                assert module['mData']['ExtraInfo']['overall'] == 'D'
                cases += 1

for failure in ['wrong-id', 'missing-id', 'missing-source', 'five-slots',
                'empty-label', 'missing-value', 'hidden']:
    stats, module, attrs, skills = context(families['localized'], WR_VALUES)
    info = module['mData']['ExtraInfo']
    info['DEStatsPlayerSkillRatings'] = True  # Must revoke old permission.
    if failure == 'wrong-id': stats['mCurrentPlayerKey'] = 8
    elif failure == 'missing-id': info.pop('PlayerId')
    elif failure == 'missing-source': info.pop('attributesSubpanelInfo')
    elif failure == 'five-slots': skills.pop(6)
    elif failure == 'empty-label': skills[6]['skillLabel'] = ''
    elif failure == 'missing-value': skills[6].pop('skillValue')
    else: module['_visible'] = False
    Machine(GLOBALS, []).run(producer, {1: stats})
    assert info['DEStatsPlayerSkillRatings'] is False, failure
    Machine(GLOBALS, []).run(owner, {1: module})
    assert attrs['mData']['ExtraInfo']['DELegacyPlayerSkillRatings'] is False, failure

# Same widget, changing player or replacing the source before the next producer
# callback: old authorization is rejected, then restored only by fresh capture.
for change in ['player', 'source']:
    stats, module, attrs, skills = context(families['localized'], WR_VALUES)
    Machine(GLOBALS, []).run(producer, {1: stats})
    info = module['mData']['ExtraInfo']
    if change == 'player': info['PlayerId'] = stats['mCurrentPlayerKey'] = 9
    else:
        fresh = copy.deepcopy(skills); fresh[1]['skillLabel'] = 'Changed native caption'
        info['attributesSubpanelInfo'] = fresh
    Machine(GLOBALS, []).run(page, {1: module})
    assert attrs['mData']['ExtraInfo']['DELegacyPlayerSkillRatings'] is False, change
    Machine(GLOBALS, []).run(producer, {1: stats})
    Machine(GLOBALS, []).run(page, {1: module})
    assert attrs['mData']['ExtraInfo']['DELegacyPlayerSkillRatings'] is True, change

for concealed in [False, UNDEFINED]:
    stats, module, attrs, skills = context(WR, WR_VALUES)
    Machine(GLOBALS, []).run(producer, {1: stats})
    Machine(GLOBALS, []).run(owner, {1: module})
    for i in range(6): attrs['mData']['ExtraInfo'][f'AttributesData{i}']['isFullyScouted'] = concealed
    Machine(GLOBALS, []).run(compile_(attribute_slots_suffix), {1: attrs})
    assert [attrs[f'mcValueSlotText{i}']['mData']['display'] for i in range(6)] == WR_VALUES

# Verify source semantics and native fingerprints in the private source asset.
sys.path.insert(0, 'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
from inspect_tb_ast import load
from build_ui_method_contracts import contract
asset = load(ROOT / 'Modules/DynastyEnhancer/Evidence/PlayerFog/Stats/playergamestats.ast')
off, argc, regs, flags, size, expected, newregs, suffix = STATS_SPECS[0]
assert fnv(asset.data[off+48:off+48+size]) == expected
c = contract(asset, off)
assert c['name'] == '_UpdatePlayerId' and c['prefix'][3:6] == ['screen', 'ingame', 'PlayerGameStats']
assert regs == newregs == 17 and argc == 0 and flags == 297
a = NativeFrameAssembler(regs+1); a.code.extend(bytes(size)); suffix(a); p = a.finish()
assert a.max_register <= regs and len(p.code) < 32768
print(f'PASS {cases} position/policy cases; exact WR, alternate captions, native 1-based slots, '
      'page rebuilds, stale identity/source rejection, concealment, Off, Blind and numeric statistics preserved.')
