from pathlib import Path
import hashlib,json
root=Path(r'C:\Apps\CFB 27\MMC\DynastyCOT')
path=root/'Modules/ExtraDynastySettings/Shared/Clock/ExtendedClock.cs'
s=path.read_text()
old='            c.Call(b + layout.Map(0x27ac630)); c.Hex("4885c0"); c.Branch("0f84", "restore");\n            c.Hex("488bc8488b00ba01000000ff501083f801"); c.Branch("0f85", "restore");\n'
assert s.count(old)==4,s.count(old)
s=s.replace(old,'            // COT choice was validated by the current-Dynasty lookup above; the global profile is not consulted.\n')
path.write_text(s,encoding='utf-8',newline='\n')
m=root/'Baselines/changes.json';doc=json.loads(m.read_text())
for entry in doc['changes']:
    if entry['path']==path.relative_to(root).as_posix():entry['sha256']=hashlib.sha256(path.read_bytes()).hexdigest()
m.write_text(json.dumps(doc,indent=2)+'\n')
test=root/'Tests/test_dynasty_clock_native.py'
s=test.read_text();marker='    for i in range(1024):\n'
assert marker in s
s=s.replace(marker,"\n    # Execute the complete emitted cap hook, with a mock native continuation.\n    # No profile accessor page exists: an accidental global-profile read would fail.\n    for offset in [0xf85f308,0xe2cf3e0,0x5d05640,0x5d87958]:\n        page=(IMAGE+offset)&~0xffff\n        if page not in pages:alloc(page,0x10000,0x40 if offset<0x10000000 else 4)\n    mock=alloc(0x300000000,0x10000);manager=mock;rules=mock+0x1000\n    C.c_uint64.from_address(IMAGE+0xf85f308).value=manager\n    C.c_uint64.from_address(IMAGE+0xe2cf3e0).value=1\n    C.c_uint64.from_address(IMAGE+0xe2cf3e8).value=1\n    C.c_float.from_address(rules+0x6c).value=40\n    C.memmove(IMAGE+0x5d05640,b'\\xc3',1)\n    tail=b'\\x66\\x0f\\x7e\\xf0\\xc3' # movd eax,xmm6; ret\n    C.memmove(IMAGE+0x5d87958,tail,len(tail))\n    cap=(OUT/'BuildCapUpdated.bin').read_bytes();cap_addr=alloc(None,len(cap),0x40);C.memmove(cap_addr,cap,len(cap))\n    wrapper=(bytes.fromhex('564883ec30f30f7f74242048be')+struct.pack('<Q',rules)\n        +bytes.fromhex('48b9')+struct.pack('<Q',manager)+bytes.fromhex('48b8')+struct.pack('<Q',cap_addr)\n        +bytes.fromhex('ffd0f30f6f7424204883c4305ec3'))\n    wrapper_addr=alloc(None,len(wrapper),0x40);C.memmove(wrapper_addr,wrapper,len(wrapper))\n    cap_fn=C.CFUNCTYPE(C.c_uint32)(wrapper_addr)\n    league(123);woff(3)\n    check(cap_fn()==struct.unpack('<I',struct.pack('<f',60))[0],'full cap hook did not extend current Dynasty')\n    woff(0);check(cap_fn()==struct.unpack('<I',struct.pack('<f',40))[0],'full cap hook changed Off mode')\n    league(0xfedcba98);check(cap_fn()==struct.unpack('<I',struct.pack('<f',40))[0],'full cap hook leaked across Dynasties')\n"+'\n'+marker)
test.write_text(s,encoding='utf-8',newline='\n')
