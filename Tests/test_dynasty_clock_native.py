"""Execute COT clock lookup/writer thunks against this process's mock pages only."""
import ctypes as C,json,struct,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'Build/ClockChecks'
IMAGE=0x140000000;DATA=0x200000000
k=C.WinDLL('kernel32',use_last_error=True)
k.VirtualAlloc.argtypes=[C.c_void_p,C.c_size_t,C.c_uint,C.c_uint];k.VirtualAlloc.restype=C.c_void_p
k.VirtualFree.argtypes=[C.c_void_p,C.c_size_t,C.c_uint];k.VirtualFree.restype=C.c_int
pages=[]
def alloc(address,size,protection=4):
    p=k.VirtualAlloc(address,size,0x3000,protection)
    if not p or (address and p!=address):raise OSError(C.get_last_error(),'mock allocation')
    pages.append(p);return p
def put(a,value):C.c_uint32.from_address(a).value=value
def get(a):return C.c_uint32.from_address(a).value
checks=0
def check(ok,message):
    global checks
    assert ok,message
    checks+=1
def snapshot():return C.string_at(DATA+0x1100,8192)
try:
    for offset in [0xd9eb808,0xf3fddbc,0xf55c998,0xf63f9f0]:
        alloc((IMAGE+offset)&~0xffff,0x10000)
    alloc(DATA,0x10000)
    descriptor=(OUT/'BuildData.bin').read_bytes();C.memmove(DATA,descriptor,len(descriptor))
    put(IMAGE+0xd9eb808,14);put(IMAGE+0xf3fddbc,3)
    functions={}
    for name in ['off-read','def-read','off-write','def-write']:
        b=(OUT/(name+'.bin')).read_bytes();p=alloc(None,len(b),0x40);C.memmove(p,b,len(b))
        functions[name]=C.CFUNCTYPE(C.c_int,C.c_int)(p) if 'write' in name else C.CFUNCTYPE(C.c_int)(p)
    off,defense,woff,wdef=[functions[n] for n in ['off-read','def-read','off-write','def-write']]
    def league(n):put(IMAGE+0xf55c998,n)
    league(123);check((off(),defense())==(1,15),'new defaults')
    woff(3);wdef(5);check((off(),defense())==(3,5),'A choices')
    league(0xfedcba98);check((off(),defense())==(1,15),'B defaults')
    woff(2);wdef(0);check((off(),defense())==(2,0),'B choices')
    league(123);check((off(),defense())==(3,5),'same-process switch back to A')
    for o in range(4):
        for d in [0,3,5,15,20,30,45,60]:
            woff(o);wdef(d);check((off(),defense())==(o,d),f'clock pair {o}/{d}')
    league(0xfedcba98);check((off(),defense())==(2,0),'A edits changed B')
    original=snapshot();seq=get(DATA+0x1000)
    for n in [0,0xffffffff]:
        league(n);woff(0);wdef(60)
        check(snapshot()==original and get(DATA+0x1000)==seq,'unknown identity mutated table')
    league(123)
    config=alloc(0x310000000,0x10000)
    C.c_uint64.from_address(IMAGE+0xf63f9f0).value=config
    C.c_ubyte.from_address(config+0x84).value=1
    woff(0);wdef(0)
    check(snapshot()==original and (off(),defense())==(-1,-1),'practice must retain native clocks and reject writes')
    C.c_ubyte.from_address(config+0x84).value=0
    check((off(),defense())==(3,60),'ordinary Dynasty lost its profiles')
    C.c_uint64.from_address(IMAGE+0xf63f9f0).value=0
    for o in [-1,4,255]:
        woff(o);check(snapshot()==original,'invalid offense')
    for d in [-1,1,4,255]:
        wdef(d);check(snapshot()==original,'invalid defense')
    for address,invalid,normal in [(IMAGE+0xd9eb808,0,14),(IMAGE+0xf3fddbc,2,3)]:
        put(address,invalid);woff(0);wdef(0);check(snapshot()==original,'foreign mode changed clocks');put(address,normal)
    put(DATA+0x1004,1);woff(0);check(snapshot()==original and get(DATA+0x1004)==1,'busy writer lock')
    put(DATA+0x1004,0)
    # Reload the actual packed native table into a fresh allocation state.
    C.memset(DATA,0,0x4000);C.memmove(DATA,descriptor,len(descriptor));C.memmove(DATA+0x1100,original,len(original))
    league(0xfedcba98);check((off(),defense())==(2,0),'reload B')
    league(123);check((off(),defense())==(3,60),'reload A')

    # Execute the complete emitted cap hook, with a mock native continuation.
    # No profile accessor page exists: an accidental global-profile read would fail.
    for offset in [0xf85f308,0xe2cf3e0,0x5d05640,0x5d87958]:
        page=(IMAGE+offset)&~0xffff
        if page not in pages:alloc(page,0x10000,0x40 if offset<0x10000000 else 4)
    mock=alloc(0x300000000,0x10000);manager=mock;rules=mock+0x1000
    C.c_uint64.from_address(IMAGE+0xf85f308).value=manager
    C.c_uint64.from_address(IMAGE+0xe2cf3e0).value=1
    C.c_uint64.from_address(IMAGE+0xe2cf3e8).value=1
    C.c_float.from_address(rules+0x6c).value=40
    C.memmove(IMAGE+0x5d05640,b'\xc3',1)
    tail=b'\x66\x0f\x7e\xf0\xc3' # movd eax,xmm6; ret
    C.memmove(IMAGE+0x5d87958,tail,len(tail))
    cap=(OUT/'BuildCapUpdated.bin').read_bytes();cap_addr=alloc(None,len(cap),0x40);C.memmove(cap_addr,cap,len(cap))
    wrapper=(bytes.fromhex('564883ec30f30f7f74242048be')+struct.pack('<Q',rules)
        +bytes.fromhex('48b9')+struct.pack('<Q',manager)+bytes.fromhex('48b8')+struct.pack('<Q',cap_addr)
        +bytes.fromhex('ffd0f30f6f7424204883c4305ec3'))
    wrapper_addr=alloc(None,len(wrapper),0x40);C.memmove(wrapper_addr,wrapper,len(wrapper))
    cap_fn=C.CFUNCTYPE(C.c_uint32)(wrapper_addr)
    league(123);woff(3)
    check(cap_fn()==struct.unpack('<I',struct.pack('<f',60))[0],'full cap hook did not extend current Dynasty')
    woff(0);check(cap_fn()==struct.unpack('<I',struct.pack('<f',40))[0],'full cap hook changed Off mode')
    league(0xfedcba98);check(cap_fn()==struct.unpack('<I',struct.pack('<f',40))[0],'full cap hook leaked across Dynasties')

    for i in range(1024):
        struct.pack_into('<Q',(C.c_ubyte*8192).from_address(DATA+0x1100),i*8,(i+1)|(1<<32)|(15<<40))
    league(2000);full=snapshot();woff(2);check(snapshot()==full and get(DATA+0x1004)==0,'capacity overflow or leaked lock')
    league(1024);woff(3);wdef(3);check((off(),defense())==(3,3),'full table existing record')
    check(get(DATA+0x1000)%2==0,'uncommitted sequence')
    result={'passed':checks,'scope':'emitted clock thunks in mock self-process memory','liveGameAccess':False}
    (OUT/'native-result.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS:',checks,'native clock checks; Dynasty switching, all 32 pairs, reload, gates, capacity and lock ownership. No game process accessed.')
finally:
    for p in reversed(pages):k.VirtualFree(p,0,0x8000)
