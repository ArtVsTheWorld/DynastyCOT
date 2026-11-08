"""Exact AIO source operand normalization and shipped adapter selection."""
from pathlib import Path
import json,struct,sys,re
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'Tools'))
sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
from inspect_tb_ast import load
from apt_source_pointers import source_pointers

rows=json.loads((ROOT/'Evidence/Aio141/method-map.json').read_text())
header=(ROOT/'Source/AioCurrencyUiPrograms.h').read_text()
for r in rows:
    source=Path(r['asset']);source=source if source.is_absolute() else ROOT/source
    apt=load(source);start=r['new']+48;original=apt.data[start:start+r['size']]
    pointers=source_pointers(apt,r['new'],r['size'])
    for base in (0x140000000,0x379160000,0x3cccb0000):
        relocated=bytearray(original)
        for at,value in pointers:struct.pack_into('<Q',relocated,at,base+value)
        for at,value in pointers:
            assert struct.unpack_from('<Q',relocated,at)[0]==base+value
            struct.pack_into('<Q',relocated,at,value)
        assert relocated==original
    # Exact complete-asset validation makes unchanged siblings necessary when
    # another callback in their ordinary native bank has moved under AIO.
    selected=r['group'] not in ('topbar','detail')
    assert (f'result.push_back({{{r["new"]},' in header)==selected
# Keep the source-pinned AIO row distinct from the stock Blueprint bank.
blueprint=(ROOT/'Source/PlayerBlueprintUiPrograms.h').read_text()
assert 'aioPrograms()' in blueprint and 'result.push_back({570688,0,5,25,2058,' in blueprint
assert 'playerBlueprintAioUi' in (ROOT/'Source/RecruitingFog.cpp').read_text()
# A decoder must reject unknown instructions, even if an apparent pointer follows.
class Bad: data=bytes(48)+b'\xff'+bytes(15)
try:source_pointers(Bad(),0,16)
except ValueError:pass
else:raise AssertionError('Unknown opcode accepted')
print(f'Passed: {len(rows)} AIO callbacks at three asset bases, complete sibling banks, exact pointer operands, topbar retained, separate Highest NIL adapter and unknown-opcode rejection.')
