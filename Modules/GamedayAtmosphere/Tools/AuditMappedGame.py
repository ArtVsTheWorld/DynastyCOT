"""Audit existing DGA checks against a private mapped-module snapshot, not a disk EXE.

Read-only file inspection. Does not start a game, locate new symbols, patch memory,
execute native functions, bypass protection, or certify gameplay compatibility.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re
import struct

ROOT = Path(__file__).resolve().parents[1]


def symbols(root):
    values = {}
    for name in ('GameBindings.h', 'NativeStartingAttendance.h', 'NativeSettingsMenu.h'):
        for key, raw in re.findall(r'(\w+)\s*=\s*(0x[0-9a-fA-F]+)',
                                   (root / 'Source' / name).read_text(encoding='utf-8-sig')):
            values.setdefault(key, int(raw, 16))
    return values


def address(expression, values):
    parts = expression.strip().split('+')
    result = 0
    for part in parts:
        term = part.strip().split('::')[-1]
        if term in values:
            result += values[term]
        elif re.fullmatch(r'0x[0-9a-fA-F]+|\d+', term):
            result += int(term, 0)
        else:
            raise ValueError('Unresolved native address: ' + expression)
    return result


def byte_list(text):
    return bytes(int(x.strip(), 0) for x in text.split(',') if x.strip())


def specifications(root=ROOT):
    values = symbols(root)
    checks = []
    patterns = (r'\b(?:bytes|requiredBytes)\(([^,\n]+),\{([^{}]+)\}\)',
                r'if\(rva==([^)]*)\)return \{([^{}]+)\}',
                r'\{(Swell\w+),\{([^{}]+)\}\}')
    for path in sorted((root / 'Source').glob('*')):
        if path.suffix not in ('.h', '.cpp'):
            continue
        text = path.read_text(encoding='utf-8-sig')
        for pattern in patterns:
            for match in re.finditer(pattern, text):
                try:
                    raw = byte_list(match[2])
                except ValueError:
                    # Named array iterators are checked explicitly below.
                    if '.begin()' in match[2] and '.end()' in match[2]:
                        continue
                    raise
                checks.append(dict(kind='bytes', source=path.name,
                                   rva=address(match[1], values), expected=raw.hex()))
    arrays = (('ReplayUpdateHook.h', 'Prefix', values['ReplayUpdate']),
              ('NativeStartingAttendance.h', 'FormulaPrefix', values['Formula']),
              ('NativeStartingAttendance.h', 'LaunchPrefix', values['LaunchCopy']),
              ('NativeStartingAttendance.h', 'SetterPrefix', values['OptionSet']),
              ('StartingAttendanceLaunchHook.h', 'Prefix', values['StartingAttendanceBoundary']),
              ('NativeSettingsMenu.h', 'ListPrefix', values['ListSite']),
              ('NativeSettingsMenu.h', 'PauseListPrefix', values['PauseListSite']),
              ('PauseMenu.h', 'Prefix', values['PauseMenuText']))
    for file, name, rva in arrays:
        match = re.search(r'\b' + name + r'=\{([^}]+)\}',
                          (root / 'Source' / file).read_text(encoding='utf-8-sig'), re.S)
        if not match:
            raise ValueError('Missing native prefix: ' + file + '/' + name)
        checks.append(dict(kind='bytes', source=file + '/' + name, rva=rva,
                           expected=byte_list(match[1]).hex()))
    runtime = (root / 'Source/NativeRuntime.cpp').read_text(encoding='utf-8-sig')
    for match in re.finditer(r'at<A>\(image\+([^()]+)\)==image\+([^&|;\n)},]+)', runtime):
        checks.append(dict(kind='pointer', source='NativeRuntime.cpp',
                           rva=address(match[1], values), target=address(match[2], values)))
    menu = (root / 'Source/NativeSettingsMenu.h').read_text(encoding='utf-8-sig')
    entries = re.search(r'constexpr A entries\[\]=\{([^}]+)\}', menu)
    if not entries:
        raise ValueError('Native menu dispatch table missing')
    for index, target in enumerate(entries[1].split(',')):
        checks.append(dict(kind='pointer', source='NativeSettingsMenu.h/slot' + str(index),
                           rva=values['ProviderVtable'] + index * 8,
                           target=address(target, values)))
    pause_entries = re.search(r'constexpr A pauseEntries\[\]=\{([^}]+)\}', menu)
    if not pause_entries:
        raise ValueError('Pause settings dispatch table missing')
    pause_targets = pause_entries[1].split(',')
    for index in range(42):
        target = pause_targets[index] if index < len(pause_targets) else '0x78cd63a'
        checks.append(dict(kind='pointer', source='NativeSettingsMenu.h/pause-slot' + str(index),
                           rva=values['PauseProviderVtable'] + index * 8,
                           target=address(target, values)))
    return checks


def audit(snapshot, checks, base=None):
    if snapshot[:2] != b'MZ' or len(snapshot) < 0x1000:
        raise ValueError('Expected a complete mapped PE image')
    nt = struct.unpack_from('<I', snapshot, 0x3c)[0]
    if nt > 0x1000 or snapshot[nt:nt + 4] != b'PE\0\0':
        raise ValueError('Invalid PE header')
    if struct.unpack_from('<H', snapshot, nt + 4)[0] != 0x8664:
        raise ValueError('Expected x64 machine')
    optional = nt + 24
    if struct.unpack_from('<H', snapshot, optional)[0] != 0x20b:
        raise ValueError('Expected x64 PE32+')
    base = struct.unpack_from('<Q', snapshot, optional + 24)[0] if base is None else base
    if not isinstance(base, int) or base < 0x10000 or base > (1 << 64) - (1 << 32):
        raise ValueError('Invalid loaded module base')
    size = struct.unpack_from('<I', snapshot, optional + 56)[0]
    if len(snapshot) != size:
        raise ValueError('Mapped image size differs; a protected disk EXE is not a mapped snapshot')
    failures = []
    for check in checks:
        expected = (bytes.fromhex(check['expected']) if check['kind'] == 'bytes'
                    else struct.pack('<Q', base + check['target']))
        actual = snapshot[check['rva']:check['rva'] + len(expected)]
        if actual != expected:
            failures.append({**check, 'actual': actual.hex()})
    return dict(checks=len(checks), passed=len(checks) - len(failures), failures=failures,
                image_base=hex(base), image_size=size,
                limitation='Existing byte/pointer checks only; no heap-layout, audible/rendered/FPS or full compatibility proof')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--snapshot', required=True, type=Path)
    parser.add_argument('--out', required=True, type=Path)
    parser.add_argument('--base', type=lambda value: int(value, 0),
                        help='Actual loaded base when different from PE preferred base')
    args = parser.parse_args()
    if args.snapshot.resolve() == args.out.resolve():
        parser.error('Output must not replace snapshot')
    try:
        data = args.snapshot.read_bytes()
        report = audit(data, specifications(), args.base)
        report['snapshot_sha256'] = hashlib.sha256(data).hexdigest()
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(report, indent=2) + '\n')
        print(f"Mapped-image audit: {report['passed']}/{report['checks']} checks passed")
        if report['failures']:
            print(json.dumps(report['failures'][:8], indent=2))
            return 1
        return 0
    except (OSError, ValueError, struct.error) as error:
        parser.exit(1, 'Audit failed: ' + str(error) + '\n')


if __name__ == '__main__':
    raise SystemExit(main())
