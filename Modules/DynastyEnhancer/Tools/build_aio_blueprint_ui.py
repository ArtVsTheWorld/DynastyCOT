"""Exact AIO Highest NIL Players row adapter; ratings display only."""
from pathlib import Path
import hashlib,json,struct,sys
from build_player_blueprint_ui import generate,blueprint_suffix
from build_recruit_notes_ui import Assembler,fnv
from apt_source_pointers import source_pointers
sys.path.insert(0,r'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools')
from inspect_tb_ast import load
ROOT=Path(__file__).resolve().parents[1]
def main():
 destination=ROOT/'Source/PlayerBlueprintUiPrograms.h';generate(destination)
 source=ROOT/'Evidence/Aio141/chunks/4495638f-5faf-b296-1167-4a724f97c001.raw'
 if hashlib.sha256(source.read_bytes()).hexdigest()!='bab074723b55e2a6441d9e33f646f92a1b5840316c9a6adaf29cf788958bfc8a':raise ValueError('Unsupported AIO 1.4.1 source asset')
 a=load(source);offset,size=570688,2058;h=a.data[offset:offset+48]
 if struct.unpack_from('<IHH',h,8)!=(0,5,25) or struct.unpack_from('<I',h,24)[0]!=size:raise ValueError('Unsupported AIO NIL player frame')
 body=a.data[offset+48:offset+48+size];expected=fnv(body)
 assembler=Assembler();assembler.code.extend(bytes(size));blueprint_suffix(assembler);p=assembler.finish()
 lines=['namespace recruitingfog::playerblueprintui {','inline std::vector<notesui::Program> aioPrograms(){std::vector<notesui::Program> result;']
 lines.append(f'result.push_back({{{offset},0,5,25,{size},{expected}ull,5,0,{size},{{'+','.join(hex(b) for b in p.code)+'}});')
 for at,value in source_pointers(a,offset,size):lines.append(f'result.back().sourcePointers.push_back({{{at},{value}}});')
 for attr,member in [('scale_offsets','scaleOffsets'),('team_scale_offsets','teamScaleOffsets'),('color_offsets','colorOffsets'),('fog_offsets','fogOffsets'),('team_offsets','teamOffsets'),('dollar_offsets','dollarOffsets')]:
  for at in getattr(p,attr):lines.append(f'result.back().{member}.push_back({at});')
 for at,value in p.literals:lines.append('{const auto pointer=reinterpret_cast<uintptr_t>('+json.dumps(value)+f');memcpy(result.back().code.data()+{at},&pointer,8);}}')
 lines+=['return result;}','}'];destination.write_text(destination.read_text()+'\n'+'\n'.join(lines)+'\n',encoding='utf-8')
 print('Generated AIO Highest NIL Players adapter; source SHA256 '+hashlib.sha256(source.read_bytes()).hexdigest())
if __name__=='__main__':main()
