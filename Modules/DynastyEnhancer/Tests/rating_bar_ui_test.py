"""Native ColorFillSlider grade color, idempotence and owned Off/reuse restoration."""
import sys
from dataclasses import replace
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import Assembler
from build_rating_bar_ui import rating_bars
from recruit_notes_vm import Machine,UNDEFINED
from grade_scales import COLORS

def program(on=True):
 a=Assembler();rating_bars(a);a.ret();p=a.finish();code=bytearray(p.code)
 for at in p.scale_offsets:code[at]=3
 for at in p.color_offsets:code[at]=int(on)
 return replace(p,code=bytes(code))
checks=0
for grade in ['POOR','FAIR','GOOD','GREAT','ELITE','Poor','Fair','Good','Great','Elite']:
 calls=[];native={'RecolorByValue':True,'FillColor':123,'BackgroundColor':456};extra=dict(native)
 bar={'mData':{'ExtraInfo':extra}}
 def setter(data):extra.update(data);calls.append(data)
 bar['SetExtraInfo']=setter
 source={'value':83,'ExtraInfo':{'DEPlayerRating':True,'RightLabelDisplayOverride':grade,'NumberOfSteps':99}}
 parent={'mSlider':bar,'mData':source};Machine({},[]).run(program(),{1:parent})
 assert extra['FillColor']==COLORS[grade] and extra['RecolorByValue'] is False and source['value']==83
 calls.clear();Machine({},[]).run(program(),{1:parent});assert not calls
 Machine({},[]).run(program(False),{1:parent});assert extra==native and bar['mDECotGradeFillState'] is UNDEFINED
 Machine({},[]).run(program(),{1:parent});source['ExtraInfo']['DEPlayerRating']=False
 Machine({},[]).run(program(),{1:parent});assert extra==native
 source['ExtraInfo']['DEPlayerRating']=True;source['ExtraInfo']['NumberOfSteps']=20
 calls.clear();Machine({},[]).run(program(),{1:parent});assert not calls and extra==native
 source['ExtraInfo']['NumberOfSteps']=99;Machine({},[]).run(program(),{1:parent});extra['FillColor']=999
 Machine({},[]).run(program(False),{1:parent});assert extra['FillColor']==999 and extra['RecolorByValue'] is True
 checks+=6
print(f'PASS {checks} native heat-map color, no-op repaint, Off/reuse, protected upgrade bars and foreign-writer checks')
