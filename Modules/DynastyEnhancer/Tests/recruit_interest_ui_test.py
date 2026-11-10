"""Execute emitted recruiting suffixes in the existing isolated UI interpreter."""
from pathlib import Path
import sys,copy
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_interest_ui import Assembler,interest_row,progress,cutoff,scout_row,target_cell,small_card
from recruit_notes_vm import Machine
checks=0
styles={"art_styleguide":{"Font":{"HALIGN_LEFT":"left","HALIGN_CENTER":"center","RESIZE_SCALEDOWN":"scale","RESIZE_NONE":"none"}}}

def program(fn,mode):
 a=Assembler();fn(a);p=a.finish();code=bytearray(p.code)
 for at in p.scale_offsets:code[at]=mode
 p.code=bytes(code);return p

def widget(text='',x=10,width=30,visible=True):
 w={'mData':{'display':text,'ExtraInfo':{}},'_x':x,'_width':width,'_visible':visible}
 w['SetLabel']=lambda value:w['mData'].update(display=value)
 def extra(e):
  w['mData']['ExtraInfo'].update(e)
  if 'resizeWidth' in e:w['_width']=e['resizeWidth']
 w['SetExtraInfo']=extra;return w
for mode in range(4):
 for value,label in [(90,'FAVORITE'),(70,'CONTENDER'),(50,'IN THE MIX'),(30,'OUTSIDE LOOKING IN'),(10,'LONG SHOT'),(0,'INTEREST UNKNOWN')]:
  r={'mcPosition':widget('3'),'mcTopSlider':widget(x=100),'mcUpArrow':widget(),'mcDownArrow':widget(visible=False),'mData':{'ExtraInfo':{'SliderValue':value}}}
  Machine(styles,[]).run(program(interest_row,mode),{1:r})
  assert r['mcTopSlider']['_visible']==(mode==0)
  assert r['mcPosition']['_visible']==(mode!=3)
  if mode==1:assert r['mcPosition']['mData']['display']==label and r['mcPosition']['_x']==100 and r['mcPosition']['mData']['ExtraInfo']['hAlign']=='left'
  if mode==3:assert not r['mcUpArrow']['_visible'] and not r['mcDownArrow']['_visible']
  checks+=3
r={'mcPosition':widget('3'),'mcTopSlider':widget(x=100),'mcUpArrow':widget(),'mcDownArrow':widget(visible=False),'mData':{'ExtraInfo':{'SliderValue':90}}}
Machine(styles,[]).run(program(interest_row,1),{1:r});r['mcPosition']['SetLabel']('3');Machine(styles,[]).run(program(interest_row,0),{1:r});assert r['mcPosition']['_x']==10 and r['mcPosition']['_width']==30 and r['mcPosition']['mData']['display']=='3';checks+=1
r={'mcTopSlider':widget(),'mcTopSliderMask':widget(visible=False)}
Machine(styles,[]).run(program(progress,1),{1:r});assert not r['mcTopSlider']['_visible'];Machine(styles,[]).run(program(progress,0),{1:r});assert r['mcTopSlider']['_visible'] and not r['mcTopSliderMask']['_visible'];checks+=2
loc={'loc':{'career':{'Recruiting':{'PROJECTED_CUTOFF':'Projected Cutoff','LOCKED_OUT':'Locked Out'}}}}
# Exercise the actual model provider, preserving school data and lockout facts.
for mode in range(4):
 for count in [0,5,10,31]:
  schools=[{'display':'School '+str(i),'ExtraInfo':{'IsBelowCutoff':i>4}} for i in range(count)]
  divider={'display':'Projected Cutoff','ExtraInfo':{'height':10}}
  locked={'display':'Locked Out','ExtraInfo':{'height':10}}
  rows=schools[:3]+[divider]+schools[3:]+[locked] if count<31 else schools
  original=list(rows);removed=[]
  def remove(i):removed.append(rows.pop(i))
  provider={'GetNumItems':lambda:len(rows),'GetItemData':lambda i:rows[i],'RemoveItem':remove}
  r={'mTopSchoolItemProvider':provider}
  Machine(loc,[]).run(program(cutoff,mode),{1:r})
  expected=[v for v in original if v is not divider] if mode in [1,3] else original
  assert rows==expected and locked in rows if count<31 else rows==expected
  assert all(any(v is school for v in rows) for school in schools)
  Machine(loc,[]).run(program(cutoff,mode),{1:r});assert rows==expected
  checks+=3
for invalid in [-1,33,'10',float('nan')]:
 called=[];provider={'GetNumItems':lambda:invalid,'GetItemData':lambda i:called.append(i),'RemoveItem':lambda i:called.append(i)}
 Machine(loc,[]).run(program(cutoff,1),{1:{'mTopSchoolItemProvider':provider}});assert not called;checks+=1
for mode_index,mode in enumerate(['Native','Feedback','RankOnly','Blind']):
 r={'mData':{'ExtraInfo':{'BTFPreviouslyScouted':True,'BTFInterestMode':mode}},'mcName':widget('S. Edwards'),'mcUserRanking':widget('3')}
 for i in range(4):Machine(styles,[]).run(program(scout_row,mode_index),{1:r});assert r['mcName']['mData']['display']=='S. Edwards';checks+=1
 assert r['mcUserRanking']['_visible']==(mode in ['Native','RankOnly']);checks+=1
for fn in [interest_row,progress,cutoff,scout_row,target_cell,small_card]:
 Machine(loc,[]).run(program(fn,1),{1:{}});checks+=1

for percent in range(10,101,10):
 calls=[];icon=widget();icon.update(SetValue=lambda value:calls.append(value),GetWidth=lambda:25,GetHeight=lambda:25)
 dynas={'LIBRARY_ID':99};dynas.update({'ASSETID_DYNAS_GENERAL_SCOUTING'+('' if n==100 else '_'+str(n)):n for n in range(10,101,10)})
 g={'Number':lambda value:float(value),'shared_enum':{'fblibraries':{'global':{'DynastyIcons':dynas}}},'cafe':{'utility':{'ColorUtil':{'ResetColor':lambda icon:None}}}}
 r={'mcIcon':icon,'mWidth':60,'mHeight':40};Machine(g,[]).run(program(target_cell,0),{1:r,4:{'display':'TargetNone,BTFScouted,'+str(percent)}})
 assert calls==[percent] and icon['mData']['ExtraInfo']['AssetLibrary']==99 and icon['_x']==17.5 and icon['_y']==7.5;checks+=1
for display in ['TargetNone','TargetTargeted','Target,61','TargetNone,Other,40','TargetNone,BTFScouted,999','TargetNone,BTFScouted,-10']:
 r={'mcIcon':widget()};Machine({},[]).run(program(target_cell,0),{1:r,4:{'display':display}});checks+=1

for mode in range(4):
 r={'mData':{'ExtraInfo':{'IsCommited':False,'IsSigned':False,'SliderMaxValue':100.125}},'mcTopSlider':widget(),'mcNumberLabel':widget(),'mcSlotDivider':widget(),'mcSlotDividerDotted':widget()}
 for i,value in enumerate([90,70,50,30,10]):
  r['mData']['ExtraInfo']['SchoolSlot'+str(i)]={'value':value}
  r['mcTopNumber'+str(i)]=widget(str(i+1),x=10,width=15)
  r['mcInfluenceSlider'+str(i)]=widget(x=50,width=145);r['mcInfluenceSlider'+str(i)]['GetWidth']=lambda:145
  r['mcArrowUp'+str(i)]=widget();r['mcArrowDown'+str(i)]=widget(visible=False)
 Machine(styles,[]).run(program(small_card,mode),{1:r})
 assert r['mcTopSlider']['_visible']==(mode==0);checks+=1
 for i,label in enumerate(['FAVORITE','CONTENDER','IN THE MIX','OUTSIDE LOOKING IN','LONG SHOT']):
  number=r['mcTopNumber'+str(i)];bar=r['mcInfluenceSlider'+str(i)]
  assert bar['_visible']==(mode==0) and number['_visible']==(mode!=3);checks+=1
  if mode==1:assert number['mData']['display']==label and number['_x']==50 and number['mData']['ExtraInfo']['resizeWidth']==145;checks+=1
  if mode==3:assert not r['mcArrowUp'+str(i)]['_visible'] and not r['mcArrowDown'+str(i)]['_visible'];checks+=1
 if mode in [1,2,3]:
  Machine(styles,[]).run(program(small_card,0),{1:r});assert r['mcTopSlider']['_visible'] and r['mcTopNumber0']['_x']==10 and r['mcInfluenceSlider0']['_visible'];checks+=1
# Real card shape: the slot carries value/extravalue, not sliderMaxValue.
# Exercise absolute and normalized native bar values, tied leaders and no data.
for values,labels in [
 ([100,92,65,30,0],['FAVORITE','FAVORITE','IN THE MIX','OUTSIDE LOOKING IN','LONG SHOT']),
 (['100','92','65','30','0'],['FAVORITE','FAVORITE','IN THE MIX','OUTSIDE LOOKING IN','LONG SHOT']),
 ([1,.92,.65,.3,0],['FAVORITE','FAVORITE','IN THE MIX','OUTSIDE LOOKING IN','LONG SHOT']),
 ([0,0,0,0,0],['INTEREST UNKNOWN']*5),
 ([90,70,50,30,float('nan')],['FAVORITE','CONTENDER','IN THE MIX','OUTSIDE LOOKING IN','INTEREST UNKNOWN'])]:
 r={'mData':{'ExtraInfo':{'IsCommited':False,'IsSigned':False,'SliderMaxValue':825}},'mcTopSlider':widget(),'mcNumberLabel':widget(),'mcSlotDivider':widget(),'mcSlotDividerDotted':widget()}
 for i,value in enumerate(values):
  r['mData']['ExtraInfo']['SchoolSlot'+str(i)]={'value':value,'extravalue':0,'IsEmpty':False}
  r['mcTopNumber'+str(i)]=widget(str(i+1),x=10,width=15)
  r['mcInfluenceSlider'+str(i)]=widget(x=50,width=145);r['mcInfluenceSlider'+str(i)]['GetWidth']=lambda:145
  r['mcArrowUp'+str(i)]=widget();r['mcArrowDown'+str(i)]=widget(visible=False)
 Machine(styles,[]).run(program(small_card,1),{1:r})
 for i,label in enumerate(labels):
  assert r['mcTopNumber'+str(i)]['mData']['display']==label
  assert not r['mcInfluenceSlider'+str(i)]['_visible'];checks+=1
print('PASS:',checks,'emitted UI checks: all tiers, Native/Rank Only/Feedback/Blind, hidden bars/ranks/arrows, restored widths/progress/cutoffs, unchanged names across redraw and missing-widget guards. No game access.')
