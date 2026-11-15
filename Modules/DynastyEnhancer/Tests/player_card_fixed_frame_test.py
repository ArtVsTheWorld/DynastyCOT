"""Run the full existing card suite with actual native scratch-frame limits."""
from pathlib import Path
import sys,inspect,struct,math
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'Tools'))
from recruit_notes_vm import Machine,UNDEFINED,ScriptFunction
from build_recruit_notes_ui import NativeFrameAssembler

namespace=dict(math=math,struct=struct,UNDEFINED=UNDEFINED,ScriptFunction=ScriptFunction)
source=inspect.getsource(Machine.run).replace('    def run(','def run(',1).replace('\n    ','\n')
source=source.replace('pc, stack = begin, []','pc, stack = begin, []; locals_ = {}')
source=source.replace('stack.append(self.globals[stack.pop()])','stack.append((lambda key: locals_.get(key,self.globals.get(key,UNDEFINED)))(stack.pop()))')
source=source.replace('elif op == 0x4F:', 'elif op == 0x4D:\n                stack[-1],stack[-2]=stack[-2],stack[-1]\n            elif op == 0x3C:\n                value,key=stack.pop(),stack.pop();locals_[key]=value\n            elif op == 0x4F:')
exec(source,namespace)
limits={}
class Frame(dict):
    def __init__(self,count,values):self.count=count;super().__init__(values);assert all(0<=k<count for k in values)
    def get(self,key,default=None):assert 0<=key<self.count,('read outside native frame',key,self.count);return super().get(key,default)
    def __setitem__(self,key,value):assert 0<=key<self.count,('write outside native frame',key,self.count);return super().__setitem__(key,value)
class LocalMachine(Machine):
    def run(self,program,registers,begin=0,end=None):
        if id(program) in limits:registers=Frame(limits[id(program)][1],registers)
        return namespace['run'](self,program,registers,begin,end)

test=ROOT/'Tests/player_card_ui_test.py';text=test.read_text()
text=text.replace('from recruit_notes_vm import Machine,UNDEFINED','from __main__ import LocalMachine as Machine,UNDEFINED,NativeFrameAssembler,limits')
text=text.replace('a=Assembler();suffix(a);a.ret();return a.finish()', "count={'group_suffix':3,'level_suffix':6,'subskills_suffix':8,'skill_slots_suffix':12}.get(suffix.__name__)\n    a=Assembler() if count is None else NativeFrameAssembler(count)\n    suffix(a);a.ret();p=a.finish()\n    if count is not None:limits[id(p)]=(p,count)\n    return p")
exec(compile(text,str(test),'exec'),dict(__file__=str(test),__name__='fixed_frame_suite'))
print('PASS: existing card/progression suite with invocation-local scratch and bounded native frames (3/6/8/12 registers).')
