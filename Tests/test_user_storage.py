"""Per-user storage/regression checks. Fixtures only; never inject or launch game."""
from pathlib import Path
import datetime,hashlib,json,re,shutil,subprocess
ROOT=Path(__file__).resolve().parents[1]
WORK=ROOT/'Build'/('US-'+datetime.datetime.now(datetime.timezone.utc).strftime('%m%d%H%M%S'))
WORK.mkdir();logs=[];results={}
CSC=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe')
def run(args,cwd=WORK):
 p=subprocess.run([str(x) for x in args],cwd=cwd,capture_output=True,text=True,encoding='utf-8',errors='replace');logs.append(p.stdout+p.stderr);(WORK/'results.log').write_text('\n'.join(logs),encoding='utf-8')
 if p.returncode:raise AssertionError(logs[-1])
 print(p.stdout.strip()[-700:],flush=True);return p.stdout
def managed(name,sources,args=(),refs=()):
 exe=WORK/(name+'.exe');run([CSC,'/nologo','/platform:x64','/warnaserror+','/out:'+str(exe),*(('/reference:'+x) for x in refs),*sources]);results[name]=run([exe,*args]);return exe
common=ROOT/'Core/UserStorage.cs'
fixture=WORK/'Migration';fixture.mkdir();(fixture/'.cot-fixture').write_text('isolated')
managed('UserStorageChecks',[common,ROOT/'Tests/UserStorageChecks.cs'],[WORK,fixture])
extra=ROOT/'Modules/ExtraDynastySettings/Plugin'
managed('DynastyClockProfileChecks',[common,extra/'ClockPreferences.cs',extra/'DynastyClockProfiles.cs',ROOT/'Tests/DynastyClockProfileChecks.cs'],[WORK/'Clocks'],['System.Web.Extensions.dll'])
meta={name:json.loads((ROOT/'Build'/('current-standalone-'+name+'.json')).read_text()) for name in ['BeyondTheField','ExtraSettings','ExpandedPractice']}
gda=json.loads((ROOT/'Build/current-gameday-standalone.json').read_text());meta['GamedayAtmosphere']=gda
for name,m in meta.items():
 assert hashlib.sha256(Path(m['dll']).read_bytes()).hexdigest()==m['dllSha256']
 for rel,digest in m['sourceHashes'].items():assert hashlib.sha256((ROOT/rel).read_bytes()).hexdigest()==digest,'Rebuild changed source: '+rel
managed('ClockHelperChecks',[ROOT/'Tests/ClockHelperChecks.cs'],[Path(meta['ExtraSettings']['work'])/'ExtraSettings.Persistence.exe',WORK])
# Production assemblies: file preparation, helper integrity leases and guards only.
exe=WORK/'StandaloneModulesFixture.exe';run([CSC,'/nologo','/platform:x64','/reference:System.Web.Extensions.dll','/out:'+str(exe),ROOT/'Tests/StandaloneModulesFixture.cs'])
mmc=WORK/'Program Files MMC';(mmc/'Plugins').mkdir(parents=True);(mmc/'.cot-fixture').write_text('isolated');(mmc/'MMCModManager.exe').write_text('marker never run')
for name,m in meta.items():shutil.copyfile(m['dll'],mmc/'Plugins'/(name+'.dll'))
for rel in gda.get('supportFiles',{}):
 p=mmc/rel;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(Path(gda['stage'])/rel,p)
data={
 'COT-Runtime/DynastyEnhancer/RuntimeHistory/42/published.json':'original published words and account IDs',
 'COT-Runtime/DynastyEnhancer/DynastyEnhancementSettings/42.json':'original fog and GPI choices',
 'Plugins/OfflineDynastySettings/DynastyClocks.json':'original per-dynasty clock file',
 'Plugins/DynamicGamedayAtmosphere/History/visitor-plans-v3.txt':'original visitor plans',
 'Plugins/SituationalPractice/custom-session.txt':'original practice data',
}
for rel,value in data.items():p=mmc/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(value)
before={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in mmc.rglob('*') if p.is_file()}
# Native Windows DACL denies writes/deletes even for the test's current user.
script=WORK/'DeniedProduction.ps1';script.write_text('''param($Root,$Exe,$Parent,$Sdk)
$ErrorActionPreference='Stop'
$acl=[IO.Directory]::GetAccessControl($Root,[Security.AccessControl.AccessControlSections]::Access)
$denied=New-Object Security.AccessControl.DirectorySecurity
$denied.SetSecurityDescriptorBinaryForm($acl.GetSecurityDescriptorBinaryForm(),[Security.AccessControl.AccessControlSections]::Access)
$rights=[Security.AccessControl.FileSystemRights]::Write -bor [Security.AccessControl.FileSystemRights]::Delete -bor [Security.AccessControl.FileSystemRights]::DeleteSubdirectoriesAndFiles
$rule=New-Object Security.AccessControl.FileSystemAccessRule([Security.Principal.WindowsIdentity]::GetCurrent().User,$rights,([Security.AccessControl.InheritanceFlags]::ContainerInherit -bor [Security.AccessControl.InheritanceFlags]::ObjectInherit),[Security.AccessControl.PropagationFlags]::None,[Security.AccessControl.AccessControlType]::Deny)
$denied.AddAccessRule($rule)
[IO.Directory]::SetAccessControl($Root,$denied)
try {
 $failed=$false
 try{[IO.File]::WriteAllText((Join-Path $Root 'write-probe'),'must fail')}catch [UnauthorizedAccessException] {$failed=$true}
 if(!$failed){throw 'Fixture does not actually deny writes'}
 & $Exe files $Root $Parent $Sdk
 if($LASTEXITCODE -ne 0){throw 'Production preparation failed with installation read-only'}
} finally {[IO.Directory]::SetAccessControl($Root,$acl)}
''',encoding='utf-8')
results['ProductionWriteDenied']=run(['powershell.exe','-NoProfile','-ExecutionPolicy','Bypass','-File',script,mmc,exe,WORK,meta['BeyondTheField']['sdk']])
assert before=={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in mmc.rglob('*') if p.is_file()},'Runtime changed protected installation files'
for name,m in meta.items():
 if name=='GamedayAtmosphere':continue
 assembly_profile=mmc.with_name(mmc.name+'-Profile')/'Ace/DynastyCOT'
assert (assembly_profile/'BeyondTheField/RuntimeHistory/42/published.json').read_text()==data['COT-Runtime/DynastyEnhancer/RuntimeHistory/42/published.json']
# Check actual helper persistence writes both clock and playcall snapshots to user data.
source=ROOT/'Modules/GamedayAtmosphere/Source';tests=source.parent/'Tests'
reg=WORK/'Regression/Build/Checks';reg.mkdir(parents=True);(WORK/'Regression/Source').mkdir();shutil.copyfile(source/'DynamicGamedayAtmosphere.ini',WORK/'Regression/Source/DynamicGamedayAtmosphere.ini')
for name,files in {'SettingsTests':['PluginSettings.cs'],'ManagedRuntimeLoaderTests':['ManagedRuntimeLoader.cs','LaunchDiagnostics.cs']}.items():
 out=reg/(name+'.exe');run([CSC,'/nologo','/platform:x64','/out:'+str(out),tests/(name+'.cs'),*(source/x for x in files)]);results[name]=run([out],cwd=reg)
# Practice logic checks retain native contract/scope cases, with no game process.
practice=ROOT/'Modules/DrivePractice';sources=[practice/'Plugin'/x for x in ['DriveEngine.cs','NativeRuntime.cs','NativeCompatibility.cs','DynastyControllerCompatibility.cs','NativeDrivePolicy.cs','SessionStats.cs','DefenseChoice.cs','Diagnostics.cs']]
for name in ['DriveChecks','NativeDriveChecks','DefenseChecks','OpponentChecks','SessionStatsChecks','ScrimmageChecks','ModeChecks','StartChecks','FirstDownLineChecks','DiagnosticsChecks']:
 managed(name,[common,*sources,practice/'Tools'/(name+'.cs')],refs=['System.IO.Compression.dll'])
report=dict(format=1,work=str(WORK),results=results,modules={n:dict(version=m['version'],dllSha256=m['dllSha256']) for n,m in meta.items()},protectedInstallationUnchanged=True,liveInstall=False,gameAccess=False,published=False)
(ROOT/'Build/user-storage-verification.json').write_text(json.dumps(report,indent=2)+'\n');print('PASS per-user storage suite:',WORK)
