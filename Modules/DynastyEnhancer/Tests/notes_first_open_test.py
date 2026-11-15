"""Navigation may execute before discovery; Board.Init must repair its array."""
from pathlib import Path
import sys,time
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import Assembler,ensure_init_tab,append_tab
from recruit_notes_vm import Machine,UNDEFINED

GLOBALS={'shared_enum_career':{'recruitinghub':{'StateMachine':{'STATE_SCOUTING_TAB':'scout'},'ScreenKey':{'SUBSCREEN_RECRUITINGHUB_BOARD_SCOUTING':'scoutScreen'}}}}

def program(builder):
    a=Assembler();builder(a);assert a.max_register<=3 or builder==ensure_init_tab;return a.finish()

def native_tabs():return [{'display':'Overview','key':700}, {'display':'Recruiting','key':701}, {'display':'Scouting','StateMachineKey':'scout','ScreenKey':'scoutScreen','key':702}]

code=program(ensure_init_tab)
def opening(rows):
    params={'SubscreensData':rows};view={'GetScreenInfo':lambda:{'GetInitParams':lambda:params}}
    regs={1:{'model':'original'},2:{'super':'original'},4:view,5:{'service':'original'},6:{'parent':'original'}}
    before=regs.copy();Machine(GLOBALS,[]).run(code,regs)
    assert all(regs[k] is before[k] for k in [1,2,4,5,6]),'Initialization arguments changed'
    return rows

# Reproduce the actual live sequence: native Navigation returns three tabs,
# the worker admits the group, then the first Board.Init consumes that array.
rows=native_tabs();identities=list(rows);opening(rows)
assert len(rows)==4 and rows[3]['DENotes'] is True
assert all(rows[i] is identities[i] for i in range(3))
assert rows[2]['display']=='Scouting (100%)'
assert rows[3]['StateMachineKey']=='scout' and rows[3]['ScreenKey']=='scoutScreen'
assert rows[3]['key']==rows[2]['key']==702,'Late initialization invented an unregistered native screen layer'
for _ in range(10):opening(rows)
assert len(rows)==4 and rows[2]['display']=='Scouting (100%)','Repeated Init duplicated Notes or the percentage reservation'

# When Navigation was already attached, its fourth row remains untouched.
rows=native_tabs();Machine(GLOBALS,[]).run(program(append_tab),{1:rows});identities=list(rows);opening(rows)
assert len(rows)==4 and all(rows[i] is identities[i] for i in range(4))

for rows in [[],[{},{}],[{},{},{}],[{},{},None],[{},{},42],[{},{},{'StateMachineKey':'wrong','ScreenKey':'scoutScreen'}],[{},{},{'StateMachineKey':'scout','ScreenKey':'wrong'}],[{},{},{'StateMachineKey':'scout','ScreenKey':'scoutScreen'}]]:
    count=len(rows);opening(rows);assert len(rows)==count,'Unrecognized native navigation changed'
for params in [{},{'SubscreensData':None},{'SubscreensData':UNDEFINED},{'SubscreensData':'abc'}]:
    Machine(GLOBALS,[]).run(code,{4:{'GetScreenInfo':lambda:{'GetInitParams':lambda:params}}})
for view in [None,UNDEFINED,{'GetScreenInfo':lambda:None},{'GetScreenInfo':lambda:{'GetInitParams':lambda:None}}]:
    Machine(GLOBALS,[]).run(code,{4:view})
begin=time.perf_counter()
for _ in range(2000):opening(native_tabs())
print(f'PASS: missed Navigation -> first Init repair, already-present/repeated opens, native argument ownership, malformed/sparse data; 2000 VM initializations {(time.perf_counter()-begin)*1000:.2f} ms.')
