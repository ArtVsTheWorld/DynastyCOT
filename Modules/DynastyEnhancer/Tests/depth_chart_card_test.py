"""Execute the pause depth-card display path, with native source values intact."""
from pathlib import Path
from dataclasses import replace
import copy,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import Assembler,fnv
from build_player_ingame_ui import depth_module_suffix,DEPTH_SPECS
from build_player_card_ui import badge_suffix,attribute_slots_suffix
from grade_scales import label
from recruit_notes_vm import Machine,UNDEFINED

def program(suffix,scale=0,fog=True,colors=True):
 a=Assembler();suffix(a);p=a.finish();code=bytearray(p.code)
 for offsets,value in [(p.scale_offsets,scale),(p.fog_offsets,int(fog)),(p.color_offsets,int(colors))]:
  for at in offsets:code[at]=value
 return replace(p,code=bytes(code))
def widget(value):
 w={'mData':{'display':value,'ExtraInfo':{'fontColor':0xFFFFFF,'font':'native'}}}
 w['SetLabel']=lambda value:w['mData'].update(display=value)
 w['SetExtraInfo']=lambda patch:w['mData']['ExtraInfo'].update(patch)
 return w

GLOBALS={'art_styleguide':{'Font':{'RESIZE_SCALEDOWN':'scaledown'}}}
mark=program(depth_module_suffix);parent=program(badge_suffix)
cases=0
for scale in range(5):
 for fog,colors in [(True,True),(True,False),(False,True),(False,False)]:
  # Every boundary for default letters; representative boundaries of all
  # remaining scales, including Blind and the descriptive uppercase style.
  for value in (range(101) if scale==0 else [0,59,60,69,70,79,80,89,90,94,99,100]):
   for as_text in [False,True]:
    raw=str(value) if as_text else value
    source=[{'skillLabel':name,'skillValue':raw} for name in ['Speed','Agility','Awareness','Throw Power','Short Acc','Mid Acc']]
    before=copy.deepcopy(source)
    attrs={'mData':{'ExtraInfo':{f'AttributesData{i}':{'ratingName':s['skillLabel'],'ratingValue':s['skillValue'],'isFullyScouted':True} for i,s in enumerate(source)}},**{f'mcValueSlotText{i}':widget(raw) for i in range(6)}}
    info={'PlayerId':7,'roster':False,'overall':'A+','attributesSubpanelInfo':source,'injuryRisk':0,'SeasonHealth':88}
    module={'_visible':True,'mData':{'ExtraInfo':info},'mcInfoSlotAttributes':attrs}
    Machine(GLOBALS,[]).run(mark,{1:{'mcPlayerModule':module}})
    Machine(GLOBALS,[]).run(parent,{1:module})
    p=program(attribute_slots_suffix,scale,fog,colors)
    Machine(GLOBALS,[]).run(p,{1:attrs})
    expected=label(value,scale) if fog else str(value) if colors else raw
    assert all(attrs[f'mcValueSlotText{i}']['mData']['display']==expected for i in range(6)),(scale,fog,colors,value,as_text)
    assert source==before and info['overall']=='A+' and info['roster'] is False
    assert info['injuryRisk']==0 and info['SeasonHealth']==88
    # Native redraw/reset and a recycled widget must apply the current policy,
    # never a cached grade for another player, and keep native colors when off.
    for i in range(6):attrs[f'mcValueSlotText{i}']['mData'].update(display=raw,ExtraInfo={'fontColor':0x112233,'font':'native'})
    Machine(GLOBALS,[]).run(p,{1:attrs})
    assert attrs['mcValueSlotText0']['mData']['display']==expected
    if not colors:assert attrs['mcValueSlotText0']['mData']['ExtraInfo']['fontColor']==0x112233
    cases+=1
for visible in [False,UNDEFINED]:
 m={'_visible':visible,'mData':{'ExtraInfo':{}},'mcInfoSlotAttributes':{}}
 Machine(GLOBALS,[]).run(mark,{1:{'mcPlayerModule':m}})
 assert 'DEDepthChartPlayerRatings' not in m['mData']['ExtraInfo']
for revealed in [False,UNDEFINED]:
 attrs={'mData':{'ExtraInfo':{'DEDepthChartPlayerRatings':True,'AttributesData0':{'isFullyScouted':revealed,'ratingValue':99}}},'mcValueSlotText0':widget('?')}
 Machine(GLOBALS,[]).run(program(attribute_slots_suffix),{1:attrs});assert attrs['mcValueSlotText0']['mData']['display']=='?'
for value in ['?',-1,101,'99%',None,'99.0','099',' 99','99 ']:
 attrs={'mData':{'ExtraInfo':{'DEDepthChartPlayerRatings':True,'AttributesData0':{'isFullyScouted':True,'ratingValue':value}}},'mcValueSlotText0':widget(value)}
 Machine(GLOBALS,[]).run(program(attribute_slots_suffix),{1:attrs});assert attrs['mcValueSlotText0']['mData']['display']==value
for receiver in [{},{'mcPlayerModule':{}},{'mcPlayerModule':{'_visible':True,'mData':{}}}]:Machine(GLOBALS,[]).run(mark,{1:receiver})
unmarked={'mData':{'ExtraInfo':{'AttributesData0':{'isFullyScouted':True,'ratingValue':99}}},'mcValueSlotText0':widget(99)}
Machine(GLOBALS,[]).run(program(attribute_slots_suffix),{1:unmarked});assert unmarked['mcValueSlotText0']['mData']['display']==99

sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
from inspect_tb_ast import load
apt=load(Path('Evidence/PlayerFog/madden_scrdepthchartselectplayer.ast'))
for off,argc,regs,flags,size,expected,newregs,suffix in DEPTH_SPECS:
 assert fnv(apt.data[off+48:off+48+size])==expected
 assert regs==newregs==21 and argc==1 and flags==297
 assert len(mark.code)+size<32768
assert len(program(attribute_slots_suffix).code)+388<32768
print(f'PASS {cases} pause depth-card policies: six numeric/text attributes, all grade scales, colors/fog toggles, redraws, hidden/invalid values, exact screen fingerprint and untouched skills/health/Overall; no game access.')
