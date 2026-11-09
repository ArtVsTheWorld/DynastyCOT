"""Legacy shared-slider formatter regression. The live NIL toggle path is tested separately."""
import copy,math,sys
from dataclasses import replace
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_recruit_notes_ui import Assembler
from build_player_card_ui import generic_slider_draw_suffix
from build_player_nil_ui import nil_draw_suffix
from recruit_notes_vm import Machine,UNDEFINED

GLOBALS={'Math':{'round':lambda v:math.floor(v+.5),'floor':math.floor},'art_styleguide':{'Font':{'RESIZE_SCALEDOWN':4}}}
class Bounded(Assembler):
    def reg(self,n):assert 0<=n<10;super().reg(n)
    def store(self,n):assert 0<=n<10;super().store(n)
def program(builder,on=True):
    a=Bounded();builder(a);a.ret();p=a.finish();code=bytearray(p.code)
    for at in p.dollar_offsets:code[at]=int(on)
    # Prove Current NIL does not depend on any rating/fog feature being on.
    for at in p.fog_offsets+p.color_offsets:code[at]=0
    return replace(p,code=bytes(code))
def widget():
    calls=[];native={'fontColor':0x123456,'font':'native','resize':2,'resizeWidth':200,'maxNumLines':3}
    text={'mData':{'display':'0','ExtraInfo':dict(native)}}
    def label(value):text['mData']['display']=value;calls.append(('label',value))
    def style(value):text['mData']['ExtraInfo'].update(value);calls.append(('style',dict(value)))
    text.update(SetLabel=label,SetExtraInfo=style)
    slider={'mData':{'value':0,'ExtraInfo':{'ValueWidth':88,'MinValue':0,'MaxValue':1000,'IncreaseValue':5}},'mcRightValue':text}
    return slider,text,calls,native

slider,text,calls,native=widget();source={'Ovr':64,'CurrentNIL':0,'NILValue':0,'RiskOfTransfer':80}
row={'mcCurrentNIL':slider,'mData':{'ExtraInfo':source}}
Machine(GLOBALS,[]).run(program(nil_draw_suffix),{1:row})
assert slider['mDECotCurrentNIL'] is True and source=={'Ovr':64,'CurrentNIL':0,'NILValue':0,'RiskOfTransfer':80}
for value,expected in [(0,'$0'),(5,'$20K'),(10,'$40K'),(250,'$1.0M'),(1000,'$4.0M'),(0,'$0')]:
    slider['mData']['value']=value
    # Native adjustment/deferred/reused-row paints can replace the text after
    # its parent's Draw. The slider suffix must repair the final native paint.
    for _ in range(2):
        text['mData']['display']=str(value);before=copy.deepcopy(slider['mData'])
        Machine(GLOBALS,[]).run(program(generic_slider_draw_suffix),{1:slider})
        assert text['mData']['display']==expected and slider['mData']==before
        assert text['mData']['ExtraInfo']['resizeWidth']==88 and text['mData']['ExtraInfo']['maxNumLines']==1
    calls.clear();Machine(GLOBALS,[]).run(program(generic_slider_draw_suffix),{1:slider})
    assert not calls, 'Unchanged NIL redraw caused another setter/style allocation'
    text['mData']['ExtraInfo']['fontColor']=0
    Machine(GLOBALS,[]).run(program(generic_slider_draw_suffix),{1:slider})
    assert text['mData']['ExtraInfo']['fontColor']==0, 'Selected native foreground was overwritten'

Machine(GLOBALS,[]).run(program(generic_slider_draw_suffix,False),{1:slider})
assert text['mData']['display']=='0'
assert {k:v for k,v in text['mData']['ExtraInfo'].items() if k!='fontColor'}=={k:v for k,v in native.items() if k!='fontColor'}
assert slider['mDECotNilMoney'] is UNDEFINED
other,text,calls,_=widget();other['mData']['value']=5;before=copy.deepcopy(other['mData'])
Machine(GLOBALS,[]).run(program(generic_slider_draw_suffix),{1:other})
assert not calls and other['mData']==before, 'An unrelated slider was treated as money'
print('PASS: legacy shared-slider formatter, 0/5/10/K/M edits and deferred paints, row reuse, all fog/colors Off, zero unchanged setters, native selection color, numeric budgets/ranges and ten-register frame preserved; scoped Off restoration.')
