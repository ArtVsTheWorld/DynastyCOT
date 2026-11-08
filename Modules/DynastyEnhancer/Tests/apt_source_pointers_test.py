"""Typed nested APT relocations, including the approved empty-pool form."""
from pathlib import Path
from types import SimpleNamespace
import struct
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'Tools'))
from apt_source_pointers import source_pointers

data = bytearray(512)
# Outer callback body at48: nested-function opcode + aligned48-byte header,
# with one typed string operand in its child body at104..120.
data[48] = 0x8e
struct.pack_into('<QIHHQI', data, 56, 300, 0, 4, 25, 320, 16)
data[104] = 0xa1
struct.pack_into('<Q', data, 112, 400)
apt = SimpleNamespace(data=bytes(data))
nested = []
assert source_pointers(apt, 0, 72, nested) == [(64, 400)]
assert nested == [(8, 300, 320, True)]
try: source_pointers(apt, 0, 72)
except ValueError: pass
else: raise AssertionError('Nested metadata became implicitly allowed')
for offset, value in [(56, 9999), (72, 9999), (88, 1), (96, 1)]:
    changed = bytearray(data)
    struct.pack_into('<Q', changed, offset, value)
    try: source_pointers(SimpleNamespace(data=bytes(changed)), 0, 72, [])
    except ValueError: pass
    else: raise AssertionError(('Foreign nested source header accepted', offset))
changed = bytearray(data); struct.pack_into('<I', changed, 88, 0x98765432); struct.pack_into('<Q', changed, 96, 0x12345678)
nested = []
assert source_pointers(SimpleNamespace(data=bytes(changed)), 0, 72, nested) == [(64, 400)]
assert nested == [(8, 300, 320, False)]
print('PASS: typed recursive pointer offsets, explicit nested/empty-pool contracts and foreign metadata rejection.')
