"""Execute display aliases against isolated native-shaped books. No live access."""
import ctypes as C,runpy,struct,json
from pathlib import Path
ctx=runpy.run_path(str(Path(__file__).with_name('test_formation_subs.py')))
globals().update({k:v for k,v in ctx.items() if not k.startswith('__')})
base_passed=checks;checks=0
def check(value,message):
 global checks
 assert value,message;checks+=1
for name,off in [('FormationSetName',0x7000),('TypedSetMembership',0x7400),('PreviewTreeFind',0x5b00),('KickoffName',0x7800),('ReturnName',0x7820)]:put(R+off,(a.exports/(name+'.bin')).read_bytes())
q(library,B+0xb3324d0)
def nodes(root):
 todo=[root];seen=set()
 while todo:
  at=todo.pop()
  if not at:continue
  assert at not in seen and len(seen)<1024;seen.add(at);yield at;todo.extend([qv(at),qv(at+8)])
def entries():
 for tn in nodes(qv(core+0x320)):
  for fn in nodes(qv(tn+0x38)):
   for sn in nodes(qv(fn+0x38)):
    obj=sn+0x28;q(obj,B+0xb2e82e0);put(obj+0x208,b'no name\0'+b'\0'*7+b'\x08')
    yield iv(tn+0x20),obj
member=C.WINFUNCTYPE(C.c_uint64,C.c_uint64,C.c_uint64,C.c_int)(R+0x7400)
def invoke(obj,wrong=False,outerwrong=False):
 # Reproduce the exact set-list call and enclosing owner stack slots.
 frame=0x398;ret=B+0x3760895+(0x2000 if wrong else 0)
 code=hx('4881ec')+struct.pack('<I',frame)+hx('48b8')+struct.pack('<Q',R+0x7000)+hx('ffd0')
 at=ret-len(code);put(at,code+hx('4881c4')+struct.pack('<I',frame)+hx('c3'))
 outerret=B+0x7fc9d92+(0x2000 if outerwrong else 0);outer=outerret-16
 put(outer,hx('4883ec2848b8')+struct.pack('<Q',at)+hx('ffd04883c428c3'))
 return C.string_at(C.WINFUNCTYPE(C.c_uint64,C.c_uint64)(outer)(obj)).decode('ascii')
for side in [1,11]:
 make_books(side,3);rows=list(entries())
 for typ,obj in rows:
  original=C.string_at(obj,0x328)
  for requested in [2,12]:check(member(core,obj,requested)==(typ==requested),'exact pointer in required typed book')
  wanted='Kickoff' if typ==2 else 'Kickoff Return' if typ==12 else 'no name'
  check(invoke(obj)==wanted,'UI-copy alias only for kickoff/return')
  check(invoke(obj,wrong=True)=='no name','other name callers stay native')
  check(invoke(obj,outerwrong=True)=='no name','unrelated set builder owner stays native')
  check(C.string_at(obj,0x328)==original,'canonical set name, ID, metadata unchanged')
  if typ in [2,12]:
   put(obj+0x208,b'Return Middle\0'+b'\0'+b'\x02')
   check(invoke(obj)=='Return Middle','meaningful native names remain intact')
   put(obj,original)
 extra=next(obj for typ,obj in rows if typ==side+1)
 for location,value in [(B+0xf55c998,0),(B+0xd9eb808,8),(B+0xf3fddbc,4)]:
  before=iv(location);n(location,value);check(invoke(extra)=='no name','scope failure keeps native name');n(location,before)
 stale=alloc(0x328);q(stale,B+0xb2e82e0);n(stale+0x220,iv(extra+0x220));put(stale+0x208,b'no name\0'+b'\0'*7+b'\x08')
 check(invoke(stale)=='no name','matching ID without current pointer ownership rejected')
 check(member(core,extra,99)==0,'unsupported type rejected without alias')
report=dict(passed=checks,companionPassed=base_passed,gameAccess=False,nativeMetadataChanged=False,pending='Live list and title alias on kickoff and return')
a.output.with_name(a.output.stem+'-names.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report))
