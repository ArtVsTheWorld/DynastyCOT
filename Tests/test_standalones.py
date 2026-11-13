"""Integration of all standalone payloads in fixtures; no watcher/game callbacks."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,zipfile
ROOT=Path(__file__).resolve().parents[1];NAMES=['BeyondTheField','ExtraSettings','ExpandedPractice']
parser=argparse.ArgumentParser();parser.add_argument('--module',action='append',choices=NAMES);args=parser.parse_args();selected=args.module or NAMES
META={name:json.loads((ROOT/f'Build/current-standalone-{name}.json').read_text(encoding='utf-8')) for name in NAMES}
GAMEDAY=json.loads((ROOT/'Build/current-gameday-standalone.json').read_text(encoding='utf-8'))
BASE=ROOT/'Build'/('StandaloneChecks-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));BASE.mkdir();checks=0;logs=[]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def check(ok,message):
 global checks
 assert ok,message;checks+=1
def run(args,**kw):
 result=subprocess.run([str(x) for x in args],capture_output=True,text=True,cwd=BASE,**kw);logs.append(result.stdout+result.stderr)
 if result.returncode:raise AssertionError(logs[-1])
 return result.stdout
def seed(name):
 root=BASE/name;(root/'Plugins').mkdir(parents=True);(root/'.cot-fixture').write_text('isolated');(root/'MMCModManager.exe').write_bytes(b'fixture; never executed')
 for module in NAMES:shutil.copyfile(META[module]['dll'],root/f'Plugins/{module}.dll')
 shutil.copyfile(GAMEDAY['dll'],root/'Plugins/GamedayAtmosphere.dll')
 for rel in GAMEDAY.get('supportFiles',{}):
  target=root/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(Path(GAMEDAY['stage'])/rel,target)
 return root
def fixture(mode,root):
 global checks
 output=run([EXE,mode,root,BASE,META[NAMES[0]]['sdk']]);checks+=int(output.split()[1])
compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe');EXE=BASE/'StandaloneModulesFixture.exe'
run([compiler,'/nologo','/platform:x64','/reference:System.Web.Extensions.dll','/out:'+str(EXE),ROOT/'Tests/StandaloneModulesFixture.cs'])
for name,meta in META.items():
 check(sha(Path(meta['dll']))==meta['dllSha256'],'DLL build identity')
 if name in selected:
  for relative,digest in meta['sourceHashes'].items():check(sha(ROOT/relative)==digest,'Source unchanged: '+relative)
 if name=='BeyondTheField':
  original=(ROOT/'Modules/DynastyEnhancer/Source/RuntimeFeed.h').read_bytes()
  branded=Path(meta['work'])/'Native/Modules/DynastyEnhancer/Source/RuntimeFeed.h'
  check(branded.read_bytes()==original.replace(b'folder/L"dynasty-enhancer.json"',b'folder/L"beyond-the-field.json"'),'Only native source branding delta is the configuration filename')
  check('beyond-the-field.json'.encode('utf-16le') in (Path(meta['work'])/'BeyondTheFieldNative-v103.dll').read_bytes(),'Compiled native uses the new configuration filename')
reference=ROOT/'Release/DynastyCOT-1.0.2.zip'
with zipfile.ZipFile(reference) as z:
 old_manifest=json.loads(z.read('package-manifest.json'));old={x['path']:x['sha256'] for x in old_manifest['files']}
 for name,meta in META.items():
  for path,digest in meta['payload'].items():
   if path.endswith('/beyond-the-field.json'):check(digest==old[path.replace('beyond-the-field.json','dynasty-enhancer.json')],'Only config filename changes')
   elif name=='BeyondTheField' and path.endswith('/BeyondTheFieldNative-v103.dll'):check(meta['branding']['nativeTests']=='passed','Branded native configuration source build and tests')
   elif name=='BeyondTheField' and path.endswith('/BeyondTheFieldRecruitFog-v1.dll'):
    inputs=json.loads((ROOT/'cot.build-inputs.json').read_text());check(digest==meta['branding']['fogSha256'] and meta['branding']['fogSourceBuilt'],'Matched freshly source-built fog payload')
   elif name=='BeyondTheField' and path.startswith(('COT-Runtime/DynastyEnhancer/Tools/','COT-Runtime/DynastyEnhancer/Content/Runtime/')):
    relative=path.removeprefix('COT-Runtime/DynastyEnhancer/');check(sha(ROOT/'Modules/DynastyEnhancer'/relative)==digest,'Current source-built GPI generator/content payload')
   elif name=='BeyondTheField' and path=='LICENSES/PlayerAnalytics-NOTICES.txt':check(sha(ROOT/'Modules/DynastyEnhancer/Source/Analytics.NOTICES.txt')==digest,'Current analytics notices')
   elif path.endswith(('ExtraSettings.Persistence.exe','ExpandedPractice.Runtime.exe')):check(digest==sha(Path(meta['work'])/Path(path).name),'Fresh source-built helper payload')
   else:check(old[path]==digest,'Payload parity with released COT: '+path)
 package=BASE/'COT102';z.extractall(package)
for mode in ['claim','conflict']:fixture(mode,seed(mode))
root=seed('all-modules');data={
 'COT-Runtime/DynastyEnhancer/RuntimeHistory/42/history.json':b'published words stay unchanged',
 'COT-Runtime/DynastyEnhancer/RecruitResumeHistory/42.json':b'existing persistent recruit data',
 'COT-Runtime/DynastyEnhancer/DynastyEnhancementSettings/42.json':b'existing fog choices',
 'Plugins/OfflineDynastySettings/DynastyClocks.json':b'existing per-Dynasty clocks',
 'Plugins/SituationalPractice/custom-session.txt':b'keep practice data',
 'Plugins/DynamicGamedayAtmosphere/History/visitor-plans.txt':b'keep gameday plans',
}
for relative,contents in data.items():p=root/relative;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(contents)
fixture('files',root)
for relative,contents in data.items():check((root/relative).read_bytes()==contents,'Existing data intact: '+relative)
# Real published COT installer: detect any reverse-switch incompatibility.
environment={k:v for k,v in os.environ.items() if k.lower()!='psmodulepath'}
def installer(action):
 result=subprocess.run(['powershell.exe','-NoProfile','-ExecutionPolicy','Bypass','-File',str(package/'Install.ps1'),'-MmcRoot',str(root),'-PackageRoot',str(package),'-Fixture','-FixtureRoot',str(BASE),'-Action',action],capture_output=True,text=True,env=environment);logs.append(result.stdout+result.stderr);return result
first=installer('Install');switch_issue=first.returncode!=0
if switch_issue:
 check('BeyondTheFieldNative' in first.stdout+first.stderr,'Expected source-branded runtime switch conflict')
 native=root/'COT-Runtime/DynastyEnhancer/BeyondTheFieldNative-v103.dll';shutil.move(native,BASE/'saved-standalone-native.dll')
 result=installer('Install');check(result.returncode==0,'Safe DLL-only backup resolves COT switch: '+result.stdout+result.stderr)
else:check(True,'Direct COT reverse switch supported')
for relative,contents in data.items():check((root/relative).read_bytes()==contents,'COT install preserves data')
result=installer('UninstallAll');check(result.returncode==0,'COT uninstall succeeds')
fixture('files',root)
for relative,contents in data.items():check((root/relative).read_bytes()==contents,'Rematerialization preserves data')
for name in NAMES:check((root/f'Plugins/{name}.dll').exists(),'Bundle uninstall leaves independent DLL')
report=dict(format=1,checks=checks,gamedayDllSha256=GAMEDAY['dllSha256'],modules={name:dict(version=meta['version'],dllSha256=meta['dllSha256'],sourceHashes=meta['sourceHashes'],payload=meta['payload'],branding=meta['branding']) for name,meta in META.items() if name in selected},fixtureRoot=str(BASE),reverseCotSwitchRequiresNativeBackup=switch_issue,liveAcceptance='Storage change has offline verification only; clean-package live game acceptance pending',installedIntoLiveMmc=False,published=False)
(BASE/'results.log').write_text('\n'.join(logs),encoding='utf-8');(ROOT/'Build/standalones-verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('PASS',checks,'standalone integration checks; reverse-switch native backup required:',switch_issue)
