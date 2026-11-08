from pathlib import Path
import json,hashlib
root=Path(r'C:\Apps\CFB 27\MMC\DynastyCOT')
changes=[]
for name in ['NativeSocialMedia.cpp','RecruitingFog.cpp']:
 path=root/'Modules/DynastyEnhancer/Source'/name;s=path.read_text()
 s='#include "../../../Compatibility/PracticeTextSetter.generated.h"\n'+s
 if name=='NativeSocialMedia.cpp':
  old='for(const auto& g:socialGates){unsigned char bytes[32]{};if(!read(base+g.rva,bytes,32)||memcmp(bytes,g.bytes,32))throw std::runtime_error("Unsupported game code at "+std::to_string(g.rva));}'
  new='for(const auto& g:socialGates){unsigned char bytes[32]{};if(!read(base+g.rva,bytes,32))throw std::runtime_error("Unreadable game code");if(memcmp(bytes,g.bytes,32))cotcompat::practice::normalize(base,g.rva,bytes,g.bytes,32,read);if(memcmp(bytes,g.bytes,32))throw std::runtime_error("Unsupported game code at "+std::to_string(g.rva));}'
 else:
  old='for(const auto& gate:recruitingUiGates){if(!read(base+gate.rva,actual.data(),actual.size())||actual!=gate.bytes)throw std::runtime_error("Unsupported UI-value setter fingerprint");}'
  new='for(const auto& gate:recruitingUiGates){if(!read(base+gate.rva,actual.data(),actual.size()))throw std::runtime_error("Unreadable UI-value setter");if(actual!=gate.bytes)cotcompat::practice::normalize(base,gate.rva,actual.data(),gate.bytes.data(),actual.size(),read);if(actual!=gate.bytes)throw std::runtime_error("Unsupported UI-value setter fingerprint");}'
 assert old in s,name;s=s.replace(old,new);path.write_text(s,encoding='utf-8',newline='\n');changes.append(path)
m=root/'Baselines/changes.json';doc=json.loads(m.read_text())
for path in changes:
 rel=path.relative_to(root).as_posix();doc['changes']=[x for x in doc['changes'] if x['path']!=rel]
 doc['changes'].append(dict(path=rel,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),reason='Recognize only the exact COT Practice text hook with three body/entry attestations; reject all other detours'))
m.write_text(json.dumps(doc,indent=2)+'\n')
