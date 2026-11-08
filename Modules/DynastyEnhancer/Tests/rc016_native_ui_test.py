"""Exact AIO bank ownership, direct/deferred monetary writers and native bounds."""
from pathlib import Path
from dataclasses import replace
import sys,json,re,struct,copy,math
ROOT=Path(__file__).resolve().parents[1]
sys.path[:0]=[str(ROOT/'Tools'),str(ROOT/'Tests'),'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools']
from inspect_tb_ast import load
from build_recruit_notes_ui import Assembler,fnv
from apt_source_pointers import source_pointers
from build_currency_ui import icon_label_suffix
from build_currency_topbar_ui import total_hours_offer_suffix
from recruit_notes_vm import Machine
import currency_recruiteditor_ui_test as editor
import currency_recruiting_ui_test as wallets

header=(ROOT/'Source/AioCurrencyUiPrograms.h').read_text(encoding='utf8')
rows=json.loads((ROOT/'Evidence/Aio141/method-map.json').read_text(encoding='utf8'))
for row in rows:
    if row['group'] in ['detail','topbar']:continue
    apt=load(ROOT/row['asset']);offset=row['new'];body=apt.data[offset+48:offset+48+row['size']]
    marker=f'result.push_back({{{offset},{row["argc"]},{row["registers"]},{row["flags"]},{row["size"]},{fnv(body)}ull,'
    assert marker in header,(row['group'],row['name'],'Missing known unchanged or relocated sibling')
assert any(r['group']=='recruiting' and r['old']==r['new']==0x37668 for r in rows)
assert any(r['group']=='editor' and r['name']=='HandleInput' for r in rows)

notes=(ROOT/'Source/AioRecruitNotesUiPrograms.h').read_text(encoding='utf8')
apt=load(ROOT/'Evidence/Aio141/chunks/4053c214-cac1-ede2-1167-4a724f97c001.raw')
specs=re.findall(r'result.push_back\(\{(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)ull',notes)
assert len(specs)==13
for values in specs:
    off,argc,regs,flags,size,hash_=map(int,values);h=apt.data[off:off+48]
    assert struct.unpack_from('<IHH',h,8)==(argc,regs,flags) and struct.unpack_from('<I',h,24)[0]==size
    assert fnv(apt.data[off+48:off+48+size])==hash_
    for at,value in source_pointers(apt,off,size):assert f'sourcePointers.push_back({{{at},{value}}})' in notes
assert '448488,0,2,42,121' in notes,'AIO Navigation must use the relocated callback'

# Current NIL is a numeric editing model. HandleInput completes after the
# native edit/event cascade, independently of the arithmetic writer suffix.
receiver,calls,events,_=editor.fixture(40)
editor.invoke(receiver);assert receiver['mcValue']['mData']['display']=='$160K'
for value in [10,20,250,1000]:
    receiver['mValue']=value
    editor.invoke(receiver,'_AddCurrentValue',0)
    assert receiver['mcValue']['mData']['display']=={10:'$40K',20:'$80K',250:'$1.0M',1000:'$4.0M'}[value]
    assert receiver['mValue']==value and events[-1]['value']==value
    # A delayed native numeric paint must be repaired on the same instance.
    receiver['mcValue']['mData']['display']=str(value);editor.EditorMachine(editor.GLOBALS,editor.pool).run(editor.program('HandleInput',native=False),{1:receiver})
    assert receiver['mcValue']['mData']['display'].startswith('$')
editor.EditorMachine(editor.GLOBALS,editor.pool).run(editor.program('HandleInput',enabled=False,native=False),{1:receiver})
assert receiver['mcValue']['mData']['display']=='1000' and receiver['mValue']==1000

# The offer beside recruiting hours is the unchanged sibling formerly omitted
# from AIO's bank. Exercise first draw, edit, deferred native reset and row reuse.
model={'mData':{'ExtraInfo':{'OfferPoints':10}},'mcOfferPoints':wallets.text(10),'mcHours':wallets.text('0/50'),
       'mcOfferIcon':wallets.background(18),'mcOfferBackground':wallets.background(40),'mcHoursBackground':wallets.background(100,-80),'PADDING_OFFER':8,'PADDING_TEXT':6}
for value,expected in [(10,'$40K'),(25,'$100K'),(250,'$1.0M'),(0,'$0')]:
    model['mData']['ExtraInfo']['OfferPoints']=value
    for _ in range(2):
        model['mcOfferPoints']['mData']['display']=str(value)
        before=copy.deepcopy(model['mData']);Machine(wallets.GLOBALS,[]).run(wallets.program(total_hours_offer_suffix,True),{1:model})
        assert model['mcOfferPoints']['mData']['display']==expected and model['mData']==before
        assert model['mcHours']['mData']['display']=='0/50' and not model['mcHours']['calls']

# A coach-offer wallet retains its own native font/color/alignment and width.
# Clone only the three fitting fields. Inputs and adjacent numeric data stay raw.
def program(on=True):
    a=Assembler();icon_label_suffix(a);p=a.finish();code=bytearray(p.code)
    for at in p.dollar_offsets:code[at]=int(on)
    return replace(p,code=bytes(code))
g={'Math':{'round':lambda n:math.floor(n+.5),'floor':math.floor},'art_styleguide':{'Font':{'RESIZE_SCALEDOWN':4}},'shared_enum':{'fblibraries':{'global':{'Icons':{'LIBRARY_ID':7,'ASSETID_ICON_CURRENCY_PROGRAMPOINTS':19}}}},'art_utility':{'ImageUtil':{'BuildFbImageString':lambda icon,lib:f'fbimage:{icon}:{lib}'}}}
for width in [80,120,180]:
    native_style={'fontSize':24,'fontColor':0x123456,'hAlign':'right'}
    native_style['CloneAndModify']=lambda change,base=native_style:{**base,**change}
    child=wallets.text(20);source={'Width':width,'value':'fbimage:19:7','display':20,'ExtraInfo':{'IconSize':24,'TextPadding':8,'LabelTextStyle':native_style}}
    receiver={'mData':source,'mcLabel':child};Machine(g,[]).run(program(),{1:receiver})
    assert child['mData']['display']=='$80K' and source['display']==20 and source['Width']==width
    fitted=child['mData']['ExtraInfo'];assert fitted['resize']==4 and fitted['resizeWidth']==width-32 and fitted['maxNumLines']==1
    assert fitted['fontColor']==0x123456 and fitted['hAlign']=='right' and native_style.get('resize') is None
    # Native redraw reestablishes its ordinary baseline before Off suffix.
    child['mData']['display']='20';child['mData']['ExtraInfo']=dict(native_style);count=len(child['extraCalls'])
    Machine(g,[]).run(program(False),{1:receiver});assert child['mData']['display']=='20' and len(child['extraCalls'])==count
print('RC016 native UI:13 exact AIO Notes callbacks; all19 currency siblings; numeric/deferred NIL and wallet redraw/reuse; native bounded coach fonts; Off gates PASS. Live game acceptance remains pending.')
