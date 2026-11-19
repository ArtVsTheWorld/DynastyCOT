"""Execute release preview code with fake objects in this process only.

The producer/writers are inert test callbacks. No game, save or installed files
are opened. The companion harness verifies native contracts against an explicit
private reference and tests the existing six formation bridges first.
"""
import ctypes as C, json, math, runpy, statistics, struct, sys, time
from pathlib import Path

previous_preview=None
if '--previous-preview' in sys.argv:
    i=sys.argv.index('--previous-preview');previous_preview=Path(sys.argv[i+1]);del sys.argv[i:i+2]
ctx=runpy.run_path(str(Path(__file__).with_name('test_formation_subs.py')))
globals().update({k:v for k,v in ctx.items() if not k.startswith('__')})
base_passed=checks
checks=0
def check(value,message):
    global checks
    assert value,message
    checks+=1
for name,off in [('FormationPreview',0x5000),('ReturnSetMembership',0x5800),('PreviewTreeFind',0x5b00),('PreviewFloat',0x5e00),('PreviewString',0x5f00)]:
    put(R+off,(a.exports/(name+'.bin')).read_bytes())
q(R+0x6000+0x28,R+0x5e00);q(R+0x6000+0x30,R+0x5f00)
q(library,B+0xb3324d0)
adapter=alloc(0x500);q(adapter,B+0xbae21b8);q(B+0xfa5ea78,adapter)
output=B+0xda1dda0;q(output,B+0xb5757a8)
make_books(11,3)

def nodes(root,limit=512):
    pending=[root];seen=set()
    while pending:
        at=pending.pop()
        if not at:continue
        assert at not in seen and len(seen)<limit
        seen.add(at);yield at;pending.extend([qv(at),qv(at+8)])

return_objects=[]
for tn in nodes(qv(core+0x320)):
    for fn in nodes(qv(tn+0x38)):
        for sn in nodes(qv(fn+0x38)):
            q(sn+0x28,B+0xb2e82e0)
            if iv(tn+0x20)==12:return_objects.append(sn+0x28)
sid=iv(return_objects[0]+0x220)
membership=C.WINFUNCTYPE(C.c_uint64,C.c_uint64,C.c_int)(R+0x5800)
for obj in return_objects:check(membership(core,iv(obj+0x220))==1,'current book return-set membership')
for invalid in [0,11100,11300,999999]:check(membership(core,invalid)==0,'non-return set excluded')

heard=[];labels=[];calls=[];source_y=[];source_x=[];partial=11
Float=C.WINFUNCTYPE(None,C.c_uint64,C.c_int,C.c_uint64,C.c_float)
String=C.WINFUNCTYPE(None,C.c_uint64,C.c_int,C.c_uint64,C.c_uint64)
@Float
def writer(this,index,key,value):
    assert this==output
    heard.append((index,key,float(value)))
@String
def string_writer(this,index,key,value):
    assert this==output;labels.append((index,key,value))
q(B+0xb5757a8+0x28,C.cast(writer,C.c_void_p).value)
q(B+0xb5757a8+0x30,C.cast(string_writer,C.c_void_p).value)

Producer=C.WINFUNCTYPE(C.c_uint64,C.c_uint64,C.c_uint64,C.c_uint32,C.c_uint64,C.c_uint64)
@Producer
def producer(this,dest,set_id,is_offense,flipped):
    calls.append((this,set_id,is_offense,flipped))
    vt=qv(dest)
    f=Float(qv(vt+0x28));s=String(qv(vt+0x30))
    for i in range(partial):
        f(dest,i,B+0xad964c0,source_x[i]);f(dest,i,B+0xad964e4,source_y[i])
        s(dest,i,B+0xbac4328,100+i)
    return 0x123456789abcdef0
put(B+0x7b5f6a0,hx('ff2500000000')+struct.pack('<Q',C.cast(producer,C.c_void_p).value))

def invoke(*,wrong_caller=False,singular_caller=False,offense=0,flipped=0,set_id=None):
    # A real x64 call with the exact producer's immediate owner; arg 5 remains
    # on its native stack slot. No game callback/interpreter is executed.
    code=hx('4883ec28488b442450488944242048b8')+struct.pack('<Q',R+0x5000)+hx('ffd0')
    end=B+(0x7b35e01 if singular_caller else 0x7b43251)+(0x2000 if wrong_caller else 0)
    at=end-len(code);put(at,code+hx('4883c428c3'))
    return Producer(at)(adapter,output,sid if set_id is None else set_id,offense,flipped)

def render(ys,**kw):
    global source_y,source_x,partial
    source_y=list(ys);source_x=[float(i*8-40) for i in range(11)]
    heard.clear();labels.clear();calls.clear()
    ret=invoke(**kw)
    check(ret==0x123456789abcdef0,'producer return and nonvolatile ABI preserved')
    check(len(calls)==1,'exactly one native producer invocation')
    check([x[2] for x in heard if x[1]==B+0xad964c0]==source_x[:partial],'x fields preserved')
    check(labels==[(i,B+0xbac4328,100+i) for i in range(partial)],'labels/order/slot identity preserved')
    result=[x[2] for x in heard if x[1]==B+0xad964e4]
    check(len(result)==partial,'one y delivery per native player')
    return result

fixtures={
    'return466':[-46,-46,-10,-10,-6,-6,46,46,46,46,46],
    'return6':[-60,-40,-10,-10,-6,-6,40,40,40,40,40],
    'short-return1373':[12.959866,34,34,34,48,48,48,48,48,48,48],
}
for name,ys in fixtures.items():
    for flip in [0,1]:
        got=render(ys,flipped=flip)
        expected=[C.c_float(v+8).value for v in ys] if max(ys)-min(ys)<=40 else [max(ys)+(y-max(ys))*40/(max(ys)-min(ys))+8 for y in ys]
        check(all(abs(x-y)<0.00002 for x,y in zip(got,expected)),name+' fit formula')
        check(abs(max(got)-max(ys)-8)<0.00002,'front line remains anchored')
        check(max(got)-min(got)<=40.00002,'bounded display depth')
        check(calls[0][3]==flip,'native flip argument preserved')

ys=fixtures['return6']
if previous_preview:
    current=C.string_at(R+0x5000,len((a.exports/'FormationPreview.bin').read_bytes()))
    put(B+0x7b16eb0,hx('ff2500000000')+struct.pack('<Q',C.cast(producer,C.c_void_p).value))
    put(R+0x5000,previous_preview.read_bytes())
    check(render(ys)==ys,'0.2.17 guard rejects actual plural binding, reproducing unchanged preview')
    put(R+0x5000,current)
    check(max(render(ys))-min(render(ys))<=40.00002,'correct binding now accepts and fits identical input')
for kw in [dict(wrong_caller=True),dict(singular_caller=True),dict(offense=1),dict(set_id=11100)]:
    check(render(ys,**kw)==ys,'unrelated caller/offense/ordinary formation unchanged')
for at,value,width in [(B+0xd9eb808,14,4),(B+0xf55c998,0,4),(B+0xf3fddbc,4,4),(front+0x7b1,0,1),(child+0x1a8,0,8)]:
    before=C.string_at(at,width);put(at,value.to_bytes(width,'little'))
    check(render(ys)==ys,'scope protection');put(at,before)
partial=5;check(render(ys)==ys[:5],'incomplete producer map forwarded without fitting');partial=11
for value in [float('nan'),float('inf'),-float('inf'),1e30,-1e30]:
    bad=ys.copy();bad[8]=value;got=render(bad)
    check(all((math.isnan(x) and math.isnan(y)) or x==y for x,y in zip(got,[C.c_float(v).value for v in bad])),'nonfinite values forwarded unchanged')
before=C.string_at(core+0x310,0x30);q(core+0x320,0)
check(render(ys)==ys,'missing current book safely retains native preview');put(core+0x310,before)
# No invocation may write the source library/set records or use shared y scratch.
snapshot={obj:C.string_at(obj,0x230) for obj in return_objects}
for i in range(50):render(fixtures['return466' if i%2 else 'return6'])
check(all(C.string_at(obj,0x230)==data for obj,data in snapshot.items()),'native set/assignment records unchanged')

timings={}
for label,target in [('native',B+0x7b5f6a0),('candidate',R+0x5000)]:
    # Use the same controlled producer, output and inputs. Python test callbacks
    # dominate these numbers; this is an isolated overhead check, not game FPS.
    code=hx('4883ec28488b442450488944242048b8')+struct.pack('<Q',target)+hx('ffd0')
    at=B+0x7b43251-len(code);put(at,code+hx('4883c428c3'));fn=Producer(at)
    rows=[]
    for _ in range(5):
        t=time.perf_counter_ns()
        for i in range(100):
            heard.clear();labels.clear();calls.clear();fn(adapter,output,sid,0,0)
        rows.append((time.perf_counter_ns()-t)/100/1000)
    timings[label]=dict(medianUs=statistics.median(rows),slowUs=max(rows))
report=dict(passed=checks,companionPassed=base_passed,previewProducer='controlled fake producer; released native machine-code wrapper/proxy executed',callbackUs=timings,gameAccess=False,sourceRecordsChanged=False,visualAcceptance='pending user test',fitDepth=40,anchor='maximum y / front line, plus 8 display units',previousGuardReproduction=bool(previous_preview))
a.output.with_name(a.output.stem+'-preview.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report))
