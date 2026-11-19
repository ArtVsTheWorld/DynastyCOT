"""Execute the shared legacy stats-card display path, with native source values intact."""
from pathlib import Path
from dataclasses import replace
import copy,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import Assembler,fnv
from build_player_card_ui import legacy_skill_owner_suffix,SPECS
from build_player_card_ui import badge_suffix,attribute_slots_suffix
from build_recruit_notes_ui import NativeFrameAssembler
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
mark=program(legacy_skill_owner_suffix);parent=program(badge_suffix)
cases=0
for scale in range(5):
 for fog,colors in [(True,True),(True,False),(False,True),(False,False)]:
  # Every boundary for default letters; representative boundaries of all
  # remaining scales, including Blind and the descriptive uppercase style.
  for value in (range(101) if scale==0 else [0,59,60,69,70,79,80,89,90,94,99,100]):
   for as_text in [False,True]:
    raw=str(value) if as_text else value
    source=[{'skillLabel':name,'skillValue':raw} for name in ['Speed','Agility','Awareness','Carrying','Trucking','Change of Direction']]
    before=copy.deepcopy(source)
    attrs={'mData':{'ExtraInfo':{f'AttributesData{i}':{'ratingName':s['skillLabel'],'ratingValue':s['skillValue'],'isFullyScouted':True} for i,s in enumerate(source)}},**{f'mcValueSlotText{i}':widget(raw) for i in range(6)}}
    info={'PlayerId':7,'roster':False,'overall':'A+','attributesSubpanelInfo':[None]+source,'injuryRisk':0,'SeasonHealth':88}
    module={'_visible':True,'mData':{'ExtraInfo':info},'mcInfoSlotAttributes':attrs}
    Machine(GLOBALS,[]).run(mark,{1:module})
    Machine(GLOBALS,[]).run(parent,{1:module})
    p=program(attribute_slots_suffix,scale,fog,colors)
    Machine(GLOBALS,[]).run(p,{1:attrs})
    expected=label(value,scale) if fog else str(value) if colors else raw
    assert all(attrs[f'mcValueSlotText{i}']['mData']['display']==expected for i in range(6)),(scale,fog,colors,value,as_text,expected,attrs['mcValueSlotText0']['mData']['display'])
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
 assert 'DELegacyPlayerSkillRatings' not in m['mData']['ExtraInfo']
for revealed in [False,UNDEFINED]:
 attrs={'mData':{'ExtraInfo':{'DEDepthChartPlayerRatings':True,'AttributesData0':{'isFullyScouted':revealed,'ratingValue':99}}},'mcValueSlotText0':widget('?')}
 Machine(GLOBALS,[]).run(program(attribute_slots_suffix),{1:attrs});assert attrs['mcValueSlotText0']['mData']['display']=='?'
for value in ['?',-1,101,'99%',None,'99.0','099',' 99','99 ']:
 attrs={'mData':{'ExtraInfo':{'DEDepthChartPlayerRatings':True,'AttributesData0':{'isFullyScouted':True,'ratingValue':value}}},'mcValueSlotText0':widget(value)}
 Machine(GLOBALS,[]).run(program(attribute_slots_suffix),{1:attrs});assert attrs['mcValueSlotText0']['mData']['display']==value
for receiver in [{},{'mcPlayerModule':{}},{'mcPlayerModule':{'_visible':True,'mData':{}}}]:Machine(GLOBALS,[]).run(mark,{1:receiver})
unmarked={'mData':{'ExtraInfo':{'AttributesData0':{'isFullyScouted':True,'ratingValue':99}}},'mcValueSlotText0':widget(99)}
Machine(GLOBALS,[]).run(program(attribute_slots_suffix),{1:unmarked});assert unmarked['mcValueSlotText0']['mData']['display']==99

# Prove ownership comes from a player DTO and six known skill names, not
# six convenient numbers. Stats/currency/unknown layouts remain native.
for changes in ['missing-id','stats-label','five-slots','hidden']:
    source=[{'skillLabel':n,'skillValue':80} for n in ['Speed','Agility','Awareness','Carrying','Trucking','Change Of Direction']]
    info={'PlayerId':9,'attributesSubpanelInfo':[None]+source};attrs={'mData':{'ExtraInfo':{'DELegacyPlayerSkillRatings':True}}}
    module={'_visible':True,'mData':{'ExtraInfo':info},'mcInfoSlotAttributes':attrs}
    if changes=='missing-id':info.pop('PlayerId')
    elif changes=='stats-label':source[0]['skillLabel']='Yards'
    elif changes=='five-slots':source.pop()
    else:module['_visible']=False
    info['attributesSubpanelInfo']=[None]+source
    Machine(GLOBALS,[]).run(mark,{1:module})
    assert attrs['mData']['ExtraInfo'].get('DELegacyPlayerSkillRatings') is not True,changes

# Existing QB captions and each position family use the same ownership rule.
for names in [('Throw Power','Short Accuracy','Medium Accuracy','Deep Accuracy','Throw on Run','Break Sack'),('Tackle','Hit Power','Pursuit','Man Coverage','Zone Coverage','Press'),('Kick Power','Kick Accuracy','Stamina','Toughness','Injury','Strength')]:
    attrs={'mData':{'ExtraInfo':{}}};source=[{'skillLabel':n,'skillValue':80} for n in names]
    module={'_visible':True,'mData':{'ExtraInfo':{'PlayerId':10,'attributesSubpanelInfo':[None]+source}},'mcInfoSlotAttributes':attrs}
    Machine(GLOBALS,[]).run(mark,{1:module})
    assert attrs['mData']['ExtraInfo'].get('DELegacyPlayerSkillRatings') is True,names

# The captured producer is indexed native data, without an Array.length
# member. Its receiving caption is exactly "Catch in Traff". A length check
# or full-name-only vocabulary used to reject this valid player context.
class NativeSkills(dict):
    def get(self,key,default=None):
        assert key!='length','Native skill container does not expose Array.length'
        assert not isinstance(key,int) or 1<=key<=6,'Only six displayed slots may be inspected'
        return super().get(key,default)
for count in [6,8,32,100]:
    source=[{'skillLabel':n,'skillValue':80} for n in ['Speed','Agility','Awareness','Medium Route Running','Catching','Catch in Traff']]
    source += [{'skillLabel':'Unused background data','skillValue':81} for _ in range(count-6)]
    skills=NativeSkills({i+1:v for i,v in enumerate(source)})
    attrs={'mData':{'ExtraInfo':{f'AttributesData{i}':{'ratingName':v['skillLabel'],'ratingValue':v['skillValue'],'isFullyScouted':True} for i,v in enumerate(source[:6])}},**{f'mcValueSlotText{i}':widget(80) for i in range(6)}}
    info={'PlayerId':7,'attributesSubpanelInfo':skills}
    module={'_visible':True,'mData':{'ExtraInfo':info},'mcInfoSlotAttributes':attrs}
    for _ in range(2):
        Machine(GLOBALS,[]).run(mark,{1:module})
        assert attrs['mData']['ExtraInfo'].get('DELegacyPlayerSkillRatings') is True,count
        Machine(GLOBALS,[]).run(program(attribute_slots_suffix),{1:attrs})
        assert all(attrs[f'mcValueSlotText{i}']['mData']['display']==label(80,0) for i in range(6))
    assert len(source)==count and all(v['skillValue'] in (80,81) for v in source)
for count in [0,5]:
    source=NativeSkills({i+1:{'skillLabel':'Speed','skillValue':80} for i in range(count)})
    attrs={'mData':{'ExtraInfo':{'DELegacyPlayerSkillRatings':True}}}
    module={'_visible':True,'mData':{'ExtraInfo':{'PlayerId':7,'attributesSubpanelInfo':source}},'mcInfoSlotAttributes':attrs}
    Machine(GLOBALS,[]).run(mark,{1:module})
    assert attrs['mData']['ExtraInfo'].get('DELegacyPlayerSkillRatings') is False,count
# The same check follows the native three-register page-change callback.
# Logical scratch uses invocation-local variables, never an expanded frame.
page=NativeFrameAssembler(3);legacy_skill_owner_suffix(page);page_program=page.finish()
assert page.max_register<3
source=NativeSkills({i+1:{'skillLabel':n,'skillValue':80} for i,n in enumerate(['Speed','Agility','Awareness','Throw Power','Short Acc','Mid Acc'])})
attrs={'mData':{'ExtraInfo':{}}};module={'_visible':True,'mData':{'ExtraInfo':{'PlayerId':7,'attributesSubpanelInfo':source}},'mcInfoSlotAttributes':attrs}
Machine(GLOBALS,[]).run(page_program,{1:module})
assert attrs['mData']['ExtraInfo']['DELegacyPlayerSkillRatings'] is True
# A native rebuild may give the child a new ExtraInfo object. Reopening its
# page must mark that current object, never a stale previous player's object.
old_extra=attrs['mData']['ExtraInfo'];attrs['mData']['ExtraInfo']={}
Machine(GLOBALS,[]).run(page_program,{1:module})
assert attrs['mData']['ExtraInfo']['DELegacyPlayerSkillRatings'] is True and old_extra is not attrs['mData']['ExtraInfo']

print('PASS native 1-based producer without Array.length, verified truncated caption, six-slot bound and repeated page publication; source records unchanged.')

sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
from inspect_tb_ast import load
asset,specs=SPECS['controlPrograms'];apt=load(Path(asset))
for off,argc,regs,flags,size,expected,newregs,suffix,copy_bytes in specs[:1]:
 assert fnv(apt.data[off+48:off+48+size])==expected
 assert regs==newregs==23 and argc==0 and flags==25
 assert len(mark.code)+size<32768
assert len(program(attribute_slots_suffix).code)+388<32768
print(f'PASS {cases} shared legacy stats-card policies: six numeric/text attributes, all grade scales, colors/fog toggles, redraws, hidden/invalid values, exact screen fingerprint and untouched skills/health/Overall; no game access.')
