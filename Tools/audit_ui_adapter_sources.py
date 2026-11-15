"""Map every existing adapter to reviewed class/method declarations, offline."""
from pathlib import Path
import sys,json,struct,hashlib,time
R=Path(__file__).resolve().parents[1];sys.path[:0]=[str(R/'Modules/DynastyEnhancer/Tools'),'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools']
from inspect_tb_ast import load
from apt_ui_symbols import symbols
from build_recruit_notes_ui import fnv
from build_ui_method_contracts import contract

def main():
 folder=R/'Build/UiDiscovery/UniversalAdapters';banks=json.loads((folder/'programs.json').read_text());wanted={}
 for bank in banks:
  for i,m in enumerate(bank['methods']):wanted.setdefault((m['offset'],m['argc'],m['registers'],m['flags'],m['size'],m['hash']),[]).append((bank['bank'],i))
 matched={};dedupe=set();errors=[];started=time.perf_counter();processed=0
 for file in json.loads((folder/'assets.json').read_text()):
  try:
   apt=load(file);digest=hashlib.sha256(apt.data+apt.const_data).hexdigest()
   if digest in dedupe:continue
   dedupe.add(digest);possible=[]
   for key in wanted:
    off,argc,regs,flags,n,h=key
    if off+48+n>len(apt.data):continue
    if struct.unpack_from('<IHH',apt.data,off+8)!=(argc,regs,flags) or struct.unpack_from('<I',apt.data,off+24)[0]!=n:continue
    if fnv(apt.data[off+48:off+48+n])==h:possible.append(key)
   if not possible:continue
   index=symbols(apt);processed+=1
   for key in possible:
    off=key[0]
    if off not in index:
     matched.setdefault(key,[]).append(dict(asset=file,symbolMissing=True));continue
    c=contract(apt,off);matched.setdefault(key,[]).append(dict(asset=file,contract=c))
  except StopIteration:continue
  except (AssertionError,ValueError,KeyError,IndexError,struct.error) as e:errors.append(dict(asset=file,error=str(e)[:200]))
 report=[]
 for bank in banks:
  rows=[]
  for i,m in enumerate(bank['methods']):
   key=(m['offset'],m['argc'],m['registers'],m['flags'],m['size'],m['hash']);candidates=matched.get(key,[]);contracts={json.dumps(c['contract'],sort_keys=True) for c in candidates if 'contract' in c}
   rows.append(dict(index=i,method=m,candidates=candidates,distinctContracts=len(contracts)))
  report.append(dict(bank=bank['bank'],methods=rows))
 doc=dict(seconds=time.perf_counter()-started,uniqueAssets=len(dedupe),matchedAssets=processed,banks=report,errors=errors)
 (folder/'source-audit.json').write_text(json.dumps(doc,indent=2))
 unresolved=[dict(bank=b['bank'],index=m['index'],offset=m['method']['offset'],matches=len(m['candidates']),contracts=m['distinctContracts']) for b in report for m in b['methods'] if m['distinctContracts']!=1]
 print(json.dumps(dict(seconds=round(doc['seconds'],2),assets=len(dedupe),methods=sum(len(b['methods']) for b in banks),unresolved=unresolved,errors=len(errors)),indent=2))
if __name__=='__main__':main()
