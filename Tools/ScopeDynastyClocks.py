from pathlib import Path
import json,hashlib
root=Path(r'C:\Apps\CFB 27\MMC\DynastyCOT');mod=root/'Modules/ExtraDynastySettings';changed=[]
def edit(rel,fn):
 p=mod/rel;s=p.read_text();out=fn(s);assert out!=s,rel;p.write_text(out,encoding='utf-8',newline='\n');changed.append(p)
guard='''            c.MovR10(image+0xf63f9f0);c.Hex("4d8b124d85d2");c.Branch("0f84",p+"scopeReady");
            c.Hex("4180ba8400000000");c.Branch("0f85",p+"done");
            c.Label(p+"scopeReady");
'''
def native(s):
 s=s.replace('bool defense,string p) {\n            c.Hex("9c5152','bool defense,string p,bool excludePractice=true) {\n            c.Hex("9c5152',1)
 s=s.replace('c.Hex("41b9");c.Int(defense?15:1);','c.Hex("41b9");c.Int(-1);',1)
 needle='            c.MovR10(data+DynastyClockProfiles.TableOffset);c.Hex("41b8");'
 s=s.replace(needle,'            if(excludePractice){\n'+guard+'            }\n            c.Hex("41b9");c.Int(defense?15:1);\n'+needle,1)
 needle='            // Try once: do not block the UI or spin over a concurrent writer.'
 s=s.replace(needle,guard+needle,1)
 return s
edit('Plugin/DynastyClockNative.cs',native)
def main(s):
 for suffix in ['Runtime','Enum']:
  needle='AppendDynastyClockRead(c,b,d,false,"cot'+suffix+'");'
  s=s.replace(needle,needle+'\n            c.Hex("83f8ff");c.Branch("0f84","restore");',1)
 return s
edit('Shared/Clock/DynastyPlayClockAction.cs',main)
def menu(s):
 s=s.replace('AppendDynastyClockRead(c,b,d,false,"cotMenu");','AppendDynastyClockRead(c,b,d,false,"cotMenu",false);c.Hex("83f8ff");c.Branch("0f84","done");',1)
 s=s.replace('AppendDynastyClockRead(c,b,d,true,"cotDefMenu");','AppendDynastyClockRead(c,b,d,true,"cotDefMenu",false);c.Hex("83f8ff");c.Branch("0f84","done");',1)
 s=s.replace('c.Hex("ffc34883c428c3");','c.Hex("ffc3");c.Label("done");c.Hex("4883c428c3");',1)
 return s
edit('Plugin/CombinedClockRuntime.cs',menu)
edit('Plugin/DefensivePlayClock.cs',lambda s:s.replace('AppendDynastyClockRead(c,b,d,true,"cotDefEnable");','AppendDynastyClockRead(c,b,d,true,"cotDefEnable");c.Hex("83f8ff");c.Branch("0f84","defensiveGateDone");',1))
edit('Tools/GameSettingsMenuChecks.cs',lambda s:s.replace('0xf55c998, 0xf905618','0xf63f9f0, 0xf55c998, 0xf905618',1))
p=root/'Tests/test_dynasty_clock_native.py';s=p.read_text()
s=s.replace('[0xd9eb808,0xf3fddbc,0xf55c998]','[0xd9eb808,0xf3fddbc,0xf55c998,0xf63f9f0]')
needle='    for o in [-1,4,255]:'
s=s.replace(needle,'''    config=alloc(0x310000000,0x10000)
    C.c_uint64.from_address(IMAGE+0xf63f9f0).value=config
    C.c_ubyte.from_address(config+0x84).value=1
    woff(0);wdef(0)
    check(snapshot()==original and (off(),defense())==(-1,-1),'practice must retain native clocks and reject writes')
    C.c_ubyte.from_address(config+0x84).value=0
    check((off(),defense())==(3,60),'ordinary Dynasty lost its profiles')
    C.c_uint64.from_address(IMAGE+0xf63f9f0).value=0
'''+needle,1)
p.write_text(s,encoding='utf-8',newline='\n')
m=root/'Baselines/changes.json';doc=json.loads(m.read_text())
for p in changed:
 rel=p.relative_to(root).as_posix();doc['changes']=[e for e in doc['changes'] if e['path']!=rel]
 doc['changes'].append(dict(path=rel,sha256=hashlib.sha256(p.read_bytes()).hexdigest(),reason='Per-Dynasty clocks with explicit unknown-identity passthrough and practice exclusion'))
m.write_text(json.dumps(doc,indent=2)+'\n')
