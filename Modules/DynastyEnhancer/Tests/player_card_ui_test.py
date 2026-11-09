"""Execute custom presentation suffixes; preserve native model/progression values."""
import copy
import math
from pathlib import Path
import sys
from dataclasses import replace
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import Assembler, GRADE_COLORS
from build_player_card_ui import (header_suffix,badge_suffix,attributes_suffix,
                                 template_suffix,group_suffix,level_suffix,attribute_slots_suffix,
                                 skill_slots_suffix,rating_value_draw_suffix,subskills_suffix,
                                 generic_slider_draw_suffix,SPECS,append_program,fnv)
from recruit_notes_vm import Machine,UNDEFINED


def program(suffix):
    a=Assembler();suffix(a);a.ret();return a.finish()


for marked in [False,True]:
    for grade in ['A','B','C','D','F','?','85',85,None]:
        target={'mData':{'display':'native','ExtraInfo':{'fontColor':0xFFFFFF,'font':'native'}}}
        target['SetLabel']=lambda value:target['mData'].update(display=value)
        target['SetExtraInfo']=lambda extra:target['mData']['ExtraInfo'].update(extra)
        data={'value':4,'ExtraInfo':{'DEPlayerRating':marked,'RightLabelDisplayOverride':grade,'MinValue':0,'MaxValue':100}}
        before=copy.deepcopy(data)
        Machine({},[]).run(program(generic_slider_draw_suffix),{1:{'mData':data,'mcRightValue':target}})
        assert data==before, 'Shared slider model was changed'
        if marked and ((isinstance(grade,str) and grade in GRADE_COLORS) or grade in ['85',85]):
            assert target['mData']['display']==grade and target['mData']['ExtraInfo']['fontColor']==GRADE_COLORS.get(grade,GRADE_COLORS['B'])
        else:assert target['mData']['display']=='native' and target['mData']['ExtraInfo']['fontColor']==0xFFFFFF
        assert target['mData']['ExtraInfo']['font']=='native'


def text_widget(label='old',color=0xFFFFFF):
    target={'mData':{'display':label,'ExtraInfo':{'fontColor':color,'font':'preserve','size':24}},'calls':[]}
    def set_label(value):target['mData']['display']=value;target['calls'].append(('label',value))
    def set_style(value):target['mData']['ExtraInfo'].update(value);target['calls'].append(('style',value))
    target.update(SetLabel=set_label,SetExtraInfo=set_style)
    return target


def expected(value):
    return 'A' if value>=90 else 'B' if value>=80 else 'C' if value>=70 else 'D' if value>=60 else 'F'


for value in range(101):
    child=text_widget(str(value));data={'value':value,'unrelated':17}
    item={'mData':data,'mcValue':child}
    receiver={'mcRatingValueItem':item,'mOVRValue':value,'delta':3}
    Machine({},[]).run(program(header_suffix),{1:receiver})
    grade=expected(value)
    assert data=={'value':value,'unrelated':17,'ExtraInfo':{'DEPlayerRating':True}} and receiver['mOVRValue']==value and receiver['delta']==3
    assert child['mData']['display']==grade and child['mData']['ExtraInfo']['fontColor']==GRADE_COLORS[grade]
    child['mData']['ExtraInfo']['fontColor']=0xFFFFFF
    Machine({},[]).run(program(rating_value_draw_suffix),{1:item})
    assert child['mData']['ExtraInfo']['fontColor']==GRADE_COLORS[grade] and data['value']==value
    assert child['mData']['ExtraInfo']['font']=='preserve' and child['mData']['ExtraInfo']['size']==24
    child['calls'].clear();Machine({},[]).run(program(header_suffix),{1:receiver})
    assert child['calls']==[], 'Unchanged draw reapplied styles or label'
    # Native reentry/focus can restore white and the numeric text. Restore both.
    child['mData']['display']=str(value);child['mData']['ExtraInfo']['fontColor']=0xFFFFFF
    Machine({},[]).run(program(header_suffix),{1:receiver})
    assert child['mData']['display']==grade and child['mData']['ExtraInfo']['fontColor']==GRADE_COLORS[grade]

for value in [UNDEFINED,None,'?','--','Hidden','92',-1,101,float('nan'),float('inf')]:
    child=text_widget('?');receiver={'mcRatingValueItem':{'mData':{'value':value},'mcValue':child}}
    Machine({},[]).run(program(header_suffix),{1:receiver})
    assert child['calls']==[],'Hidden/invalid rating was disclosed'
for receiver in [{},{'mcRatingValueItem':{}},{'mcRatingValueItem':{'mData':{'value':90}}}]:
    Machine({},[]).run(program(header_suffix),{1:receiver})

for value in ['A','B','C','D','F',92,0,45000,'NIL','B+','C–A','?',None]:
    child=text_widget(value);item={'mData':{'value':value},'mcValue':child}
    Machine({},[]).run(program(badge_suffix),{1:{'mcStat':item}})
    if isinstance(value,str) and value in GRADE_COLORS:
        assert child['mData']['ExtraInfo']['fontColor']==GRADE_COLORS[value]
    else:assert child['calls']==[], 'Shared module recolored an unrelated statistic or value'
    assert item['mData']['value']==value

for marked in [True,False]:
    for current,maximum in [(59,90),(60,79),(80,100),(0,0),(UNDEFINED,UNDEFINED),('?',-1)]:
        child=text_widget(current);max_child=text_widget(maximum)
        data={'value':current,'ExtraInfo':{'DEPlayerRating':marked,'MaxValue':maximum,'ShowMaxValue':False,'ShowSlider':False},'sliderValue':current}
        receiver={'mData':data,'mcValue':child,'mcMaxValue':max_child}
        before_extra=dict(data['ExtraInfo'])
        Machine({},[]).run(program(attributes_suffix),{1:receiver})
        assert data['ExtraInfo']==before_extra and data['value']==current and data['sliderValue']==current
        for value,target in [(current,child),(maximum,max_child)]:
            if marked and isinstance(value,int) and 0<=value<=100:
                grade=expected(value);assert target['mData']['display']==grade and target['mData']['ExtraInfo']['fontColor']==GRADE_COLORS[grade]
            else:assert target['calls']==[]

globals_={'fb_shared_enum':{'BindingType':{'ATTRIBUTES':'attributes','INFO':'info'}}}
for binding in ['attributes','skills','stats']:
    for kind in ['info','container']:
        data={'ExtraInfo':{'Width':150,'ShowSlider':False}}
        result=Machine(globals_,[]).run(program(template_suffix),{1:data,2:binding,3:kind})
        assert result is data
        assert data['ExtraInfo'].get('DEPlayerRating',False)==(binding=='attributes' and kind=='info')
        assert data['ExtraInfo']['Width']==150 and data['ExtraInfo']['ShowSlider'] is False

for rating in [0,59,60,79,80,90,100]:
    styles={}
    for name in ['FocusValueTextStyle','UnfocusValueTextStyle','UnfocusableValueTextStyle']:
        style={'font':'native-font','fontColor':0xFFFFFF,'size':28}
        style['CloneAndModify']=lambda changes,original=style:{**original,**changes}
        styles[name]=style
    clip={'mData':{'value':4,'ExtraInfo':styles},'calls':[]}
    def update(value):clip['calls'].append(value);clip['mData']['ExtraInfo'].update(value)
    clip['SetExtraInfo']=update
    data={'ExtraInfo':{'Rating':rating,'Level':4,'MaxSkillGroupCap':10}}
    old_data=copy.deepcopy(data)
    Machine({},[]).run(program(group_suffix),{1:data,2:clip})
    assert data==old_data and clip['mData']['value']==4
    assert clip['mData']['ExtraInfo']['RightLabelDisplayOverride']==expected(rating)
    for name in ['FocusValueTextStyle','UnfocusValueTextStyle','UnfocusableValueTextStyle']:
        style=clip['mData']['ExtraInfo'][name]
        assert style['fontColor']==GRADE_COLORS[expected(rating)] and style['font']=='native-font' and style['size']==28
    clip['calls'].clear()
    Machine({},[]).run(program(group_suffix),{1:data,2:clip})
    assert clip['calls']==[], 'Unchanged skill group reapplied native styles'
    for name in ['FocusValueTextStyle','UnfocusValueTextStyle','UnfocusableValueTextStyle']:
        clip['mData']['ExtraInfo'][name]['fontColor']=0xFFFFFF
        Machine({},[]).run(program(group_suffix),{1:data,2:clip})
        assert clip['mData']['ExtraInfo'][name]['fontColor']==GRADE_COLORS[expected(rating)]
    updates=[];old_extra={'font':'native-title','hAlign':'right','fontColor':0xFFFFFF}
    old_extra['CloneAndModify']=lambda patch,original=old_extra:{**original,**patch}
    levels=[{'display':rating,'ExtraInfo':old_extra},{'display':3,'ExtraInfo':{'fontColor':0x00FF00}}]
    bonus=copy.deepcopy(levels[1])
    def level_update(index,key,value):updates.append((index,key));levels[index][key]=value
    receiver={'mSkillLevelDataProvider':{'UpdateProperty':level_update,'GetItemData':lambda index:levels[index]}}
    globals_={'cafe':{'utility':{'ObjectUtil':{'MergeInto':lambda target,source:target.update(source)}}}}
    Machine(globals_,[]).run(program(level_suffix),{1:receiver,4:rating,5:3})
    assert updates==[(0,'display'),(0,'ExtraInfo')] and levels[0]['display']==expected(rating)
    assert levels[1]==bonus and old_extra['fontColor']==0xFFFFFF, 'Upgrade bonus or shared style was altered'
    assert levels[0]['ExtraInfo']['fontColor']==GRADE_COLORS[expected(rating)]
    assert levels[0]['ExtraInfo']['font']=='native-title' and levels[0]['ExtraInfo']['hAlign']=='right'
    updates.clear()
    Machine(globals_,[]).run(program(level_suffix),{1:receiver,4:rating,5:3})
    assert updates==[], 'Unchanged skill title notified native data provider'
    levels[0]['display']=rating
    Machine(globals_,[]).run(program(level_suffix),{1:receiver,4:rating,5:3})
    assert updates==[(0,'display')], 'Native title reset did not repaint exactly once'
    updates.clear();levels[0]['ExtraInfo']['fontColor']=0xFFFFFF
    Machine(globals_,[]).run(program(level_suffix),{1:receiver,4:rating,5:3})
    assert updates==[(0,'ExtraInfo')] and levels[1]==bonus

# Native display listeners can replace the item while handling UpdateProperty.
# Style the current item, retaining that listener's unrelated font/layout data.
replacement_style={'font':'native-rebound','fontColor':0xFFFFFF,'size':30}
replacement_style['CloneAndModify']=lambda patch:{**replacement_style,**patch}
rebound=[{'display':64,'ExtraInfo':{'fontColor':0xFFFFFF}}]
def replace_on_title(index,key,value):
    if key=='display':rebound[0]={'display':value,'ExtraInfo':replacement_style}
    else:rebound[0][key]=value
receiver={'mSkillLevelDataProvider':{'UpdateProperty':replace_on_title,'GetItemData':lambda index:rebound[index]}}
Machine({},[]).run(program(level_suffix),{1:receiver,4:64,5:3})
assert rebound[0]['display']=='D' and rebound[0]['ExtraInfo']['font']=='native-rebound'
assert rebound[0]['ExtraInfo']['fontColor']==GRADE_COLORS['D'] and rebound[0]['ExtraInfo']['size']==30

for revealed in [True,False]:
    widgets=[text_widget() for _ in range(6)]
    entries=[{'ratingValue':grade,'isFullyScouted':revealed,'ratingName':'Speed'} for grade in ['A','B','C','D','F','?']]
    receiver={'mData':{'ExtraInfo':{'AttributesData'+str(i):entry for i,entry in enumerate(entries)}}}
    receiver.update({'mcValueSlotText'+str(i):widget for i,widget in enumerate(widgets)})
    before=copy.deepcopy(entries)
    Machine({},[]).run(program(attribute_slots_suffix),{1:receiver})
    assert entries==before
    for grade,widget in zip(['A','B','C','D','F','?'],widgets):
        if revealed and grade!='?':assert widget['mData']['ExtraInfo']['fontColor']==GRADE_COLORS.get(grade,GRADE_COLORS['B'])
        else:assert widget['calls']==[]
for rating in [59,70,90]:
    entry={'Rating':rating,'Level':3,'SkillGroupCap':8,'MaxSkillGroupCap':10}
    gauge={'mData':{'value':3,'ExtraInfo':{}},'calls':[]}
    def apply_gauge(extra):gauge['calls'].append(extra);gauge['mData']['ExtraInfo'].update(extra)
    gauge['SetExtraInfo']=apply_gauge
    receiver={'mData':{'ExtraInfo':{'SkillData0':entry}},'mcSkill0':gauge}
    before=copy.deepcopy(entry)
    Machine({},[]).run(program(skill_slots_suffix),{1:receiver})
    assert entry==before and gauge['mData']['value']==3
    assert gauge['calls']==[{'RightLabelDisplayOverride':expected(rating),'DEPlayerRating':True}]
    gauge['calls'].clear();Machine({},[]).run(program(skill_slots_suffix),{1:receiver})
    assert gauge['calls']==[], 'Unchanged six-slot rating renderer reapplied its style'
for suffix in [attribute_slots_suffix,skill_slots_suffix]:
    for receiver in [{},{'mData':{}},{'mData':{'ExtraInfo':{}}}]:Machine({},[]).run(program(suffix),{1:receiver})

for rating in range(101):
    style={'font':'native','size':26,'fontColor':0xFFFFFF}
    style['CloneAndModify']=lambda patch,original=style:{**original,**patch}
    original_extra={'UnfocusValueTextStyle':style,'MinValue':0,'MaxValue':100,'IncreaseValue':2,'CanIncrease':True}
    rows=[{'value':rating,'display':'Speed','ExtraInfo':original_extra}];updates=[]
    def update(i,key,value):rows[i][key]=value;updates.append((i,key))
    provider={'GetNumItems':lambda:len(rows),'GetItemData':lambda i:rows[i],'UpdateProperty':update}
    globals_={'cafe':{'utility':{'ObjectUtil':{'MergeInto':lambda target,source:target.update(source)}}}}
    Machine(globals_,[]).run(program(subskills_suffix),{1:{'mSubskillsDataProvider':provider}})
    assert rows[0]['value']==rating and rows[0]['display']=='Speed' and updates==[(0,'ExtraInfo')]
    extra=rows[0]['ExtraInfo'];assert extra is not original_extra and extra['RightLabelDisplayOverride']==expected(rating)
    assert 'RightLabelDisplayOverride' not in original_extra and style['fontColor']==0xFFFFFF
    for key in ['MinValue','MaxValue','IncreaseValue','CanIncrease']:assert extra[key]==original_extra[key]
    assert extra['UnfocusValueTextStyle']['font']=='native' and extra['UnfocusValueTextStyle']['fontColor']==GRADE_COLORS[expected(rating)]
    updates.clear()
    Machine(globals_,[]).run(program(subskills_suffix),{1:{'mSubskillsDataProvider':provider}})
    assert updates==[], 'Unchanged subskill notified native data provider'
    # A reused row, missing marker, or native focus/style reset must not be
    # suppressed by the no-op guard. Numeric progression remains untouched.
    for field,value in [('RightLabelDisplayOverride','native'),('DEPlayerRating',False)]:
        rows[0]['ExtraInfo'][field]=value;updates.clear()
        Machine(globals_,[]).run(program(subskills_suffix),{1:{'mSubskillsDataProvider':provider}})
        assert updates==[(0,'ExtraInfo')] and rows[0]['ExtraInfo']['RightLabelDisplayOverride']==expected(rating)
    rows[0]['ExtraInfo']['UnfocusValueTextStyle']['fontColor']=0xFFFFFF;updates.clear()
    Machine(globals_,[]).run(program(subskills_suffix),{1:{'mSubskillsDataProvider':provider}})
    assert updates==[(0,'ExtraInfo')] and rows[0]['value']==rating

# Reuse the same subskill row across scale, fog and color transitions. Inspect
# actual display/style fields so no widget-identity cache can hide new values.
row={'value':86,'display':'Speed','ExtraInfo':{}}
notifications=[]
def redraw_update(index,key,value):
    row[key]=value;notifications.append((index,key))
redraw_provider={'GetNumItems':lambda:1,'GetItemData':lambda index:row,'UpdateProperty':redraw_update}
from grade_scales import label
for scale in [0,1,2,3,4,0]:
    for colors in [True,False,True]:
        for fog in [True,False,True]:
            if not colors and not fog:continue # Both off delegates to native.
            p=program(subskills_suffix);code=bytearray(p.code)
            for offsets,value in [(p.scale_offsets,scale),(p.color_offsets,int(colors)),(p.fog_offsets,int(fog))]:
                for at in offsets:code[at]=value
            p=replace(p,code=bytes(code))
            Machine(globals_,[]).run(p,{1:{'mSubskillsDataProvider':redraw_provider}})
            assert row['ExtraInfo']['RightLabelDisplayOverride']==(label(86,scale) if fog else '86')
            assert row['value']==86 and row['display']=='Speed'
            notifications.clear()
            Machine(globals_,[]).run(p,{1:{'mSubskillsDataProvider':redraw_provider}})
            assert notifications==[], 'Unchanged row notified provider after option transition'

sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
from inspect_tb_ast import load
for asset,specs in SPECS.values():
    apt=load(Path(asset))
    for off,argc,regs,flags,size,hash_,newregs,suffix,count in specs:
        original=apt.data[off+48:off+48+size]
        assert fnv(original)==hash_ and newregs>=regs and newregs<=64
        emitted=append_program(size,suffix,count)
        copy_count=size if count is None else count
        assert emitted.code[:copy_count]==bytes(copy_count), 'Native game bytes were emitted'
        assert len(emitted.code)<32768
        if count is not None:assert original[count:]==bytes([0xB9,1,0x3E])
print('Passed: all 101 card grade boundaries/colors, hidden values, marker gating, native model/slider/progression preservation, focus resets, style cloning and private-asset fingerprint gates.')
