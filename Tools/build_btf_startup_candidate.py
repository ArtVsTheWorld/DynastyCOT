"""Source-build a private startup fix. Never install, inject, publish or edit saves."""
from pathlib import Path
import sys,json,hashlib,zipfile,subprocess
import build_standalones,build_final_readiness
R=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 values=list(build_standalones.DEFS['BeyondTheField']);values[2]='1.0.25';build_standalones.DEFS['BeyondTheField']=tuple(values)
 sys.argv=[sys.argv[0],'--module','BeyondTheField'];build_final_readiness.main()
 m=json.loads((R/'Build/current-standalone-BeyondTheField.json').read_text());assert m['version']=='1.0.25'
 for p,h in m['sourceHashes'].items():assert sha(R/p)==h,p
 out=R/'Release/BtfStartup-Private';out.mkdir(exist_ok=True)
 readme=out/'README.txt';readme.write_text('''Ace's Beyond the Field 1.0.25-startup1 — PRIVATE TEST

Fixes a documented temporary Windows module-list error during startup by retrying only the read, at most eight times over 700 ms. Native DLL loading itself is not repeated. Detailed failures now name the runtime, operation and Windows error.

Close the game and MMC before replacing BeyondTheField.dll in your usual Plugins folder. Reopen MMC/game, then check the native fog and Social Media status. Keep a backup of the prior DLL outside Plugins for rollback. Existing settings, accounts and published history are retained.

The included feature list and guide describe the unchanged 1.0.24 feature set. This private build changes managed startup only. It has not been installed or injected into the current game. Other users' missing-path failures remain undiagnosed until their complete paths or package.log are available.
''',encoding='utf8')
 files={'BeyondTheField.dll':Path(m['dll']),'README.txt':readme,'FEATURES.md':R/'Packaging/Standalone/BeyondTheField/FEATURES.md','BeyondTheField-Player-Guide.pdf':R/'docs/player-guides/BeyondTheField-Player-Guide.pdf'}
 archive=out/'BeyondTheField-1.0.25-startup1.zip'
 with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for leaf,p in files.items():z.write(p,leaf)
 with zipfile.ZipFile(archive) as z:assert z.testzip() is None and len(z.namelist())==4
 proof=dict(name='BeyondTheField',version='1.0.25',candidate='startup1',sourceBase=subprocess.check_output(['git','rev-parse','HEAD'],cwd=R,text=True).strip(),uncommittedFix=True,archiveSha256=sha(archive),dllSha256=sha(Path(m['dll'])),payload=m['payload'],sourceHashes=m['sourceHashes'],files={leaf:sha(p) for leaf,p in files.items()},verification=json.loads((R/'Build/btf-startup-verification.json').read_text()),installed=False,injected=False,published=False)
 (out/'BeyondTheField-1.0.25-startup1-verification.json').write_text(json.dumps(proof,indent=2)+'\n',encoding='utf8')
 (out/'SHA256.txt').write_text(sha(archive)+'  '+archive.name+'\n',encoding='ascii')
 print('PRIVATE CANDIDATE:',archive,flush=True)
if __name__=='__main__':main()
