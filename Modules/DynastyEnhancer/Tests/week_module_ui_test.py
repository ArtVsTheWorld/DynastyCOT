"""Six typed practice-banner ratings, native frames and independent policies."""
from pathlib import Path
import sys, inspect, math, struct
from dataclasses import replace
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import NativeFrameAssembler
from build_week_module_ui import suffix, reviewed
from recruit_notes_vm import Machine, UNDEFINED, ScriptFunction
from grade_scales import label
ns=dict(math=math,struct=struct,UNDEFINED=UNDEFINED,ScriptFunction=ScriptFunction)
s=inspect.getsource(Machine.run).replace('    def run(','def run(',1).replace('\n    ','\n')
s=s.replace('pc, stack = begin, []','pc, stack = begin, []; locals_ = {}')
s=s.replace('stack.append(self.globals[stack.pop()])','stack.append((lambda k: locals_.get(k,self.globals.get(k,UNDEFINED)))(stack.pop()))')
s=s.replace('elif op == 0x4F:', 'elif op == 0x4D:\n                stack[-1],stack[-2]=stack[-2],stack[-1]\n            elif op == 0x3C:\n                v,k=stack.pop(),stack.pop();locals_[k]=v\n            elif op == 0x4F:')
exec(s,ns)
class Local(Machine): run=ns['run']
class Frame(dict):
    def get(self,k,default=None):
        assert 0<=k<5
        return super().get(k,default)
    def __setitem__(self,k,v):
        assert 0<=k<5
        super().__setitem__(k,v)
names=('mcAwayTeamOvr','mcAwayTeamOff','mcAwayTeamDef','mcHomeTeamOvr','mcHomeTeamOff','mcHomeTeamDef')
a=NativeFrameAssembler(5);suffix(a);base=a.finish();checks=0
for scale in range(5):
    for fog in (False,True):
        for colors in (False,True):
            p=replace(base,code=bytearray(base.code))
            for offsets,value in [(p.team_scale_offsets,scale),(p.scale_offsets,4),(p.team_offsets,int(fog)),(p.fog_offsets,0),(p.color_offsets,int(colors))]:
                for at in offsets:p.code[at]=value
            for n in range(101):
                receiver={'record':'7-3','score':24,'currency':75}
                for name in names:
                    t={'mData':{'display':str(n),'ExtraInfo':{'fontColor':0xffffff,'font':'native'}},'_width':64}
                    t['SetLabel']=lambda v,t=t:t['mData'].update(display=v)
                    t['SetExtraInfo']=lambda v,t=t:t['mData']['ExtraInfo'].update(v)
                    receiver[name]={'mData':{'value':n,'ExtraInfo':{}},'mcValue':t}
                Local({'art_styleguide':{'Font':{'RESIZE_SCALEDOWN':9}}},[]).run(p,Frame({1:receiver}))
                for name in names:
                    assert receiver[name]['mcValue']['mData']['display']==(label(n,scale) if fog else str(n))
                    assert receiver[name]['mData']['value']==n
                    assert receiver[name]['mData']['ExtraInfo']['DETeamRating'] is True
                    checks+=1
                assert receiver['record']=='7-3' and receiver['score']==24 and receiver['currency']==75
reviewed()
print(f'PASS: {checks} practice team badges, all scales/Blind/Off/colors, fixed native frame, numeric source preservation.')
