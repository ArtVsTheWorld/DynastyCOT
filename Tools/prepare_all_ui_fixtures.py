"""Prepare private native UI fixtures for every reviewed adapter method."""
from pathlib import Path
import sys,json,struct,hashlib
R=Path(__file__).resolve().parents[1];sys.path[:0]=[str(R/'Modules/DynastyEnhancer/Tools'),'C:/Apps/CFB 27/Working/Teambuilder Import Plugin/Tools']
from inspect_tb_ast import load
from apt_ui_symbols import symbols

def main():
 folder=R/'Build/UiDiscovery/UniversalAdapters/Fixtures';folder.mkdir(exist_ok=True);report=json.loads((folder.parent/'source-audit.json').read_text());result=[];cached={}
 for bank in report['banks']:
  if bank['bank']=='notes':continue
  rows=[]
  for row in bank['methods']:
   c=next(c for c in row['candidates'] if 'contract'in c);file=c['asset']
   if file not in cached:
    apt=load(file);label=hashlib.sha256(apt.data).hexdigest()+'.bin';(folder/label).write_bytes(apt.data);cached[file]=(apt,label)
   apt,label=cached[file];m=row['method'];off=m['offset'];pool=symbols(apt)[off][0]
   rows.append(dict(index=row['index'],file=label,offset=off,poolHex=[x.encode('latin1').hex() for x in pool],pointers=m['pointers'],nested=m['nested']))
  result.append(dict(bank=bank['bank'],methods=rows))
 (folder/'methods.json').write_text(json.dumps(result,separators=(',',':')));print(f'Prepared {sum(len(b["methods"]) for b in result)} non-Notes methods from {len(cached)} reviewed assets.')
if __name__=='__main__':main()
