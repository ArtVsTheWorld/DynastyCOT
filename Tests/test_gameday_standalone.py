"""Verify production managed DLL, without watching/injecting any live process."""
from pathlib import Path
import datetime,hashlib,json,os,shutil,subprocess,zipfile
ROOT=Path(__file__).resolve().parents[1]
META=json.loads((ROOT/'Build/current-gameday-standalone.json').read_text(encoding='utf-8'))
BASE=ROOT/'Build'/('GamedayStandaloneChecks-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'))
BASE.mkdir();checks=0;log=[]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def check(ok,message):
 global checks
 assert ok,message
 checks+=1
def run(command,**kw):
 result=subprocess.run([str(x) for x in command],cwd=BASE,capture_output=True,text=True,**kw)
 log.append(result.stdout+result.stderr)
 if result.returncode:raise AssertionError('\n'.join(log))
 return result.stdout
def seed(name):
 root=BASE/name;(root/'Plugins').mkdir(parents=True);(root/'.cot-fixture').write_text('isolated',encoding='utf-8')
 (root/'MMCModManager.exe').write_bytes(b'Isolated COT marker, never executed')
 shutil.copyfile(META['dll'],root/'Plugins/GamedayAtmosphere.dll');return root
def fixture(case,root):
 global checks
 output=run([EXE,case,root,BASE,META['sdk']]);checks+=int(output.strip().split()[-1])
compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe');EXE=BASE/'GamedayStandaloneFixture.exe'
run([compiler,'/nologo','/platform:x64','/out:'+str(EXE),ROOT/'Tests/GamedayStandaloneFixture.cs'])
check(sha(Path(META['dll']))==META['dllSha256'],'Exact source-built managed DLL')
for relative,digest in META['sourceHashes'].items():check(sha(ROOT/relative)==digest,'Source changed since build: '+relative)
reference=ROOT/'Release/DynastyCOT-1.0.2.zip'
check(sha(reference)==META['referenceArchiveSha256'],'Reference bundle unchanged')
with zipfile.ZipFile(reference) as archive:
 manifest=json.loads(archive.read('package-manifest.json'));embedded={x['path']:x['sha256'] for x in manifest['embedded']}
 for item in META['resources']:check(embedded['resource:'+item['name']]==item['sha256'],'Resource equals released engine/defaults/tool')
 # Confirm published COT really uses this same named guard; no execution needed.
 bundle=archive.read('Plugins/DynastyCOT.dll')
 check('Local\\Ace.DynamicGamedayAtmosphere.MMC.'.encode('utf-16le') in bundle,'Published COT shared OS guard')
 package=BASE/'COT102';archive.extractall(package)
for case in ['files','cot-first','standalone-first']:fixture(case,seed(case))
# Actual released installer install/uninstall with shared data and independent DLL.
root=seed('cot-roundtrip');data=root/'Plugins/DynamicGamedayAtmosphere';(data/'History').mkdir(parents=True)
preserved={data/'DynamicGamedayAtmosphere.ini':b'[General]\r\nConfigVersion=2\r\nKeepUserChoices=1\r\n'}
preserved.update({data/'History'/name:('existing '+name).encode() for name in ['attendance-history.txt','audience-foundations.txt','visitor-plans.txt']})
for path,contents in preserved.items():path.write_bytes(contents)
preserved[root/'Plugins/GamedayAtmosphere.dll']=Path(META['dll']).read_bytes()
environment={k:v for k,v in os.environ.items() if k.lower()!='psmodulepath'}
def install(action):
 run(['powershell.exe','-NoProfile','-ExecutionPolicy','Bypass','-File',package/'Install.ps1','-MmcRoot',root,'-PackageRoot',package,'-Fixture','-FixtureRoot',BASE,'-Action',action],env=environment)
 for path,contents in preserved.items():check(path.exists() and path.read_bytes()==contents,'COT '+action+' preserved '+str(path.relative_to(root)))
install('Install');fixture('extract',root)
native=root/'DGA-Runtime/Runtime-2.1.9/GamedayAtmosphereNative-2.1.9.dll'
check(sha(native)==META['resources'][0]['sha256'],'Released COT native cache matches standalone')
install('UninstallAll');check(not native.exists(),'Bundle removed its owned cache')
fixture('extract',root);check(sha(native)==META['resources'][0]['sha256'],'Standalone rematerializes native after bundle uninstall')
for path,contents in preserved.items():check(path.read_bytes()==contents,'Rematerialization preserved '+str(path.relative_to(root)))
# Existing storage/settings/direct-loader unit tests run without touching game.
source=ROOT/'Modules/GamedayAtmosphere/Source';tests=source.parent/'Tests'
regression=BASE/'Existing/Build/Checks';regression.mkdir(parents=True)
regression_source=BASE/'Existing/Source';regression_source.mkdir()
shutil.copyfile(source/'DynamicGamedayAtmosphere.ini',regression_source/'DynamicGamedayAtmosphere.ini')
for name,files in {
 'SettingsTests':['PluginSettings.cs'],
 'PluginStorageTests':['PluginSettings.cs','PluginStorage.cs'],
 'RuntimeStorageTests':['PluginSettings.cs','PluginStorage.cs'],
 'ManagedRuntimeLoaderTests':['ManagedRuntimeLoader.cs','LaunchDiagnostics.cs'],
}.items():
 exe=regression/(name+'.exe');run([compiler,'/nologo','/platform:x64','/out:'+str(exe),tests/(name+'.cs'),*(source/name for name in files)])
 run([exe]);check(True,name+' passed')
report=dict(format=1,module=META['module'],version=META['version'],checks=checks,dllSha256=META['dllSha256'],sourceHashes=META['sourceHashes'],referenceArchiveSha256=META['referenceArchiveSha256'],fixtureRoot=str(BASE),cases=['resource parity','MMC branding','fresh/current/legacy/future settings preservation','existing history preservation','cached resource integrity and repair','native runtime outside Plugins','legacy DLL detection','both bundled/standalone guard orders','duplicate assembly guard','released COT install/uninstall/cache recovery','settings/storage/direct loader regression suites'],liveGameAcceptance='pending',installedIntoLiveMmc=False,published=False)
(BASE/'results.log').write_text('\n'.join(log),encoding='utf-8')
(ROOT/'Build/gameday-standalone-verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('PASS',checks,'standalone checks including 4 existing regression suites; no live install/game calls')
