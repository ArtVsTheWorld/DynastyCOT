"""Offline VM verification of the team-only circular label suffix/prefix."""
from dataclasses import replace
from functools import lru_cache
from pathlib import Path
import sys, time
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'Tools'))
from build_recruit_notes_ui import Assembler
from build_team_circle_ui import circle_text, mark_team_item
from recruit_notes_vm import Machine
from grade_scales import label, COLORS

GLOBALS={'art_styleguide':{'Font':{'RESIZE_SCALEDOWN':9},'Color':{'UNFOCUS_BUTTON_FONT_COLOR':0x123456}}}

@lru_cache(None)
def program(builder, scale=0, fog=True, colors=True):
    a=Assembler();builder(a);p=a.finish();code=bytearray(p.code)
    for offsets,value in [(p.scale_offsets,4), (p.team_scale_offsets,scale),
                          (p.fog_offsets,0), (p.team_offsets,int(fog)), (p.color_offsets,int(colors))]:
        for at in offsets:code[at]=value
    return replace(p,code=bytes(code))

def widget(n, caption='OVR', marked=True):
    text={'mData':{'display':str(n),'ExtraInfo':{'fontColor':0x123456,'font':'native'}},'_width':80}
    text['SetLabel']=lambda v:text['mData'].update(display=v)
    text['SetExtraInfo']=lambda v:text['mData']['ExtraInfo'].update(v)
    return {'mData':{'value':n,'ExtraInfo':{'Label':caption,'DETeamRating':marked}},'mcValue':text}

checks=0
for caption in ['OVR','OFF','DEF']:
    for scale in range(5):
        for fog in [False,True]:
            for colors in [False,True]:
                for n in range(101):
                    w=widget(n,caption)
                    Machine(GLOBALS,[]).run(program(circle_text,scale,fog,colors),{1:w})
                    assert w['mcValue']['mData']['display']==(label(n,scale) if fog else str(n))
                    expected=0x123456 if scale==4 or not colors else COLORS[label(n,scale)]
                    assert w['mcValue']['mData']['ExtraInfo']['fontColor']==expected
                    assert w['mData']['value']==n
                    checks+=1

# A team caption alone cannot turn a player, rank, statistic or currency into
# a team rating. Missing metadata and native display overrides stay safe.
for caption,mark in [('OVR',False),('RANK',True),('YARDS',True),('$',True)]:
    w=widget(75,caption,mark)
    Machine(GLOBALS,[]).run(program(circle_text,1),{1:w})
    assert w['mcValue']['mData']['display']=='75'
for data in [{}, {'value':75}, {'value':75,'ExtraInfo':{}}, {'value':75,'ExtraInfo':{'Label':'OVR'}}]:
    w=widget(75);w['mData']=data
    Machine(GLOBALS,[]).run(program(circle_text,1),{1:w})
    assert w['mcValue']['mData']['display']=='75'
for colors in [False,True]:
    w=widget(75);w['mData']['ExtraInfo']['Display']='native display';w['mcValue']['mData']['display']='native display'
    Machine(GLOBALS,[]).run(program(circle_text,1,False,colors),{1:w})
    assert w['mcValue']['mData']['display']=='native display'

# The native TeamRatings argument is explicitly marked without changing value
# or unrelated metadata. It does not classify non-rating children.
for caption in ['OVR','OFF','DEF','RANK','COST']:
    d={'value':71,'ExtraInfo':{'Label':caption,'Display':'native'}}
    Machine(GLOBALS,[]).run(program(mark_team_item),{5:d})
    assert d['value']==71 and d['ExtraInfo']['Display']=='native'
    assert d['ExtraInfo'].get('DETeamRating',False)==(caption in ['OVR','OFF','DEF'])

# Both native Draw and the timer can repaint the numeric label. The suffix
# must reapply policy while leaving the animation's numeric source intact.
w=widget(95)
for scale in [1,4,3,0,4,2]:
    for _ in range(3):
        w['mcValue']['mData']['display']='95'
        Machine(GLOBALS,[]).run(program(circle_text,scale),{1:w})
        assert w['mcValue']['mData']['display']==label(95,scale)
        assert w['mData']['value']==95
begin=time.perf_counter()
for _ in range(2000):Machine(GLOBALS,[]).run(program(circle_text,1),{1:w})
print(f'PASS: {checks} team-circle policies, strict semantic markers, redraw/animation, native Off and values; 2000 VM suffixes {(time.perf_counter()-begin)*1000:.1f} ms.')
