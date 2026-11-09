"""Native circle geometry, independent grade modes, reuse and no-op repaints."""
import sys,math
from pathlib import Path
from dataclasses import replace
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import Assembler
from build_rating_badge_fit_ui import fit_badge
from recruit_notes_vm import Machine

def program(player=3,team=3):
    a=Assembler();fit_badge(a);a.ret();p=a.finish();raw=bytearray(p.code)
    for at in p.scale_offsets:raw[at]=player
    for at in p.team_scale_offsets:raw[at]=team
    return replace(p,code=bytes(raw))
G={'art_styleguide':{'Font':{'RESIZE_SCALEDOWN':4,'HALIGN_CENTER':1}}}
checks=0
for team in (False,True):
 for width in (48,72,100,128):
  for word in ('POOR','FAIR','GOOD','GREAT','ELITE'):
   extra={'fontColor':0x529ff0,'font':'native','size':32};calls=[]
   def set_extra(value):extra.update(value);calls.append(value)
   child={'mData':{'display':word,'ExtraInfo':extra},'SetExtraInfo':set_extra}
   source={'value':83,'display':'OVR','ExtraInfo':{'DETeamRating':team}}
   badge={'mData':source,'mcValue':child,'mcOutlineSmall':{'_visible':True,'_width':width},'mcOutlineLarge':{'_visible':False,'_width':200}}
   Machine(G,[]).run(program(),{1:badge})
   assert math.isclose(extra['resizeWidth'],width*.74) and extra['hAlign']==1 and extra['maxNumLines']==1
   assert extra['fontColor']==0x529ff0 and extra['font']=='native' and extra['size']==32 and source['value']==83
   calls.clear();Machine(G,[]).run(program(),{1:badge});assert not calls
   badge['mcOutlineSmall']['_width']=width+10;Machine(G,[]).run(program(),{1:badge});assert math.isclose(extra['resizeWidth'],(width+10)*.74)
   calls.clear();Machine(G,[]).run(program(0,0),{1:badge});assert not calls
   checks+=5
for label in ('83','A+','?','$40K','Elite player'):
 calls=[];extra={};child={'mData':{'display':label,'ExtraInfo':extra},'SetExtraInfo':calls.append}
 Machine(G,[]).run(program(),{1:{'mData':{'ExtraInfo':{}},'mcValue':child,'mcOutlineSmall':{'_visible':True,'_width':100}}});assert not calls
for width in (0,20,513,float('nan'),None,'100'):
 calls=[];child={'mData':{'display':'GREAT','ExtraInfo':{}},'SetExtraInfo':calls.append}
 Machine(G,[]).run(program(),{1:{'mData':{'ExtraInfo':{}},'mcValue':child,'mcOutlineSmall':{'_visible':True,'_width':width}}});assert not calls
print(f'PASS {checks} circle-fit checks plus non-grade/invalid geometry rejection; fonts, colors and numeric models preserved')
