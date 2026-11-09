"""Repair only a mismatched native highlight on a marked 20-step Ratings bucket."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import Assembler
from build_rating_bar_ui import restore_bucket_focus
from recruit_notes_vm import Machine
a=Assembler();restore_bucket_focus(a);a.ret();p=a.finish()
G={'cafe':{'enum':{'FocusMethod':{'JUMP':2}}}}
checks=0
for marked in (False,True):
 for steps in (20,99):
  for parent in (False,True):
   for focused in (False,True):
    calls=[];child={'mIsFocused':focused}
    def focus(method):assert method==2;child['mIsFocused']=True;calls.append('Focus')
    def unfocus(method):assert method==2;child['mIsFocused']=False;calls.append('Unfocus')
    child.update(Focus=focus,Unfocus=unfocus)
    source={'value':7,'ExtraInfo':{'DEPlayerRating':marked,'NumberOfSteps':steps,'RightLabelDisplayOverride':'GREAT'}}
    widget={'mData':source,'mcHighlight':child,'mIsFocused':parent}
    Machine(G,[]).run(p,{1:widget})
    want=marked and steps==20 and parent!=focused
    assert calls==(['Focus' if parent else 'Unfocus'] if want else [])
    assert source['value']==7 and source['ExtraInfo']['RightLabelDisplayOverride']=='GREAT'
    calls.clear();Machine(G,[]).run(p,{1:widget});assert not calls
    checks+=3
for widget in ({},{'mData':{}},{'mData':{'ExtraInfo':{'DEPlayerRating':True,'NumberOfSteps':20}}}):
 Machine(G,[]).run(p,{1:widget})
print(f'PASS {checks} native focus synchronization checks; unmarked/attribute sliders, numeric levels and stable repaints unchanged')
