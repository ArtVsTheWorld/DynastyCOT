"""Grade presentation must be ready before AddItems; native values stay numeric."""
import sys,math,copy
from dataclasses import replace
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import Assembler
from build_ratings_publication_ui import prepare,fit_level
from recruit_notes_vm import Machine
from grade_scales import BANDS

def style():
 d={'fontColor':0xeeeeee,'font':'native'};d['CloneAndModify']=lambda changes:{**d,**changes};return d
sg={k:style for k in ('GetStyleSUBHEADER_8','GetStyleSPEC_NUMBER_7')}
G={'Math':{'round':lambda x:math.floor(x+.5),'floor':math.floor},'art_styleguide':{'Color':{'PRIMARY_LIGHT_COLOR':0xeeeeee},'Font':{'GetStyleguide':lambda n:sg,'RESIZE_SCALEDOWN':4,'HALIGN_RIGHT':2}},'cafe':{'utility':{'ObjectUtil':{'MergeInto':lambda target,source:target.update(source)}}}}
def program(f,scale=3,colors=True,fog=True):
 a=Assembler();f(a);a.ret();p=a.finish();raw=bytearray(p.code)
 for offsets,value in [(p.scale_offsets,scale),(p.fog_offsets,int(fog)),(p.color_offsets,int(colors))]:
  for at in offsets:raw[at]=value
 return replace(p,code=bytes(raw))
checks=0
for scale in range(5):
 for colors in (False,True):
  for fog in (False,True):
   shared=style();rows=[{'value':v,'ExtraInfo':{'Max':99,'Min':0,'FocusValueTextStyle':shared}} for v in (74,83,89,21)]
   old=copy.deepcopy(shared);Machine(G,[]).run(program(prepare,scale,colors,fog),{7:rows})
   assert shared==old and [r['value'] for r in rows]==[74,83,89,21]
   for r in rows:
    e=r['ExtraInfo'];assert (e['Max'],e['Min'])==(99,0)
    if fog or colors:assert e['DEPlayerRating'] is True and isinstance(e['RightLabelDisplayOverride'],str)
   if not colors:assert all(r['ExtraInfo']['UnfocusValueTextStyle']['fontColor']==0xeeeeee for r in rows)
   checks+=len(rows)
for values in [[],[None], [{'value':-1,'ExtraInfo':{}}],[{'value':101,'ExtraInfo':{}}]]:
 Machine(G,[]).run(program(prepare),{7:values});checks+=1
for width,want in [(72,72),(0,60),(500,60),(16,16)]:
 extra=style();row={'Width':width,'display':'Great','ExtraInfo':extra};bonus={'display':'+ 1'};calls=[]
 def update(i,k,v):assert i==0 and k=='ExtraInfo';row[k]=v;calls.append((i,k))
 rec={'mSkillLevelDataProvider':{'GetItemData':lambda i:row if i==0 else bonus,'UpdateProperty':update}}
 p=program(fit_level);Machine(G,[]).run(p,{1:rec})
 assert row['ExtraInfo']['resizeWidth']==want and row['ExtraInfo']['fontColor']==0xeeeeee and bonus=={'display':'+ 1'}
 calls.clear();Machine(G,[]).run(p,{1:rec});assert not calls
 checks+=1
print(f'PASS {checks} presentation checks: all scales/toggles, native values/styles, sparse/sentinel inputs, bounds and no repeated title notifications')
