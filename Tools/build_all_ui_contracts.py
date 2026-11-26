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
  extra=[]
  if b['bank']=='card.0':
   from build_ui_method_contracts import contract
   from build_player_card_ui import SPECS
   asset,specs=SPECS['controlPrograms'];apt=load(Path(asset))
   extra=[contract(apt,spec[0]) for spec in specs[len(candidates):]]
  lines.append('if(bank=='+json.dumps(b['bank'])+'){contracts={'+','.join([*(cpp(c['contract']) for c in candidates),*(cpp(c) for c in extra)])+'};')
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
 # Stats owns its typed active-player skills after native capture. Its own
 # isolated bank preserves every unrelated in-game adapter on incompatibility.
 from build_ui_method_contracts import contract
 from build_player_ingame_ui import STATS_SPECS
 stats=load(R/'Modules/DynastyEnhancer/Evidence/PlayerFog/Stats/playergamestats.ast')
 lines.append('if(bank=="card.66"){contracts={'+','.join(cpp(contract(stats,s[0])) for s in STATS_SPECS)+'};}')
 if not any(b['bank']=='recruiting.8' and b['methods'] for b in doc['banks']):
  # This small header bank was added after the original private audit.
  from build_ui_method_contracts import contract
  from build_recruit_interest_ui import FULL_OFFER_HEADER_BODY
  vanilla=load(R/'Build/Followup-20261112/DynastyEnhancer/Evidence/RecruitingFog/Assets/common__ui__node_cfm__recruitinghub__corenode.ast')
  lines.append('if(bank=="recruiting.8"){contracts={'+cpp(contract(vanilla,FULL_OFFER_HEADER_BODY-48))+'};}')
 lines+=['if(contracts.size()!=programs.size())throw std::runtime_error("Missing universal UI contracts for "+bank);', 'using Validator=std::function<bool(uintptr_t,const Header&,const notesui::GuardedRead&)>;std::vector<Validator> validators;', 'for(const auto& contract:contracts)validators.emplace_back(dependency?Validator{}:Validator([contract](uintptr_t definition,const Header& header,const notesui::GuardedRead& read){return declarationMatches(definition,contract,read)&&prefixMatches(contract,header,read,reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)))&&poolMatches(contract,header,read,reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)));}));','runtime.configureDiscovery(std::move(programs),std::move(validators));return contracts;','}','}']
 target=R/'Modules/DynastyEnhancer/Source/UiAdapterContracts.generated.h';target.write_text('\n'.join(lines)+'\n');print('Generated contracts for all non-Notes banks; Notes remains an atomic dependency group.')
if __name__=='__main__':main()
