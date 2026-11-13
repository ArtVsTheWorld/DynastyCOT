"""Grade only player-name/OVR text in the fingerprinted FullReport renderer."""
from pathlib import Path
import json,struct,sys
from build_recruit_notes_ui import Assembler,fnv
from build_player_card_ui import path,numeric_display_grade,palette,text_label

POSITIONS=['QB','RB','HB','FB','WR','TE','LT','LG','C','RG','RT','LE','RE','DT','LOLB','MLB','ROLB','CB','FS','SS','EDGE','LEDG','REDG','WILL','MIKE','SAM','OLB','K','P']

def suffix(a):
    path(a,1,'mData');a.store(2);a.missing(2,'done')
    path(a,2,'display');a.string('Players to Watch');a.raw(0x66);a.branch('section')
    path(a,2,'display');a.string('PLAYERS TO WATCH');a.raw(0x66);a.branch('done',0xB8)
    a.label('section');path(a,2,'ExtraInfo');a.store(2);a.missing(2,'done')
    path(a,2,'SituationArray');a.store(2);a.missing(2,'done');a.number(0);a.store(3)
    a.label('loop');a.reg(3);a.number(4);a.raw(0x48);a.branch('done',0xB8)
    a.reg(3);path(a,2,'length');a.raw(0x48);a.branch('done',0xB8)
    a.reg(2);a.reg(3);a.raw(0x4E);a.store(4);a.missing(4,'next')
    for position in POSITIONS:path(a,4,'label');a.string(position);a.raw(0x66);a.branch('player')
    a.branch('next',0x99);a.label('player');path(a,4,'value');a.store(5)
    a.reg(5);a.raw(0x44);a.string('string');a.raw(0x66);a.branch('next',0xB8)
    path(a,5,'length');a.number(160);a.raw(0x67);a.branch('next')
    a.string(' ');a.number(1);a.reg(5);a.string('split');a.raw(0x52);a.store(6)
    path(a,6,'length');a.number(2);a.raw(0x48);a.branch('next')
    path(a,6,'length');a.number(16);a.raw(0x67);a.branch('next')
    path(a,6,'length');a.number(1);a.raw(0x0B);a.store(7)
    a.reg(6);a.reg(7);a.raw(0x4E);a.store(8)
    numeric_display_grade(a,8,9,'next','weeklyGrade');palette(a,9,10,'next','weeklyPalette')
    a.string('');a.store(11);a.number(0);a.store(12)
    a.label('nameLoop');a.reg(12);a.reg(7);a.raw(0x48);a.branch('nameDone',0xB8)
    a.reg(11);a.reg(6);a.reg(12);a.raw(0x4E,0x47);a.string(' ');a.raw(0x47);a.store(11)
    a.reg(12);a.raw(0x50);a.store(12);a.branch('nameLoop',0x99)
    a.label('nameDone');a.reg(11);a.reg(9);a.raw(0x47);a.store(11)
    a.reg(1);a.string('mcValue');a.reg(3);a.raw(0x47,0x4E);a.store(13)
    text_label(a,13,11,10,14,'next','weeklyText')
    a.label('next');a.reg(3);a.raw(0x50);a.store(3);a.branch('loop',0x99)
    a.label('done');a.ret()


def matchup_suffix(a):
    # TeamProfile._SetStatsData has already resolved its WidgetData.ExtraInfo
    # to r3 and drawn the ten native label/value pairs. Only the exact
    # '<position>: <name> <OVR> vs <name> <OVR>' presentation is admitted.
    a.number(0);a.store(13)
    a.label('fieldLoop');a.reg(13);a.number(20);a.raw(0x48);a.branch('matchDone',0xB8)
    a.reg(13);a.number(10);a.raw(0x48);a.branch('valueField',0xB8)
    a.string('RecordLabel');a.store(14);a.string('mcStatsLabel');a.store(15);a.reg(13);a.store(16);a.branch('fieldReady',0x99)
    a.label('valueField');a.string('RecordValue');a.store(14);a.string('mcStatsValue');a.store(15);a.reg(13);a.number(10);a.raw(0x0B);a.store(16)
    a.label('fieldReady');a.reg(3);a.reg(14);a.reg(16);a.raw(0x47,0x4E);a.store(5)
    a.reg(5);a.raw(0x44);a.string('string');a.raw(0x66);a.branch('nextField',0xB8)
    path(a,5,'length');a.number(256);a.raw(0x67);a.branch('nextField')
    a.string(' ');a.number(1);a.reg(5);a.string('split');a.raw(0x52);a.store(6)
    a.reg(6);a.number(0);a.raw(0x4E);a.store(7)
    for pos in POSITIONS:a.reg(7);a.string(pos+':');a.raw(0x66);a.branch('matchPosition')
    a.branch('nextField',0x99)
    a.label('matchPosition');a.string(' vs ');a.number(1);a.reg(5);a.string('split');a.raw(0x52);a.store(17)
    path(a,17,'length');a.number(2);a.raw(0x66);a.branch('nextField',0xB8)
    a.empty_object(18);a.number(0);a.store(19)
    a.label('sideLoop');a.reg(17);a.reg(19);a.raw(0x4E);a.store(5)
    a.string(' ');a.number(1);a.reg(5);a.string('split');a.raw(0x52);a.store(6)
    path(a,6,'length');a.number(2);a.raw(0x48);a.branch('nextField')
    path(a,6,'length');a.number(16);a.raw(0x67);a.branch('nextField')
    path(a,6,'length');a.number(1);a.raw(0x0B);a.store(7)
    a.reg(6);a.reg(7);a.raw(0x4E);a.store(8)
    numeric_display_grade(a,8,9,'nextField','matchGrade')
    a.string('');a.store(11);a.number(0);a.store(12)
    a.label('sideNameLoop');a.reg(12);a.reg(7);a.raw(0x48);a.branch('sideNameDone',0xB8)
    a.reg(11);a.reg(6);a.reg(12);a.raw(0x4E,0x47);a.string(' ');a.raw(0x47);a.store(11)
    a.reg(12);a.raw(0x50);a.store(12);a.branch('sideNameLoop',0x99)
    a.label('sideNameDone');a.reg(18);a.reg(19);a.reg(11);a.reg(9);a.raw(0x47,0x4F)
    a.reg(19);a.raw(0x50);a.store(19);a.reg(19);a.number(2);a.raw(0x48);a.branch('sideLoop')
    a.reg(1);a.reg(15);a.reg(16);a.raw(0x47,0x4E);a.store(20);a.missing(20,'nextField')
    a.reg(18);a.number(0);a.raw(0x4E);a.string(' vs ');a.raw(0x47);a.reg(18);a.number(1);a.raw(0x4E,0x47)
    a.number(1);a.reg(20);a.call('SetLabel')
    # One mixed matchup line retains native color; two different grades must
    # not be painted as if the whole line had a single rating.
    a.empty_object(21);a.reg(21);a.string('resize');a.global_object('art_styleguide','Font','RESIZE_SCALEDOWN');a.raw(0x4F)
    a.reg(21);a.number(1);a.reg(20);a.call('SetExtraInfo')
    a.label('nextField');a.reg(13);a.raw(0x50);a.store(13);a.branch('fieldLoop',0x99)
    a.label('matchDone');a.ret()


def picker_program(a):
    # Presentation-only equivalent of FocusTraining._SetupPopupData: retain
    # one native adapter call, row order/keys/identity, every existing field,
    # and the original AddItem call. Only WidgetData.ExtraInfo.Label2 changes.
    a.number(0);path(a,1,'mTrainingAdapter');a.string('GetFocusTrainingPlayerList');a.raw(0x52);a.store(2)
    a.number(0);a.store(3)
    a.label('pickerLoop');a.reg(3);path(a,2,'length');a.raw(0x48);a.branch('pickerDone',0xB8)
    a.global_object('fb_shared_enum','BindingType','OPTIONS');a.number(1);path(a,1,'mView');a.string('GetTemplate');a.raw(0x52);a.store(4)
    a.reg(2);a.reg(3);a.raw(0x4E);a.store(5)
    a.reg(4);a.string('display');path(a,5,'playerFirstName');a.string(' ');a.raw(0x47);path(a,5,'playerLastName');a.raw(0x47,0x4F)
    for name in ['value','key']:
        a.reg(4);a.string(name);path(a,5,name);a.raw(0x4F)
    path(a,4,'ExtraInfo');a.store(6);a.missing(6,'pickerCreateExtra');a.branch('pickerExtraReady',0x99)
    a.label('pickerCreateExtra');a.empty_object(6);a.reg(4);a.string('ExtraInfo');a.reg(6);a.raw(0x4F)
    a.label('pickerExtraReady')
    for dest,source in [('number','number'),('pos','pos'),('height','height'),('weight','weight'),('college','college'),('xp','xp'),('Label2','overall'),('Label3','devTrait'),('Label4','age'),('yearsPro','yearsPro')]:
        a.reg(6);a.string(dest);path(a,5,source);a.raw(0x4F)
    path(a,5,'overall');a.store(7);numeric_display_grade(a,7,8,'pickerAdd','pickerGrade')
    a.reg(6);a.string('Label2');a.reg(8);a.raw(0x4F)
    a.label('pickerAdd');a.reg(4);a.number(1);path(a,1,'mOptionsProvider');a.call('AddItem')
    a.reg(3);a.raw(0x50);a.store(3);a.branch('pickerLoop',0x99)
    a.label('pickerDone');a.ret()

def generate(asset,destination):
    sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
    from inspect_tb_ast import load
    lines=['// Generated scoped Weekly Strategy presentation; verified source records remain native.',
           '#pragma once','#include "RecruitNotesUiPrograms.h"','namespace recruitingfog::weeklyreportui {',
           ]
    specs=[(asset,0x1490,156,0,5,25,15,suffix,156),
           (asset.parent/'teamprofile.ast',0x2770,588,1,13,41,22,matchup_suffix,588),
           (asset.parent/'corenode.ast',0x6e58,324,0,5,41,9,picker_program,0)]
    evidence=[]
    for method,(source,off,size,argc,regs,flags,newregs,emit,copy) in zip(['programs','teamProfilePrograms','pickerPrograms'],specs):
        lines.append(f'inline std::vector<notesui::Program> {method}(){{std::vector<notesui::Program> result;')
        apt=load(source);header=apt.data[off:off+48]
        assert (struct.unpack_from('<I',header,8)[0],struct.unpack_from('<H',header,12)[0],struct.unpack_from('<H',header,14)[0])==(argc,regs,flags)
        original=apt.data[off+48:off+48+size];assert len(original)==size
        assert fnv(original)=={5264:9462526817822673144,10096:13331554265108800653,28248:3931116635804362792}[off]
        a=Assembler();a.code.extend(bytes(copy));emit(a);p=a.finish();assert len(p.code)<=32768
        lines.append(f'result.push_back({{{off},{argc},{regs},{flags},{size},{fnv(original)}ull,{newregs},0,{copy},{{'+','.join(hex(b) for b in p.code)+'}});')
        for name,values in [('scaleOffsets',p.scale_offsets),('fogOffsets',p.fog_offsets),('colorOffsets',p.color_offsets),('teamScaleOffsets',p.team_scale_offsets)]:
            for at in values:lines.append(f'result.back().{name}.push_back({at});')
        for at,value in p.literals:lines.append('{const auto pointer=reinterpret_cast<uintptr_t>('+json.dumps(value)+f');memcpy(result.back().code.data()+{at},&pointer,8);}}')
        evidence.append(dict(asset=source.name,headerOffset=off,bodySize=size,bodyHash=fnv(original),newBodySize=len(p.code),nativeCopyBytes=copy))
        lines.append('return result;}')
    lines+=['}'];destination.write_text('\n'.join(lines)+'\n',encoding='utf8')
    print(json.dumps(evidence))

if __name__=='__main__':generate(Path(sys.argv[1]),Path(sys.argv[2]))
