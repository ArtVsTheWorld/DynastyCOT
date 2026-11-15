"""Generate symbolic contracts for every existing BTF UI bank, offline only."""
from pathlib import Path
import sys,json,struct
R=Path(__file__).resolve().parents[1];sys.path[:0]=[str(R/'Modules/DynastyEnhancer/Tools'),'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools']
from inspect_tb_ast import load
from build_ui_method_contracts import cpp

def main():
 doc=json.loads((R/'Build/UiDiscovery/UniversalAdapters/source-audit.json').read_text());lines=['// Generated class/method contracts. No asset offsets or native bodies.','#pragma once','namespace recruitingfog::uidiscovery {','inline std::vector<Contract> configureAdapter(const std::string& bank,notesui::Runtime& runtime,bool dependency=false){','auto programs=runtime.discoveryPrograms();std::vector<Contract> contracts;']
 for b in doc['banks']:
  if b['bank']=='notes' or not b['methods']:continue
  candidates=[next((c for c in row['candidates'] if 'contract'in c),None) for row in b['methods']]
  if any(c is None or row['distinctContracts']!=1 for row,c in zip(b['methods'],candidates)):raise ValueError('Unresolved '+b['bank'])
  lines.append('if(bank=='+json.dumps(b['bank'])+'){contracts={'+','.join(cpp(c['contract']) for c in candidates)+'};')
  for row,c in zip(b['methods'],candidates):
   m=row['method'];i=row['index'];apt=load(c['asset'])
   def text(at):return apt.data[at:apt.data.index(0,at)].decode('latin1')
   for q in m['pointers']:
    at=q['offset'];value=q['value'];assert apt.data[m['offset']+48+at-8:m['offset']+48+at].rstrip(b'\0')[-1:]==b'\xa1'
    lines.append('for(auto& q:programs.at('+str(i)+').sourcePointers)if(q.offset=='+str(at)+'){q.byText=true;q.text='+json.dumps(text(value))+';}')
   for n in m['nested']:
    off=m['offset']+48+n['offset'];argc=int.from_bytes(apt.data[off+8:off+12],'little');args=[]
    for k in range(argc):
     at=n['params']+16*k;reg=struct.unpack_from('<I',apt.data,at)[0];ptr=struct.unpack_from('<Q',apt.data,at+8)[0];args.append('{'+str(reg)+','+json.dumps(text(ptr))+'}')
    lines.append('for(auto& q:programs.at('+str(i)+').nestedDefinitions)if(q.offset=='+str(n['offset'])+'){q.semantic=true;q.nameText='+json.dumps(text(n['name']))+';q.arguments={'+','.join(args)+'};}')
  lines.append('}')
 lines+=['if(contracts.size()!=programs.size())throw std::runtime_error("Missing universal UI contracts for "+bank);', 'using Validator=std::function<bool(uintptr_t,const Header&,const notesui::GuardedRead&)>;std::vector<Validator> validators;', 'for(const auto& contract:contracts)validators.emplace_back(dependency?Validator{}:Validator([contract](uintptr_t definition,const Header& header,const notesui::GuardedRead& read){return declarationMatches(definition,contract,read)&&prefixMatches(contract,header,read,reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)))&&poolMatches(contract,header,read,reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)));}));','runtime.configureDiscovery(std::move(programs),std::move(validators));return contracts;','}','}']
 target=R/'Modules/DynastyEnhancer/Source/UiAdapterContracts.generated.h';target.write_text('\n'.join(lines)+'\n');print('Generated contracts for all non-Notes banks; Notes remains an atomic dependency group.')
if __name__=='__main__':main()
