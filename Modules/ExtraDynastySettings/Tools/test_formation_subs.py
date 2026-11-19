"""Execute exported release bridges against fake native objects, in this process only.

Requires an explicitly supplied private old-build PE; never opens the live game.
Native metadata/string allocation routines are replaced by inert test stubs.
"""
import argparse, ctypes as C, hashlib, json, re, statistics, struct, time
from pathlib import Path

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--exports',type=Path,required=True)
p.add_argument('--reference',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
p.add_argument('--remote',type=lambda s:int(s,0),default=0x160000000)
p.add_argument('--reserve-near',action='store_true',help='Reserve the entire old nearby arena in THIS process only.')
p.add_argument('--expect-initial-fallback',action='store_true',help='Reproduce the old DLL missing the first-entry owner, for before/after evidence.')
a=p.parse_args()
B=0x140000000; R=a.remote; D=0x220000000 if a.reserve_near else 0x170000000
k=C.WinDLL('kernel32',use_last_error=True)
k.VirtualAlloc.argtypes=[C.c_void_p,C.c_size_t,C.c_uint32,C.c_uint32];k.VirtualAlloc.restype=C.c_void_p
assert k.VirtualAlloc(B,0x10000000,0x2000,4)==B
if a.reserve_near:
    assert R< B-0x60000000 or R>=B+0x60000000
    assert k.VirtualAlloc(B-0x60000000,0x60000000,0x2000,4)==B-0x60000000
    assert k.VirtualAlloc(B+0x10000000,0x50000000,0x2000,4)==B+0x10000000
assert k.VirtualAlloc(R,0x8000,0x3000,0x40)==R
assert k.VirtualAlloc(D,0x800000,0x3000,4)==D
pages=set()
def put(at,data):
    for page in range(at&~4095,(at+len(data)+4095)&~4095,4096):
        if B<=page<B+0x10000000 and page not in pages:
            assert k.VirtualAlloc(page,4096,0x1000,0x40)==page
            pages.add(page)
    C.memmove(at,data,len(data))
def q(at,v):put(at,struct.pack('<Q',v))
def n(at,v):put(at,struct.pack('<I',v&0xffffffff))
def iv(at):return C.c_int.from_address(at).value
def qv(at):return C.c_uint64.from_address(at).value
def hx(s):return bytes.fromhex(s)
cursor=D+0x20000
def alloc(size):
    global cursor
    at=cursor;cursor=(cursor+size+15)&~15
    assert cursor<D+0x800000
    C.memset(at,0,size);return at

pe=a.reference.read_bytes();e=struct.unpack_from('<I',pe,60)[0];opt=e+24
sections=[]
for i in range(struct.unpack_from('<H',pe,e+6)[0]):
    at=opt+struct.unpack_from('<H',pe,e+20)[0]+i*40
    vs,rva,sz,raw=struct.unpack_from('<IIII',pe,at+8);sections.append((rva,sz,raw))
def body(rva,size):
    for start,length,raw in sections:
        if start<=rva and rva+size<=start+length:return pe[raw+rva-start:raw+rva-start+size]
    raise ValueError(hex(rva))
source=(Path(__file__).resolve().parents[1]/'Plugin/FormationSubsBindings.cs').read_text()
contracts=[]
for name,rva,size,digest in re.findall(r'new Region\("([^"]+)",(0x[0-9a-f]+),(0x[0-9a-f]+),"([0-9a-f]+)"\)',source):
    rva=int(rva,16);size=int(size,16);data=body(rva,size)
    assert hashlib.sha256(data).hexdigest()==digest,name
    put(B+rva,data);contracts.append(name)
for kind in range(6):put(R+kind*0x800,(a.exports/f'Bridge{kind}.bin').read_bytes())
for typ,offset in [(2,0x3000),(12,0x3400)]:put(R+offset,(a.exports/f'Item{typ}.bin').read_bytes())
put(R+0x3800,(a.exports/'Find.bin').read_bytes())
put(R+0x3d00,(a.exports/'Lookup.bin').read_bytes())
put(R+0x6100,(a.exports/'FormationGroup.bin').read_bytes())
put(R+0x6500,(a.exports/'GroupedFormationSet.bin').read_bytes())
# Reproduce the real six-entry hook shape in this fake process: the unchanged
# menu CALL reaches the lookup entry, whose native fallback is the exact clone.
put(B+0x3479750,hx('ff2500000000')+struct.pack('<Q',R+0x1000)+b'\x90'*6)
# Native ordinary set wrappers are also executed. Their dependencies above are exact.
for rva,size in [(0x3489720,67),(0x348c720,85)]:put(B+rva,body(rva,size))
# No real allocator or game object is invoked by these fake objects.
for rva in [0x7acca0,0x347e1a0]:put(B+rva,hx('c3'))
put(B+0x347e620,hx('33c0c3'))
put(B+0x3489380,hx('b8d2040000c3')) # recognizable ordinary-path play count

library=D; core=library+0x1d18; vt=D+0x10000; output=D+0x12000
q(library,vt)
for slot,rva in [(0xc0,0x2f24d50),(0x318,0x3489030),(0x340,0x3488850),(0x350,0x3489720),(0x370,0x348c720),(0x390,0x3489380),(0x520,0x2f25700)]:q(vt+slot,B+rva)
outvt=D+0x11000;q(output,outvt);q(outvt+0x20,R+0x3b00);put(R+0x3b00,hx('488d4108c3'))
q(B+0xe2d3918,library);n(B+0xd9eb808,-1);n(B+0xf3fddbc,3);n(B+0xf55c998,123);q(B+0xf63f9f0,0)
front=alloc(0x800);career=alloc(0x500);owner=alloc(0x100);binding=alloc(0x30);child=alloc(0x200);db=alloc(0x30)
q(B+0xfa519b8,front);q(front,B+0xbaa8938);put(front+0x7b1,b'\1');q(front+0x468,career)
q(career,B+0xbce9a78);q(career+0x230,owner);q(career+0x240,binding);q(career+0x238,db)
q(owner,B+0xb4dbab8);q(binding,owner);q(binding+8,child);q(child,B+0xbd0d2f8);q(child+0x1a8,db);q(db,B+0xbd0d258)

def tree(header,items,write):
    items=sorted(items)
    def build(rows,parent=header):
        if not rows:return 0
        m=len(rows)//2;key,value=rows[m];node=alloc(0x400)
        n(node+0x20,key);q(node+0x10,parent);write(node,value)
        q(node,build(rows[m+1:],node));q(node+8,build(rows[:m],node));return node
    q(header+0x10,build(items));n(header+0x20,len(items))
    # Core helpers use root and count; sentinel min/max are unused here.
    q(header,header);q(header+8,header)
def nested(node,value,writer):tree(node+0x28,value,writer)
def metadata(node,value):
    formid,ordinal,typ=value
    at=node+0x28;n(at+0x310,formid);n(at+0x314,ordinal);n(at+0x318,typ);q(at+0x308,15)
def make_books(ordinary,extra,collision=False):
    """Arbitrary IDs and multiple sets; no observed school/book IDs are baked in."""
    typ=1 if ordinary==1 else 11;n(library+0x344,typ)
    forms={typ:[(103+i*7,(103+i*7,i,typ)) for i in range(ordinary)],typ+1:[(803+i*11,(803+i*11,i,typ+1)) for i in range(extra)],typ+2:[(999,(999,0,typ+2))]}
    if collision and extra:forms[typ+1][0]=(103,(103,0,typ+1))
    tree(core+0xd0,forms.items(),lambda node,v:nested(node,v,metadata))
    ordinals={t:[(v[1][1],v[0]) for v in fs] for t,fs in forms.items()}
    tree(core+0x130,ordinals.items(),lambda node,v:nested(node,v,lambda nd,fid:n(nd+0x24,fid)))
    setmaps={t:[(v[1][1],[(j,10000+t*100+v[1][1]*10+j) for j in range(2)]) for v in fs] for t,fs in forms.items()}
    tree(core+0x310,setmaps.items(),lambda nd,v:nested(nd,v,lambda nd2,v2:nested(nd2,v2,lambda leaf,sid:n(leaf+0x28+0x220,sid))))
    plays={t:[(v[1][1],[(j,[]) for j in range(2)]) for v in fs] for t,fs in forms.items()}
    tree(core+0x10,plays.items(),lambda nd,v:nested(nd,v,lambda nd2,v2:nested(nd2,v2,lambda leaf,pl:tree(leaf+0x28,[(i,i) for i in range(3)],lambda nd3,x:n(nd3+0x24,x)))))
    put(core,b'\1')
    return typ

callbacks=[0x37609f4,0x3760a63,0x3760779,0x3760799,0x37607c8,0x37607f5]
def thunk(kind,wrong_outer=False,wrong_inner=False,arg5=0,arg6=-1,enumeration=False,initial=False,unrelated=False):
    immediate=callbacks[kind]
    isenum=kind in [0,1] or enumeration
    if enumeration and kind>=3:immediate=[0,0,0,0x3760abf,0x3760afd,0x3760b52][kind]
    outerret=B+((0x37117ab if unrelated else 0x7fc9d2b if initial else 0x7fdd27f) if isenum else 0x7fc9d92)
    frame=0x3d8 if isenum else 0x398
    innerret=B+immediate
    if wrong_inner:innerret+=0x2000
    if wrong_outer:outerret+=0x2000
    code=hx('4881ec')+struct.pack('<I',frame)
    code+=hx('48b8')+struct.pack('<Q',arg5)+hx('4889442420')
    code+=hx('c7442428')+struct.pack('<I',arg6&0xffffffff)
    code+=hx('48b8')+struct.pack('<Q',B+0x3479750 if kind==2 else R+kind*0x800)+hx('ffd0')
    inner=innerret-len(code);put(inner,code+hx('4881c4')+struct.pack('<I',frame)+hx('c3'))
    outer=outerret-16
    put(outer,hx('4883ec2848b8')+struct.pack('<Q',inner)+hx('ffd04883c428c3'))
    return C.WINFUNCTYPE(C.c_uint64,C.c_uint64,C.c_uint64,C.c_uint64,C.c_uint64)(outer)

checks=0
def check(value,message):
    global checks
    assert value,message;checks+=1
def call(kind,*args,**kw):return thunk(kind,**kw)(*args)
def counter(kind,field):return iv(R+0x4000+kind*0x20+field)
for ordinary,extra in [(1,2),(11,3)]:
    typ=make_books(ordinary,extra)
    initial_count=call(0,library,0,typ,0,initial=True)
    check(initial_count==(ordinary if a.expect_initial_fallback else ordinary+extra),'first-entry owner before/after count')
    check(call(0,library,0,typ,0,unrelated=True)==ordinary,'other native menu owner remains unchanged')
    if not a.expect_initial_fallback:
        call(1,library,0,output,ordinary,initial=True)
        check(iv(output+0x310)==803 and iv(output+0x314)==0x10000000,'initial-list item real ID/tag')
        check(call(3,library,0,0x10000000,0,enumeration=True,initial=True)==2,'initial-list tagged set count')
        sp=call(4,library,0,0x10000000,1,enumeration=True,initial=True)
        check(iv(sp+0x220)==10000+(typ+1)*100+1,'initial-list tagged set ID')
        check(call(5,library,0,0x10000000,1,enumeration=True,initial=True)==3,'initial-list play count')
    count=call(0,library,0,typ,0)
    check(count==ordinary+extra,f'count {typ}: {count}')
    check(counter(0,0)>0 and counter(0,4)>0 and counter(0,8)>0,'bounded callback-stage diagnostics')
    for ordinal in range(ordinary+extra):
        C.memset(output+8,0,0x328)
        call(1,library,0,output,ordinal)
        special=ordinal>=ordinary;ix=ordinal-ordinary if special else ordinal
        check(iv(output+0x310)==(803+ix*11 if special else 103+ix*7),'real formation ID')
        check(iv(output+0x314)==(ix|0x10000000 if special else ix),'transient ordinal')
        check(iv(output+0x318)==typ+int(special),'native type')
    for fid,wanted in [(103,typ),(803,typ+1),(987654,0)]:
        ok=call(2,core,0,typ,fid,arg5=output)&0xff
        check(bool(ok)==bool(wanted),'lookup success/missing')
        if wanted:check(iv(output+0x318)==wanted and iv(output+0x310)==fid,'lookup retains ID/type')
    ordinary_lookup=C.WINFUNCTYPE(C.c_uint64,C.c_uint64,C.c_int,C.c_int,C.c_int,C.c_uint64)(B+0x3479750)
    for fid,wanted in [(103,True),(803,False),(987654,False)]:
        ok=ordinary_lookup(core,0,typ,fid,output)&0xff
        check(bool(ok)==wanted,'non-menu original lookup fallback')
        if wanted:check(iv(output+0x314)==0 and iv(output+0x310)==103,'fallback has untagged native ID/index')
    for enum in [False,True]:
        check(call(3,library,0,0x10000000,0,enumeration=enum)==2,'tagged set count')
        sp=call(4,library,0,0x10000000,1,enumeration=enum)
        check(iv(sp+0x220)==10000+(typ+1)*100+1,'tagged set ID')
        check(call(5,library,0,0x10000000,1,enumeration=enum)==3,'tagged play count')
    check(call(3,library,0,0,0)==2,'ordinary set count unchanged')
    check(call(5,library,0,0,0)==1234,'ordinary play path unchanged')
    check(call(5,library,0,0x10000000,0,arg6=0)==1234,'non-native filter fallback')
    for kw in [dict(wrong_outer=True),dict(wrong_inner=True)]:check(call(0,library,0,typ,0,**kw)==ordinary,'caller isolation')
    for at,value,width in [(B+0xd9eb808,14,4),(B+0xf3fddbc,4,4),(B+0xf55c998,0,4),(B+0xf55c998,0xffffffff,4),(front+0x7b1,0,1),(front,0,8),(career,0,8),(binding,0,8),(child+0x1a8,0,8),(db,0,8)]:
        original=C.string_at(at,width);put(at,value.to_bytes(width,'little'))
        check(call(0,library,0,typ,0)==ordinary,'scope fallback '+hex(at));put(at,original)
    config=alloc(0x100);put(config+0x84,b'\1');q(B+0xf63f9f0,config)
    check(call(0,library,0,typ,0)==ordinary,'online configuration fallback');q(B+0xf63f9f0,0)

for extra in [0,9]:
    make_books(1,extra);check(call(0,library,0,1,0)==1,'absent/oversize extension fallback')
make_books(1,2,True);check(call(0,library,0,1,0)==1,'ambiguous formation ID fallback')
make_books(1,2)
# Another book slot must use its own native ordinary records, never slot zero.
n(library+0xde8+0x344,1)
tree(core+0xd0+0x30,[(1,[(503,(503,0,1))])],lambda nd,v:nested(nd,v,metadata))
check(call(0,library,1,1,0)==1,'nonzero library slot retains native count')
check(call(0,library,0,2,0)==2,'different count type retains native result')
for ordinal in [0x10000008,0x20000000]:
    check(call(5,library,0,ordinal,0)==1234,'out-of-range tagged index fallback')
config=alloc(0x100);q(B+0xf63f9f0,config)
check(call(0,library,0,1,0)==3,'verified offline configuration is accepted');q(B+0xf63f9f0,0)
find=C.WINFUNCTYPE(C.c_uint64,C.c_uint64,C.c_int,C.c_int)(R+0x3800)
check(find(core,2,803)!=0 and find(core,2,987654)==0 and find(core,7,803)==0,'bounded metadata lookup')
root=qv(core+0xd0+0x10);original=C.string_at(root,8);q(root,root)
check(find(core,999,1)==0,'cyclic tree bound');put(root,original)
f=thunk(0);native=C.WINFUNCTYPE(C.c_uint64,C.c_uint64,C.c_uint64,C.c_uint64,C.c_uint64)(B+0x3489030)
timings={}
for name,fn in [('native',native),('candidate',f)]:
    rows=[]
    for j in range(7):
        start=time.perf_counter_ns()
        for i in range(10000):fn(library,0,1,0)
        rows.append((time.perf_counter_ns()-start)/10000/1000)
    timings[name]=dict(median=statistics.median(rows),slow=max(rows))
report=dict(passed=checks,contracts=len(contracts),gameAccess=False,scope='own-process fake objects; exact native map/count/copy code, allocator/string stubs',callbackUs=timings,pending='real menu rendering, native save/reload and on-field assignment acceptance')
a.output.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
