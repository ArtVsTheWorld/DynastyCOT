"""Generate exact Practice text-setter ownership recognition from emitted code."""
from pathlib import Path
import json,struct,hashlib
ROOT=Path(__file__).resolve().parents[1]
doc=json.loads((ROOT/'Build/Compatibility/practice-contract.json').read_text())
def array(data):return ','.join('0x%02x'%x for x in data)
parts=['''#pragma once
// Generated from the compiled COT Practice helper. Do not edit byte patterns.
#include <array>
#include <cstdint>
#include <cstring>
namespace cotcompat::practice {
struct Relocation {size_t offset;bool data;uintptr_t addend;};
''']
fixtures=['#pragma once\n']
for block in doc['blocks']:
 i=block['index'];raw=bytes.fromhex(block['variants'][0]);variants=[bytes.fromhex(x) for x in block['variants']]
 rel=[];offset=0
 while offset<=len(raw)-8:
  a=struct.unpack_from('<Q',raw,offset)[0];b=struct.unpack_from('<Q',variants[1],offset)[0];c=struct.unpack_from('<Q',variants[2],offset)[0]
  kind=None
  if b==a+doc['delta'] and c==a and doc['image']<=a<doc['image']+0x10000000:kind=False
  if c==a+doc['delta'] and b==a and doc['data']<=a<doc['data']+0x10000:kind=True
  if kind is None:offset+=1;continue
  rel.append((offset,kind,a-doc['data' if kind else 'image']));offset+=8
 for variant in range(4):
  check=bytearray(raw)
  for offset,kind,add in rel:struct.pack_into('<Q',check,offset,doc['data' if kind else 'image']+add+(doc['delta'] if variant&(2 if kind else 1) else 0))
  assert bytes(check)==variants[variant],(i,variant,'Incomplete relocation contract')
 original=bytes.fromhex(block['original'])
 parts.append(f'inline constexpr std::array<unsigned char,{len(raw)}> block{i}={{{array(raw)}}};\n')
 parts.append(f'inline constexpr std::array<unsigned char,{len(original)}> original{i}={{{array(original)}}};\n')
 parts.append(f'inline constexpr std::array<Relocation,{len(rel)}> reloc{i}={{{{'+','.join('{%d,%s,0x%x}'%(o,str(k).lower(),a) for o,k,a in rel)+'}};\n')
 parts.append(f'inline constexpr uintptr_t site{i}=0x{block["rva"]:x};\n')
 fixtures.append(f'inline const std::array<std::array<unsigned char,{len(raw)}>,4> fixture{i}={{{{'+','.join('{{'+array(v)+'}}' for v in variants)+'}};\n')
 block['relocations']=[dict(offset=o,data=k,addend=a) for o,k,a in rel]
parts.append('''template<size_t N,size_t R,class Read>
bool body(uintptr_t image,uintptr_t data,uintptr_t at,const std::array<unsigned char,N>& prototype,const std::array<Relocation,R>& reloc,Read read) {
 auto expected=prototype;
 for(const auto& r:reloc){uintptr_t value=(r.data?data:image)+r.addend;std::memcpy(expected.data()+r.offset,&value,8);}
 std::array<unsigned char,N> actual{};
 return read(at,actual.data(),N)&&actual==expected;
}
template<size_t N,class Read>
bool entry(uintptr_t image,uintptr_t rva,uintptr_t target,Read read) {
 std::array<unsigned char,N> expected{},actual{};expected.fill(0x90);
 expected[0]=0xff;expected[1]=0x25;expected[2]=expected[3]=expected[4]=expected[5]=0;
 std::memcpy(expected.data()+6,&target,8);
 return read(image+rva,actual.data(),N)&&actual==expected;
}
template<class Read>
bool normalize(uintptr_t image,uintptr_t rva,unsigned char* actual,const unsigned char* native,size_t length,Read read) {
 if(rva!=site14||length!=32||std::memcmp(native,original14.data(),original14.size()))return false;
 if(actual[0]!=0xff||actual[1]!=0x25)return false;
 uintptr_t target=0;std::memcpy(&target,actual+6,8);
 if(target<0x10000+14*1024)return false;
 uintptr_t code=target-14*1024;
 if((code&4095)!=0)return false;
 uintptr_t data=0;
 for(const auto& r:reloc14)if(r.data){if(!read(target+r.offset,&data,8))return false;data-=r.addend;break;}
 if(data<0x10000||(data&4095)!=0)return false;
 if(!entry<original0.size()>(image,site0,code,read)||
    !entry<original13.size()>(image,site13,code+13*1024,read)||
    !entry<original14.size()>(image,site14,target,read))return false;
 if(!body(image,data,code,block0,reloc0,read)||
    !body(image,data,code+13*1024,block13,reloc13,read)||
    !body(image,data,target,block14,reloc14,read))return false;
 // Normalize only our local copy. Actual game memory is never rewritten here.
 std::memcpy(actual,native,original14.size());return true;
}
}
''')
out=ROOT/'Compatibility';out.mkdir(exist_ok=True)
(out/'PracticeTextSetter.generated.h').write_text(''.join(parts),encoding='utf-8')
(ROOT/'Build/Compatibility/PracticeContractFixtures.h').write_text(''.join(fixtures),encoding='utf-8')
version=__import__('re').search(r'PluginVersion\("([^"]+)"\)',(ROOT/'Modules/DrivePractice/Plugin/AssemblyInfo.cs').read_text()).group(1)
doc['helperSha256']=hashlib.sha256((ROOT/('Modules/DrivePractice/Release/SituationalPractice-'+version+'/SituationalPractice/SituationalPractice.Runtime.exe')).read_bytes()).hexdigest()
(out/'practice-contract.json').write_text(json.dumps(doc,indent=2)+'\n')
print('Generated exact three-block Practice text-setter contract; all four relocation combinations verified.')
