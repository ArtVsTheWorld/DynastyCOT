"""Team-only circular rating labels. Preserve native values, fills and animation."""
from pathlib import Path
import sys,struct,json
from build_recruit_notes_ui import Assembler,fnv
from build_player_card_ui import path,numeric_grade,palette,text_label
from apt_source_pointers import source_pointers
from apt_ui_symbols import symbols

ROOT=Path(__file__).resolve().parents[1]

def caption_guard(a,register,done,prefix):
    path(a,register,'ExtraInfo');a.raw(0x76,0x49);a.branch(done)
    path(a,register,'ExtraInfo','Label');a.store(0)
    for label in ('OVR','OFF','DEF'):
        a.reg(0);a.string(label);a.raw(0x66);a.branch(prefix+'valid')
    a.branch(done,0x99);a.label(prefix+'valid')

def mark_team_item(a):
    # Exact TeamRatings._SetupListItem maps its data argument to native r5.
    # Its numeric value and all other data stay untouched. The marker is
    # display metadata and is ignored by the unmodified native controls.
    a.missing(5,'markDone');caption_guard(a,5,'markDone','markCaption')
    path(a,5,'ExtraInfo');a.string('DETeamRating');a.raw(0x73,0x4F)
    a.label('markDone')

def circle_text(a):
    done='circleDone'
    path(a,1,'mData');a.store(5);a.missing(5,done)
    caption_guard(a,5,done,'circleCaption')
    path(a,5,'ExtraInfo','DETeamRating');a.raw(0x73,0x66);a.branch(done,0xB8)
    path(a,5,'value');a.store(0);numeric_grade(a,0,2,done,'circleGrade',team=True)
    # Off retains a supported native Display override when one exists.
    a.fog_enabled(team=True);a.number(1);a.raw(0x66);a.branch('circlePalette')
    path(a,5,'ExtraInfo','Display');a.store(0);a.missing(0,'circlePalette')
    a.reg(0);a.string('');a.raw(0x66);a.branch('circlePalette')
    a.reg(0);a.store(2)
    a.label('circlePalette');palette(a,2,3,done,'circlePaletteValue',team=True)
    path(a,1,'mcValue');a.store(4);a.missing(4,done)
    text_label(a,4,2,3,0,done,'circleLabel',team=True)
    a.label(done);a.ret()

def reviewed():
    sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
    from inspect_tb_ast import load
    specs=[(ROOT/'Evidence/PlayerFog/node-fe-scriptpodart.ast','TeamRatings','_SetupListItem',mark_team_item),
           (Path('C:/Apps/CFB 27/MMC/PlayerAnalytics/Research/assets/common/ui/node_com/scriptpod.ast'),'CircularProgressBar','Draw',circle_text),
           (Path('C:/Apps/CFB 27/MMC/PlayerAnalytics/Research/assets/common/ui/node_com/scriptpod.ast'),'CircularProgressBar','_ValueAnimationInterval',circle_text)]
    out=[]
    for file,cls,method,builder in specs:
        apt=load(file);matches=[off for off,(pool,_,name) in symbols(apt).items() if cls in pool[:8] and name==method]
        if len(matches)!=1:raise ValueError(f'Expected one {cls}.{method}')
        out.append((apt,matches[0],method,builder))
    return out

def generate():
    lines=['// Generated display-only team-circle callbacks. No native code shipped.','#pragma once','#include "RecruitNotesUiPrograms.h"','namespace recruitingfog::teamcircleui {','inline std::vector<notesui::Program> programs(){std::vector<notesui::Program> result;']
    for apt,off,name,builder in reviewed():
        argc,regs,flags=struct.unpack_from('<IHH',apt.data,off+8);size=struct.unpack_from('<I',apt.data,off+24)[0];body=apt.data[off+48:off+48+size]
        # No pointer normalization or interpretation of native instructions:
        # these reviewed bodies have no pointer operands or nested functions.
        nested=[];assert not source_pointers(apt,off,size,nested) and not nested
        a=Assembler();copy_at=0
        if builder==mark_team_item:
            builder(a);a.pad_code();copy_at=len(a.code);a.code.extend(bytes(size));a.ret()
        else:a.code.extend(bytes(size));builder(a)
        assert a.max_register<=regs
        p=a.finish();code='{'+','.join(hex(b) for b in p.code)+'}'
        lines.append(f'result.push_back({{{off},{argc},{regs},{flags},{size},{fnv(body)}ull,{regs},{copy_at},{size},{code}}});')
        for attr,member in [('scale_offsets','scaleOffsets'),('team_scale_offsets','teamScaleOffsets'),('color_offsets','colorOffsets'),('fog_offsets','fogOffsets'),('team_offsets','teamOffsets')]:
            for at in getattr(p,attr):lines.append(f'result.back().{member}.push_back({at});')
        for at,text in p.literals:lines.append('{const auto pointer=reinterpret_cast<uintptr_t>('+json.dumps(text)+f');memcpy(result.back().code.data()+{at},&pointer,8);}}')
    lines+=['return result;}','}'];(ROOT/'Source/TeamCircleUiPrograms.h').write_text('\n'.join(lines)+'\n',encoding='utf8');print('Generated three frame-preserving team-circle callbacks.')

if __name__=='__main__':generate()
