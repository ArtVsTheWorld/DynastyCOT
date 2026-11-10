"""Projected OVR display, option changes, reused rows and numeric data retention."""
from pathlib import Path
from dataclasses import replace
import copy,sys,time
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import Assembler
from build_position_changes_ui import suffix,emit,SIZE,REGISTERS
from grade_scales import label
from recruit_notes_vm import Machine,UNDEFINED

a=Assembler();suffix(a);p=a.finish();checks=0
def run(cells,columns,scale=1,fog=True,colors=False):
    before=copy.deepcopy(cells);code=bytearray(p.code)
    for offsets,value in [(p.scale_offsets,scale),(p.fog_offsets,int(fog)),(p.color_offsets,int(colors))]:
        for at in offsets:code[at]=value
    result=Machine({},[]).run(replace(p,code=bytes(code)),{4:cells,5:columns})
    assert result is cells
    for old,new in zip(before,cells):assert {k:v for k,v in old.items() if k!='display'}=={k:v for k,v in new.items() if k!='display'}
    return cells
for scale in range(5):
 for rating in range(101):
  for value in [rating,str(rating)]:
   for selected in [False,True]:
    rows=[{'display':'FS','value':0},{'display':value,'value':rating,'sort':rating,'selected':selected,'style':{'color':0 if selected else 0xffffff}},{'display':1,'value':1}]
    run(rows,[0,1,3],scale=scale)
    assert rows[1]['display']==label(rating,scale)
    assert rows[0]['display']=='FS' and rows[2]['display']==1
    # An unchanged cached grade survives repeated draws without conversion.
    old=copy.deepcopy(rows);run(rows,[0,1,3],scale=scale);assert rows==old
    checks+=5
for value in [None,UNDEFINED,'?','--','','85,TrendUp',' 85','085','85.0','101','-1','Infinity',101,-1]:
    cell={'display':value,'value':88};run([cell],[1]);assert cell['display'] is value or cell['display']==value;checks+=1
for fog in [False,True]:
 for colors in [False,True]:
    cell={'display':'82','value':82};run([cell],[1],fog=fog,colors=colors)
    assert cell['display']==('B-' if fog else '82');checks+=1
for cols in [[1],[3,0,1,2],[2,3,0],[],[1,1]]:
    rows=[{'display':'82','value':82} for _ in cols];run(rows,cols)
    assert [r['display'] for r in rows]==['B-' if c==1 else '82' for c in cols];checks+=1
run([],[])
assert emit().code[:SIZE-3]==bytes(SIZE-3) and REGISTERS==11
begin=time.perf_counter()
for _ in range(300):run([{'display':'82','value':82}],[1])
print(f'PASS {checks} projected-position display checks; same-array returns, numeric values, native styles and column keys preserved. 300 offline VM rows: {(time.perf_counter()-begin)*1000:.1f}ms (not live FPS).')
