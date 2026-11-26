"""Native eligibility presentation, executed offline; no process/game access."""
import copy
import itertools
from pathlib import Path
import statistics
import sys
import time
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_interest_ui import Assembler,lockout_row,lockout_divider
from recruit_notes_vm import Machine,UNDEFINED

checks=0
globals_={'adapterenum_cfb_dynasty':{'RecruitingBoardFlow':{'CFME_RECRUITSTAGE_TOP10':0}},
          'loc':{'career':{'Recruiting':{'LOCKED_OUT':'Locked Out'}}}}
def program(fn,mode):
 a=Assembler();fn(a)
 if fn is lockout_row:a.ret()
 p=a.finish();code=bytearray(p.code)
 for at in p.scale_offsets:code[at]=mode
 p.code=bytes(code);return p
programs={(fn,mode):program(fn,mode) for fn in [lockout_row,lockout_divider] for mode in range(4)}

def run(order,owner,locked,mode=1,stage=2,limit=3,header=True,decorate=True,transformed=True):
 native={'RecruitStage':stage,'Locked':locked,'IsCommited':False,'Hours':50,'NIL':80000}
 original=copy.deepcopy(native);rows=[];source=[]
 for i,ordinal in enumerate(order):
  info={'TeamId':ordinal+1,'IsCurrentUserTeam':ordinal==owner,
        'IsBelowCutoff':stage>0 and i>=limit,'Offer':True,'VisitValue':'Wk 4',
        'SliderValue':87,'RankChange':2}
  row={'ExtraInfo':info};school={'TeamId':ordinal+1,'BTFNativeSchoolOrder':str(ordinal)}
  if not transformed:school.pop('BTFNativeSchoolOrder')
  if decorate:Machine(globals_,[]).run(programs[lockout_row,mode],{1:{'mCurrentRecruit':native},2:school,3:i,6:row,8:limit,26:len(order)})
  assert native==original
  assert info['Offer'] and info['VisitValue']=='Wk 4' and info['SliderValue']==87 and info['RankChange']==2
  rows.append(row);source.append(row)
 divider={'display':'Locked Out','ExtraInfo':{'height':10,'width':714}}
 if header:rows.insert(limit,divider)
 mutations=[]
 def remove(index):mutations.append(('remove',index));rows.pop(index)
 def add(value):mutations.append(('add',value));rows.append(value)
 receiver={'mTopSchoolItemProvider':{'GetNumItems':lambda:len(rows),'GetItemData':lambda i:rows[i],'RemoveItem':remove,'AddItem':add}}
 Machine(globals_,[]).run(programs[lockout_divider,mode],{1:receiver})
 assert native==original
 assert len([row for row in rows if row is not divider])==len(source)
 assert all(sum(row is s for row in rows)==1 for s in source)
 return rows,source,divider,receiver,mutations

for mode in range(4):
 for limit,stage in [(5,1),(3,2),(3,3),(1,4),(0,5)]:
  for owner in range(10):
   for locked in [True,False]:
    # Feedback/Blind can group across the original native stage boundary.
    for order in ([0,1,2,3,4,5,6,7,8,9],[3,1,5,0,8,2,4,9,6,7]) if mode in [1,3] else ([0,1,2,3,4,5,6,7,8,9],):
     rows,source,divider,receiver,mutations=run(order,owner,locked,mode,stage,limit)
     eligible=[r for r in source if not r['ExtraInfo']['IsBelowCutoff']]
     below=[r for r in source if r['ExtraInfo']['IsBelowCutoff']]
     assert rows==eligible+([divider]+below if below else [])
     assert source[order.index(owner)]['ExtraInfo']['IsBelowCutoff']==locked
     before=list(rows);count=len(mutations)
     Machine(globals_,[]).run(programs[lockout_divider,mode],{1:receiver})
     assert rows==before and len(mutations)==count
     checks+=4

# Live reproduction: native fourth school, actual native Locked=False.
for mode in range(4):
 rows,source,divider,_,_=run(list(range(10)),3,False,mode)
 assert rows[:4]==source[:4] and rows[4] is divider
 assert not source[3]['ExtraInfo']['IsBelowCutoff'];checks+=2
for mode in [1,3]:
 rows,source,divider,_,_=run(list(range(10)),3,False,mode,transformed=False)
 assert rows[:4]==source[:4] and rows[4] is divider;checks+=1

# Initial/Top10 retain the native lack of an actual Locked Out header.
for mode in range(4):
 for stage in [-1,0]:
  rows,source,divider,_,mutations=run(list(range(10)),4,False,mode,stage,5,False)
  assert rows==source and not mutations
  assert all(not r['ExtraInfo']['IsBelowCutoff'] for r in rows);checks+=2

# Unknown ownership/flag/order never becomes an invented eligibility decision.
for mode in range(4):
 for locked in [None,0,1,'false']:
  rows,source,divider,_,mutations=run(list(range(10)),3,locked,mode)
  assert not source[3]['ExtraInfo']['BTFNativeCutoffKnown'] and not mutations;checks+=1
for value in ['',None,'NaN','-1','32','1.5',0,False]:
 native={'RecruitStage':2,'Locked':False};info={'TeamId':4,'IsBelowCutoff':True,'IsCurrentUserTeam':True}
 Machine(globals_,[]).run(programs[lockout_row,1],{1:{'mCurrentRecruit':native},2:{'TeamId':4,'BTFNativeSchoolOrder':value},3:0,6:{'ExtraInfo':info},8:3,26:10})
 assert not info['BTFNativeCutoffKnown'];checks+=1

# Native entry routes without a DTO transform keep their original index.
# A missing ordinal with evidence of reordering must never use that fallback.
for mode in [1,3]:
 for marker in [None,'BTFStanding','BTFInterestOriginal']:
  native={'RecruitStage':2,'Locked':False};school={'TeamId':4}
  if marker:school[marker]='CONTENDER' if marker=='BTFStanding' else '{}'
  info={'TeamId':4,'IsBelowCutoff':True,'IsCurrentUserTeam':True}
  Machine(globals_,[]).run(programs[lockout_row,mode],{1:{'mCurrentRecruit':native},2:school,3:3,6:{'ExtraInfo':info},8:3,26:10})
  assert info['BTFNativeCutoffKnown']==(marker is None)
  if marker is None:assert not info['IsBelowCutoff']
  checks+=1

# No invalidation on an already-correct native list, and no new notices.
for mode in range(4):
 _,_,_,_,mutations=run(list(range(10)),1,False,mode)
 assert not mutations;checks+=1

for count in [-1,0,33,'10',float('nan')]:
 calls=[];receiver={'mTopSchoolItemProvider':{'GetNumItems':lambda:count,'GetItemData':lambda i:calls.append(i),'RemoveItem':lambda i:calls.append(i),'AddItem':lambda v:calls.append(v)}}
 Machine(globals_,[]).run(programs[lockout_divider,1],{1:receiver})
 assert not calls;checks+=1
for malformed in [{}, {'mCurrentRecruit':None}, {'mCurrentRecruit':{'RecruitStage':'Top3','Locked':False}}]:
 info={'TeamId':1,'IsBelowCutoff':False,'IsCurrentUserTeam':True}
 Machine(globals_,[]).run(programs[lockout_row,0],{1:malformed,2:{'TeamId':1},3:0,6:{'ExtraInfo':info},8:3,26:10})
 assert not info['BTFNativeCutoffKnown'];checks+=1

times=[]
for _ in range(15):
 begin=time.perf_counter()
 for __ in range(20):run(list(range(10)),3,False,1)
 times.append((time.perf_counter()-begin)*1000/20)
print(f'PASS: {checks} lockout presentation checks; native current-school flag, original membership, grouping, all modes, stages, missing/corrupt data, repeated population and unchanged action data.')
print(f'Offline Python VM including fixture construction: median {statistics.median(times):.3f} ms/card; slow batch {max(times):.3f} ms/card. Not live menu timing.')
