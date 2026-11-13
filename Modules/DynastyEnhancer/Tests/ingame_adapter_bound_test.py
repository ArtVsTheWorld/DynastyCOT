"""All policies on the formerly oversized, exact-fingerprint bank6 method."""
from pathlib import Path
from dataclasses import replace
import copy,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_player_ingame_ui import FIELDS,SPECS,player_ratings_suffix
from build_recruit_notes_ui import Assembler,fnv
from recruit_notes_vm import Machine
from grade_scales import label
GLOBALS={'art_styleguide':{'Font':{'RESIZE_SCALEDOWN':'scaledown'}}}
def widget(value):
 w={'mData':{'display':value,'ExtraInfo':{'fontColor':0x112233,'font':'native'}}}
 w['SetLabel']=lambda value:w['mData'].update(display=value)
 w['SetExtraInfo']=lambda patch:w['mData']['ExtraInfo'].update(patch)
 return w
a=Assembler();player_ratings_suffix(a);p=a.finish();count=0
for scale in range(5):
 for fog in (False,True):
  for colors in (False,True):
   code=bytearray(p.code)
   for offsets,value in [(p.scale_offsets,scale),(p.fog_offsets,int(fog)),(p.color_offsets,int(colors))]:
    for at in offsets:code[at]=value
   program=replace(p,code=bytes(code))
   for value in range(101):
    info={field:value for field,_ in FIELDS};info.update(overallrating=value,score=24,jersey=99,x=300)
    before=copy.deepcopy(info)
    receiver={'mData':{'ExtraInfo':info},**{name:widget(value) for _,name in FIELDS},'mcOvr':{'mData':{'value':value,'ExtraInfo':{}},'mcValue':widget(value)}}
    expected=label(value,scale) if fog else str(value) if colors else value
    for redraw in range(2):
     Machine(GLOBALS,[]).run(program,{1:receiver})
     for child in [receiver[name] for _,name in FIELDS]+[receiver['mcOvr']['mcValue']]:
      assert child['mData']['display']==expected,(scale,fog,colors,value,child)
      if not colors:assert child['mData']['ExtraInfo']['fontColor']==0x112233
      child['mData'].update(display=value,ExtraInfo={'fontColor':0x112233,'font':'native'})
     assert info==before and receiver['mcOvr']['mData']['value']==value
    count+=1
off,argc,regs,flags,size,expected,newregs,_=SPECS[0]
assert size+len(p.code)<=32768 and expected==11921223581905478919
sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
from inspect_tb_ast import load
body=load(Path('Evidence/PlayerFog/node-ig-scriptpod.ast')).data[off+48:off+48+size]
assert fnv(body)==expected
out=Path('../../Build/SpringFollowup-20261112');out.mkdir(parents=True,exist_ok=True);(out/'bank6-native-body.bin').write_bytes(body)
bounded=Assembler();bounded.code.extend(bytes(size));player_ratings_suffix(bounded);final=bounded.finish()
print(f'PASS: {count} all-scale/fog/color cases with redraws and immutable native values; exact native hash {expected}, compiled definition {len(final.code)} bytes (was 36404), unchanged 32768 limit. Outside-Dynasty native policy uses fog/colors false.')
