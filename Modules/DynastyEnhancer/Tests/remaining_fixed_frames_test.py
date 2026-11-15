"""Existing hub, in-game and DE suites with emitted invocation-local scratch."""
from pathlib import Path
import sys,inspect,struct,math,dataclasses
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'Tools'))
from recruit_notes_vm import Machine,UNDEFINED,ScriptFunction
from build_recruit_notes_ui import NativeFrameAssembler
ns=dict(math=math,struct=struct,UNDEFINED=UNDEFINED,ScriptFunction=ScriptFunction)
s=inspect.getsource(Machine.run).replace('    def run(','def run(',1).replace('\n    ','\n').replace('pc, stack = begin, []','pc, stack = begin, []; locals_ = {}').replace('stack.append(self.globals[stack.pop()])','stack.append((lambda k: locals_.get(k,self.globals.get(k,UNDEFINED)))(stack.pop()))').replace('elif op == 0x4F:', 'elif op == 0x4D:\n                stack[-1],stack[-2]=stack[-2],stack[-1]\n            elif op == 0x3C:\n                v,k=stack.pop(),stack.pop();locals_[k]=v\n            elif op == 0x4F:')
exec(s,ns)
class Frame(dict):
 def __init__(self,n,values):self.n=n;super().__init__(values);assert all(0<=k<n for k in values),(n,list(values))
 def get(self,k,default=None):assert 0<=k<self.n,('read',k,self.n);return super().get(k,default)
 def __setitem__(self,k,v):assert 0<=k<self.n,('write',k,self.n);super().__setitem__(k,v)
class LocalMachine(Machine):
 def run(self,p,registers,begin=0,end=None):
  if hasattr(p,'native_limit'):registers=Frame(p.native_limit,registers)
  return ns['run'](self,p,registers,begin,end)
class Tracked(NativeFrameAssembler):
 def finish(self):p=super().finish();p.native_limit=self.native_registers;return p

def bound_replace(p,**changes):
 q=dataclasses.replace(p,**changes)
 if hasattr(p,'native_limit'):q.native_limit=p.native_limit
 return q

for file in ('player_hub_ui_test.py','player_ingame_ui_test.py','weekly_report_ui_test.py'):
 p=ROOT/'Tests'/file;text=p.read_text();text=text.replace('from recruit_notes_vm import Machine,UNDEFINED','from __main__ import LocalMachine as Machine,UNDEFINED').replace('from recruit_notes_vm import Machine\n','from __main__ import LocalMachine as Machine\n')
 text=text.replace('from dataclasses import replace','from __main__ import bound_replace as replace')
 if file=='player_hub_ui_test.py':text=text.replace('a=Assembler();suffix(a);return a.finish()',"a=Tracked(6 if suffix.__name__=='comparison_stat_suffix' else 5);suffix(a);return a.finish()")
 elif file=='player_ingame_ui_test.py':
  text=text.replace('a=Assembler();player_ratings_suffix(a)','a=Tracked(8);player_ratings_suffix(a)').replace('a=Assembler();preplay_sub_player_suffix(a)','a=Tracked(5);preplay_sub_player_suffix(a)').replace('a=Assembler();kicker_range_suffix(a)','a=Tracked(8);kicker_range_suffix(a)')
  text=text.replace('a=Assembler();suffix(a)',"a=Tracked({'key_player_suffix':5,'playcall_player_suffix':14,'substitution_player_suffix':5,'depth_panel_player_suffix':5}.get(suffix.__name__,14));suffix(a)")
 else:text=text.replace('a=Assembler();emit(a)',"a=Tracked(14 if emit.__name__=='matchup_suffix' else 6);emit(a)")
 exec(compile(text,str(p),'exec'),dict(__file__=str(p),__name__='native_frame_suite',Tracked=Tracked))
print('PASS: existing hub, in-game and DE report/picker suites with emitted local scratch and bounded native frames.')
