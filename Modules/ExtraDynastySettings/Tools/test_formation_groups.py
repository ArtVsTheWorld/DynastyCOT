"""Execute grouped ST routing with fake playbooks; no live game/save access."""
import runpy,ctypes as C,json,time,statistics
from pathlib import Path

ctx=runpy.run_path(str(Path(__file__).with_name('test_formation_subs.py')))
globals().update({k:v for k,v in ctx.items() if not k.startswith('__')})
base_passed=checks;checks=0
def check(value,message):
    global checks
    assert value,message;checks+=1

def books(typ,*,name='Special Teams',extra_count=2,collision=False):
    n(library+0x344,typ);group_id=12 if typ==1 else 20
    forms={typ:[(103,(103,0,typ)),(group_id,(group_id,1,typ)),(110,(110,2,typ))],
           typ+1:[(803+i*11,(803+i*11,i,typ+1)) for i in range(extra_count)]}
    if collision:forms[typ+1][0]=(103,(103,0,typ+1))
    def meta(leaf,value):
        metadata(leaf,value)
        if value[0]==group_id and value[2]==typ:
            data=name.encode();assert len(data)<=15
            put(leaf+0x28+0x2f8,data+b'\0'*(15-len(data))+bytes([15-len(data)]))
    tree(core+0xd0,forms.items(),lambda node,v:nested(node,v,meta))
    tree(core+0x130,[(t,[(v[1][1],v[0]) for v in fs]) for t,fs in forms.items()],lambda nd,v:nested(nd,v,lambda leaf,fid:n(leaf+0x24,fid)))
    sets={typ:[(i,[(j,10000+typ*100+i*10+j) for j in range(2)]) for i in range(3)],
          typ+1:[(i,[(j,10000+(typ+1)*100+i*10+j) for j in range(3-i%2)]) for i in range(extra_count)]}
    tree(core+0x310,sets.items(),lambda nd,v:nested(nd,v,lambda nd2,v2:nested(nd2,v2,lambda leaf,sid:n(leaf+0x28+0x220,sid))))
    tree(core+0x10,[(t,[(i,[(j,[]) for j,sid in ss]) for i,ss in fs]) for t,fs in sets.items()],lambda nd,v:nested(nd,v,lambda nd2,v2:nested(nd2,v2,lambda leaf,pl:tree(leaf+0x28,[(i,i) for i in range(3)],lambda nd3,x:n(nd3+0x24,x)))))
    return group_id,sets

group_find=C.WINFUNCTYPE(C.c_uint64,C.c_uint64,C.c_int)(R+0x6100)
resolver=C.WINFUNCTYPE(C.c_uint64,C.c_uint64,C.c_int,C.c_int,C.c_int,C.c_uint64)(R+0x6500)
scratch=alloc(16)
for typ in [1,11]:
    group_id,sets=books(typ)
    source=group_find(core,typ);check(bool(source),'verified current typed Special Teams group')
    snapshot=C.string_at(source,0x328)
    for initial in [True,False]:
        check(call(0,library,0,typ,0,initial=initial)==3,'one existing ST filter; no appended formation filters')
        for index in range(3):
            call(1,library,0,output,index,initial=initial)
            check(iv(output+0x310)==[103,group_id,110][index],'original formation identity retained')
            check(iv(output+0x314)==(0x20000001 if index==1 else index),'only ST transient ordinal marked')
            check(iv(output+0x318)==typ,'native side type retained')
        check(call(2,core,0,typ,group_id,arg5=output)&255==1,'group lookup accepted')
        check(iv(output+0x310)==group_id and iv(output+0x314)==0x20000001,'group lookup keeps native ID, tags only UI ordinal')
        for enumeration in [False,True]:
            check(call(3,library,0,0x20000001,0,enumeration=enumeration)==7,'native ST + both special formations concatenated')
            expected=[10000+typ*100+10+j for j in range(2)]+[10000+(typ+1)*100+j for j in range(3)]+[10000+(typ+1)*100+10+j for j in range(2)]
            for index,wanted in enumerate(expected):
                ptr=call(4,library,0,0x20000001,index,enumeration=enumeration)
                check(bool(ptr) and iv(ptr+0x220)==wanted,'real native set pointer/ID, ordinary then special sets')
                check(call(5,library,0,0x20000001,index,enumeration=enumeration)==3,'native play-count eligibility for each mapped set')
    check(C.string_at(source,0x328)==snapshot,'source group metadata/index never modified')
    for index,expected in [(-1,(typ,1,-1,7)),(0,(typ,1,0,2)),(2,(typ+1,0,0,7)),(6,(typ+1,1,1,7))]:
        check(resolver(library,typ,1,index,scratch)==1,'bounded map success')
        got=tuple(C.c_int.from_address(scratch+i*4).value for i in range(4))
        check(got==expected,'resolved source type, form, local set and count')
    for index in [7,8,100,-2]:check(resolver(library,typ,1,index,scratch)==0,'out-of-range merged set rejected')
    check(resolver(library,typ,0,0,scratch)==0,'ordinary group cannot masquerade as ST')
    check(call(3,library,0,0,0)==2,'ordinary group count untouched')
    check(call(0,library,0,typ,0,unrelated=True)==3,'other menu owner never gets grouping/extension')
    # Preserve the separate-entry fallback when canonical native metadata is
    # absent/unsupported; never rename native records or discard formations.
    books(typ,name='Unknown Group')
    check(group_find(core,typ)==0,'unknown group fails closed')
    check(call(0,library,0,typ,0)==5,'fallback keeps the prior working separate entries')
    books(typ,extra_count=0)
    check(call(0,library,0,typ,0)==3,'no extra formations creates no extra filter')
    check(call(3,library,0,0x20000001,0)==2,'empty extra book retains original ST sets')
    books(typ,collision=True)
    check(call(0,library,0,typ,0)==3,'ambiguous extra IDs remain excluded')
    check(call(3,library,0,0x20000001,0)==2,'invalid merged inputs fall back to the original untagged group')
    books(typ)
    check(call(3,library,0,0x20000001,0)==7,'new book rebuild recovers without cached identities')

timings={}
for name,fn,args in [('ordinary',C.WINFUNCTYPE(C.c_uint64,C.c_uint64,C.c_int,C.c_int,C.c_int)(B+0x34776b0),(core,0,1,11)),('grouped',resolver,(library,11,1,-1,scratch))]:
    rows=[]
    for _ in range(5):
        t=time.perf_counter_ns()
        for i in range(10000):fn(*args)
        rows.append((time.perf_counter_ns()-t)/10000/1000)
    timings[name]=dict(medianUs=statistics.median(rows),slowUs=max(rows))
report=dict(passed=checks,companionPassed=base_passed,gameAccess=False,nativeMetadataChanged=False,source='released machine code with fake books and native helpers; rendering/save/gameplay not executed',callbackUs=timings,pending='live Special Teams filter grouping and grouped assignments')
a.output.with_name(a.output.stem+'-groups.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
