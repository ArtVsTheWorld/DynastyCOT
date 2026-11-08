"""Record explicitly named COT edits to imported sources; never update originals."""
from pathlib import Path
import argparse,hashlib,json
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--reason',required=True);p.add_argument('paths',nargs='+');a=p.parse_args()
 owned={e['path'] for e in json.loads((ROOT/'Baselines/source-files.json').read_text())['files']}
 path=ROOT/'Baselines/changes.json';doc=json.loads(path.read_text());entries={e['path']:e for e in doc['changes']}
 for rel in a.paths:
  rel=rel.replace('\\','/');target=(ROOT/rel).resolve()
  if not target.is_relative_to(ROOT) or not target.is_file():raise ValueError('Not a workspace source: '+rel)
  if rel in owned:entries[rel]=dict(path=rel,sha256=hashlib.sha256(target.read_bytes()).hexdigest(),reason=a.reason)
 doc['changes']=sorted(entries.values(),key=lambda e:e['path']);path.write_text(json.dumps(doc,indent=2)+'\n')
if __name__=='__main__':main()
