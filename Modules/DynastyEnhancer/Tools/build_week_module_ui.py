"""Team-only display suffix for the native WeekModuleHorizontal banner."""
from pathlib import Path
import sys,json,struct
from build_recruit_notes_ui import NativeFrameAssembler,fnv
from build_fog_gap_ui import team_badges
from apt_source_pointers import source_pointers
from apt_ui_symbols import symbols
R=Path(__file__).resolve().parents[1]
def reviewed():
 sys.path.insert(0,'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
 from inspect_tb_ast import load
 apt=load(R/'Evidence/PlayerFog/Gaps/current-weekmodulehorizontal.ast');matches=[off for off,(pool,_,name) in symbols(apt).items() if 'WeekModuleHorizontal'in pool[:8] and name=='Draw'];assert len(matches)==1;off=matches[0]
 argc,regs,flags=struct.unpack_from('<IHH',apt.data,off+8);size=struct.unpack_from('<I',apt.data,off+24)[0];body=apt.data[off+48:off+48+size];assert (argc,regs,flags,size,fnv(body))==(0,5,25,1898,1427184964622245652)
 nested=[];assert not source_pointers(apt,off,size,nested) and not nested
 return apt,off,argc,regs,flags,size,body

def suffix(a):team_badges(a,('mcAwayTeamOvr','mcAwayTeamOff','mcAwayTeamDef','mcHomeTeamOvr','mcHomeTeamOff','mcHomeTeamDef'),'weekModuleTeam');a.ret()
def program():
 a=NativeFrameAssembler(5);a.code.extend(bytes(1898));suffix(a);p=a.finish();assert a.max_register<5;return p

def generate():
 apt,off,argc,regs,flags,size,body=reviewed();p=program();lines=['// Generated display-only WeekModuleHorizontal team badges. Numeric data is untouched.','#pragma once','#include "RecruitNotesUiPrograms.h"','namespace recruitingfog::weekmoduleui {','inline std::vector<notesui::Program> programs(){std::vector<notesui::Program> result;',f'result.push_back({{{off},{argc},{regs},{flags},{size},{fnv(body)}ull,{regs},0,{size},{{'+','.join(hex(b) for b in p.code)+'}});']
 for src,dst in [('scale_offsets','scaleOffsets'),('team_scale_offsets','teamScaleOffsets'),('color_offsets','colorOffsets'),('fog_offsets','fogOffsets'),('team_offsets','teamOffsets'),('dollar_offsets','dollarOffsets')]:
  for at in getattr(p,src):lines.append(f'result.back().{dst}.push_back({at});')
 for at,text in p.literals:lines.append('{const auto pointer=reinterpret_cast<uintptr_t>('+json.dumps(text)+f');memcpy(result.back().code.data()+{at},&pointer,8);}}')
 lines+=['return result;}','}'];(R/'Source/WeekModuleUiPrograms.h').write_text('\n'.join(lines)+'\n');print('Generated WeekModuleHorizontal team-only display adapter.')
if __name__=='__main__':generate()
