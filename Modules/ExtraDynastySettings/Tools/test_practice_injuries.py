"""Execute release-DLL practice bridges on fake objects in THIS process only."""
import argparse, base64, ctypes as C, json, statistics, struct, time
from pathlib import Path

ap=argparse.ArgumentParser(description=__doc__)
ap.add_argument('exports',type=Path);ap.add_argument('output',type=Path)
args=ap.parse_args();exports=json.loads(args.exports.read_bytes())
B=0x140000000;D=0x220000000;R=0x160000000;S=0x230000000
k=C.WinDLL('kernel32',use_last_error=True)
k.VirtualAlloc.argtypes=[C.c_void_p,C.c_size_t,C.c_uint32,C.c_uint32];k.VirtualAlloc.restype=C.c_void_p
assert k.VirtualAlloc(B,0x10000000,0x2000,4)==B
for at,n,prot in [(D,0x4000,4),(R,0x5000,0x40),(S,0x10000,4)]:assert k.VirtualAlloc(at,n,0x3000,prot)==at
pages=set()
def put(at,data):
    if B<=at<B+0x10000000:
        for p in range(at&~4095,(at+len(data)+4095)&~4095,4096):
            if p not in pages:assert k.VirtualAlloc(p,4096,0x1000,0x40)==p;pages.add(p)
    C.memmove(at,data,len(data))
def q(at,n):put(at,struct.pack('<Q',n))
def n(at,v):put(at,struct.pack('<I',v&0xffffffff))
def iv(at):return C.c_int.from_address(at).value
def qv(at):return C.c_uint64.from_address(at).value
def hx(s):return bytes.fromhex(s)
def movrax(at):return hx('48b8')+struct.pack('<Q',at)
def jump(at):return hx('ff2500000000')+struct.pack('<Q',at)
def call(site,target):return hx('e8')+struct.pack('<i',target-site-5)

put(D,base64.b64decode(exports['data']))
for name,at in [('bridge0',R),('bridge1',R+0x800),('read',R+0x1800),('write',R+0x1c00)]:put(at,base64.b64decode(exports[name]))
for at in [0xd9eb808,0xf3fddbc,0xf55c998,0xfa519b8,0xf63f9f0]:q(B+at,0)
def scope():
    n(B+0xd9eb808,-1);n(B+0xf3fddbc,3);n(B+0xf55c998,101)
    q(B+0xfa519b8,S+0x1000);q(S+0x1000,B+0xbaa8938);put(S+0x17b1,b'\1')
    q(S+0x1468,S+0x2000);q(S+0x2000,B+0xbce9a78)
    q(S+0x2230,S+0x3000);q(S+0x3000,B+0xb4dbab8)
    q(S+0x2240,S+0x4000);q(S+0x4000,S+0x3000);q(S+0x4008,S+0x5000)
    n(D+0x1004,0)
def record(mode,league=101,offense=3,defense=5):
    q(D+0x1100,league|(offense<<32)|(defense<<40)|(mode<<48));q(D+0x1108,202|(2<<32)|(15<<40))

roll_caller=B+0x8152926;risk_caller=B+0x887b700
put(R+0x3100,hx('4881c4600100005dc3'))
put(B+0x828f0f6,hx('b801000000')+jump(R+0x3100))
put(B+0x828f2ef,hx('33c0')+jump(R+0x3100))
# Native decision frame: return address +0x158, saved caller RBP +0x150.
wrapper=(hx('554881ec60010000')+movrax(S+0x6000)+hx('4889842450010000')+
         movrax(S+0x7010)+hx('488b004889842458010000')+
         movrax(S+0x7000)+hx('f30f10300f29b42400010000f30f104004')+jump(R))
put(R+0x3000,wrapper)
# Preview's native return path restores its actual six pushes and 0xc8 frame.
put(B+0x828ddc0,hx('4881c4c8000000415f415e5f5e5d5bb807000000c3'))
put(R+0x3500,hx('4881c4800000005dc3'))
for caller in [risk_caller,risk_caller+0x40]:
    put(caller-5,call(caller-5,R+0x800));put(caller,jump(R+0x3500))
def preview_wrapper(caller):
    return hx('554881ec80000000')+movrax(S+0x7008)+hx('8b0089442470')+jump(caller-5)
put(R+0x3400,preview_wrapper(risk_caller));put(R+0x3600,preview_wrapper(risk_caller+0x40))
roll=C.CFUNCTYPE(C.c_int)(R+0x3000);risk=C.CFUNCTYPE(C.c_int)(R+0x3400)
wrong_risk=C.CFUNCTYPE(C.c_int)(R+0x3600)
read=C.CFUNCTYPE(C.c_int)(R+0x1800);write=C.CFUNCTYPE(None,C.c_int)(R+0x1c00)
checks=0
def check(value,message):
    global checks
    assert value,message
    checks+=1
def player(position,chance=1.0,random=0.0):
    q(S+0x6000-0x19,position*4);n(S+0x7008,position)
    put(S+0x7000,struct.pack('<ff',chance,random));q(S+0x7010,roll_caller)

scope()
for mode in range(3):
    for position in range(21):
        record(mode);player(position)
        suppress=mode==2 or (mode==1 and position==0)
        observed=roll();check(observed==(0 if suppress else 1),f'new injury decision {mode}/{position}: {observed}, choice {read()}, record {qv(D+0x1100):x}')
        check(risk()==(0 if suppress else 7),f'risk preview {mode}/{position}')
        check((qv(D+0x1100)>>32)&0xffff==0x0503,'clocks unchanged')
        for chance,random in [(0.0,0.5),(0.5,0.5),(0.1,0.5)]:
            player(position,chance,random);check(roll()==0,'native unsuccessful roll preserved')
player(0);record(2)
for case in ['unknown-caller','mode','network','zero-league','sentinel-league','no-career','career-vtable','local-owner','database-vtable','database-binding']:
    scope();record(2);player(0)
    if case=='unknown-caller':q(S+0x7010,roll_caller+1)
    elif case=='mode':n(B+0xd9eb808,5)
    elif case=='network':n(B+0xf3fddbc,0)
    elif case=='zero-league':n(B+0xf55c998,0)
    elif case=='sentinel-league':n(B+0xf55c998,-1)
    elif case=='no-career':q(B+0xfa519b8,0)
    elif case=='career-vtable':q(S+0x1000,0)
    elif case=='local-owner':put(S+0x17b1,b'\0')
    elif case=='database-vtable':q(S+0x3000,0)
    else:q(S+0x4000,S+0x3008)
    check(roll()==1,'scope fallback '+case)
    if case!='unknown-caller':check(risk()==7,'risk scope fallback '+case)
scope();check(wrong_risk()==7,'unrelated preview caller unchanged')
for mode in range(3):
    record(mode);check(read()==mode,'native choice read')
    for choice in range(3):
        write(choice);check(read()==choice,'native choice write')
        check((qv(D+0x1100)>>32)&0xffff==0x0503,'practice write preserves clock bits')
before=qv(D+0x1100);sequence=iv(D+0x1000)
for bad in [-1,3,256]:write(bad);check(qv(D+0x1100)==before and iv(D+0x1000)==sequence,'invalid choice rejected')
n(D+0x1004,1);write(0);check(qv(D+0x1100)==before,'busy lock never spins or changes data');n(D+0x1004,0)
n(B+0xf55c998,202);check(read()==0,'Dynasty switch does not inherit choice')
n(B+0xf55c998,303);check(read()==0,'unset Dynasty uses native injuries');write(1);check(read()==1,'new Dynasty choice')
n(B+0xf55c998,0);sequence=iv(D+0x1000);write(2);check(iv(D+0x1000)==sequence and read()==-1,'no validated Dynasty cannot save')
scope();record(1);player(0)
samples=[]
for _ in range(7):
    start=time.perf_counter()
    for i in range(10000):roll()
    samples.append((time.perf_counter()-start)*1e6/10000)
result=dict(checks=checks,medianBridgeUs=statistics.median(samples),slowBatchBridgeUs=max(samples),liveGameAccess=False,
            note='Actual exported DLL instructions on fake objects. Python/ctypes included; not live menu timing.')
args.output.write_text(json.dumps(result,indent=2))
print(f'PASS {checks} native practice decision, all 21 positions, preview, scope, caller, Dynasty, lock and writer cases; median {result["medianBridgeUs"]:.3f} us. No game access.')
