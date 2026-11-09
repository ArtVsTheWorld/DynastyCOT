"""Live-identified GenericToggle -> DefaultToggleItem path, not a slider mock."""
import sys,math,copy
from dataclasses import replace
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_nil_toggle_ui import draw,label,FixedFrameAssembler
from recruit_notes_vm import Machine
G={'Math':{'round':lambda x:math.floor(x+.5),'floor':math.floor}}
def program(builder,on=True,regs=10):
 a=FixedFrameAssembler(regs);builder(a);p=a.finish();raw=bytearray(p.code)
 for at in p.dollar_offsets:raw[at]=int(on)
 return replace(p,code=bytes(raw))
for n,want in [(0,'$0'),(5,'$20K'),(10,'$40K'),(250,'$1.0M')]:
 calls=[];text={'mData':{'display':str(n),'ExtraInfo':{'fontColor':42}}}
 def setlabel(x):text['mData']['display']=x;calls.append(x)
 text['SetLabel']=setlabel
 item={'mData':{'display':n,'value':n},'txtLabel':text}
 ctl={'GetCurrentIndex':lambda:3,'GetItem':lambda index:{'Movie':item}}
 parent={'mDECotCurrentNIL':True,'mCollectionControl':ctl}
 Machine(G,[]).run(program(draw),{1:parent});assert text['mData']['display']==want
 assert item['mData']=={'display':n,'value':n} and text['mData']['ExtraInfo']['fontColor']==42
 calls.clear();Machine(G,[]).run(program(label,regs=4),{1:item});assert not calls
 text['mData']['display']=str(n);Machine(G,[]).run(program(label,regs=4),{1:item});assert text['mData']['display']==want
 text['mData']['display']=str(n);Machine(G,[]).run(program(label,False,4),{1:item});assert text['mData']['display']==str(n)
 item.pop('mDECotCurrentNIL');Machine(G,[]).run(program(label,regs=4),{1:item});assert text['mData']['display']==str(n)
print('PASS actual toggle/item redraw, edits, K/M, Off, unrelated toggles, numeric data/native colors, unchanged setter suppression')
