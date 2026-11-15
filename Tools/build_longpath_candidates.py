"""Source-build bounded long-path candidates; never invoke a module callback."""
from pathlib import Path
import json,ast,datetime,subprocess,shutil,zipfile,hashlib
import build_standalones as base
from build_package import clean_adapter
R=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 import sys
 reuse='--reuse-native' in sys.argv
 onlyBtf='--only-btf' in sys.argv
 btfVersion=next((x.split('=',1)[1] for x in sys.argv if x.startswith('--btf-version=')),'1.0.26')
 results=[];stamp=datetime.datetime.now().strftime('%Y%m%dT%H%M%S');sdk=Path(r'C:\Apps\CFB 27\MMC\MMC_ModManager_v1.1.0.6');csc=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe')
 inputs=json.loads((R/'cot.build-inputs.json').read_text());out=R/next((x.split('=',1)[1] for x in sys.argv if x.startswith('--output-root=')),'Build/LongPathAudit');out.mkdir(parents=True,exist_ok=True);template=next(n.value for n in ast.walk(ast.parse((R/'Tools/build_standalones.py').read_text())) if isinstance(n,ast.Constant) and isinstance(n.value,str) and '[assembly: AssemblyTitle(TITLE)]' in n.value and 'public sealed class Entry:' in n.value)
 for name,version in [('BeyondTheField',btfVersion),('ExtraSettings','0.2.13'),('ExpandedPractice','0.8.25')]:
  if onlyBtf and name!='BeyondTheField':continue
  original=json.loads((R/'Build'/('current-standalone-'+name+'.json')).read_text());work=out/(name+'-'+stamp);work.mkdir();generated=work/'Generated';generated.mkdir();stage=work/'Package';stage.mkdir();previous=Path(original['work']).resolve();payload={}
  with zipfile.ZipFile(previous/'payload.zip') as z:payload={n:z.read(n) for n in z.namelist()}
  module,title,_,action,legacy=base.DEFS[name];sources=[];additional=[];nativeProof=None;helperSources=[]
  def emit(n,t):p=generated/n;p.write_text(t,encoding='utf8');return p
  if name=='BeyondTheField':
   if reuse:
    prior=json.loads((out/(name+'-candidate.json')).read_text());priorWork=Path(prior['work'])
    for rel,h in prior['sourceHashes'].items():
     if Path(rel).suffix in ['.cpp','.h','.inc']:assert sha(R/rel)==h,rel
    for leaf in ['BeyondTheFieldNative-v103.dll','BeyondTheFieldRecruitFog-v1.dll']:shutil.copyfile(priorWork/leaf,work/leaf)
    native=work/'BeyondTheFieldNative-v103.dll';nativeProof=dict(prior['nativeProof'],sourceBuildLineage=str(priorWork))
   else:native,nativeProof=base.build_social(work,inputs)
   for leaf in ['BeyondTheFieldNative-v103.dll','BeyondTheFieldRecruitFog-v1.dll']:payload['COT-Runtime/DynastyEnhancer/'+leaf]=(work/leaf).read_bytes()
   adapter=clean_adapter((R/'Modules/DynastyEnhancer/Source/MmcRecruitingPlugin.cs').read_text())
   adapter=adapter.replace('Path.Combine(PluginsFolder,"DynastyEnhancer","Runtime-v103")','Path.Combine(DynastyCOTStandalone.Runtime.Mmc,"COT-Runtime","DynastyEnhancer")').replace('dynasty-enhancer.json','beyond-the-field.json').replace('DynastyEnhancer.payload-sha256.txt','BeyondTheField.payload-sha256.txt')
   old='var issue=ConfigurationIssue();if(issue.Length!=0){if(issue!=previousIssue)Note(issue);previousIssue=issue;Thread.Sleep(1000);continue;}previousIssue=null;';assert old in adapter;adapter=adapter.replace(old,'// Social Media configuration is checked independently below.')
   old='       attempts[key]=count+1;retryAt[key]=DateTime.UtcNow.AddSeconds(10);';assert old in adapter;adapter=adapter.replace(old,'       if(nativeFile==NativeFile){var issue=ConfigurationIssue();if(issue.Length!=0){retryAt[key]=DateTime.UtcNow.AddSeconds(10);if(issue!=previousIssue)Note(issue);previousIssue=issue;continue;}previousIssue=null;}\n'+old,1)
   sources=[emit('BeyondTheFieldAdapter.cs',adapter),*[R/'Modules/GamedayAtmosphere/Source'/n for n in ['ManagedRuntimeLoader.cs','LaunchDiagnostics.cs']]]
   prefix='COT-Runtime/DynastyEnhancer/';manifest=emit('BeyondTheField.payload-sha256.txt',''.join(hashlib.sha256(b).hexdigest()+'|'+n[len(prefix):]+'\n' for n,b in sorted(payload.items()) if n.startswith(prefix) and n!=prefix+'beyond-the-field.json'));additional=[(manifest,manifest.name)]
  else:
   if name=='ExtraSettings':
    src=R/'Modules/ExtraDynastySettings/Plugin';moduleRoot=src.parent;block=(src/'build.ps1').read_text().split('$sources = @(',1)[1].split('\n)',1)[0]
    import re
    sources=[(src if owner=='here' else moduleRoot)/rel.replace('\\','/') for owner,rel in re.findall(r"\(Join-Path \$(here|workspace) '([^']+)'\)",block) if rel!='AssemblyInfo.cs'];helperSources=[src/n for n in ['PersistenceHelper.cs','DynastyClockProfiles.cs','PersistenceContracts.cs','ClockPreferences.cs','DiagnosticLog.cs']];helperName='ExtraSettings.Persistence.exe';prefix='Plugins/OfflineDynastySettings/';ns='OfflineDynastySettings'
   else:
    src=R/'Modules/DrivePractice/Plugin';sources=[src/n for n in ['PracticeAction.cs','RuntimeFiles.cs','Diagnostics.cs']];helperSources=[src/n for n in ['DriveEngine.cs','NativeRuntime.cs','NativeCompatibility.cs','DynastyControllerCompatibility.cs','NativeDrivePolicy.cs','PracticeRunner.cs','SessionStats.cs','DefenseChoice.cs','Diagnostics.cs']];helperName='ExpandedPractice.Runtime.exe';prefix='Plugins/SituationalPractice/';ns='SituationalPractice'
   helper=work/helperName;subprocess.run([str(csc),'/nologo','/target:winexe','/platform:x64','/optimize+','/warnaserror+','/define:COT_BRANDED_RUNTIME','/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/reference:System.IO.Compression.FileSystem.dll','/out:'+str(helper),*map(str,[*helperSources,R/'Core/UserStorage.cs'])],check=True);payload[prefix+helperName]=helper.read_bytes();sources.append(emit('ExpectedRuntime.cs','namespace '+ns+' {internal static class ExpectedRuntime {internal const string Hash="'+sha(helper)+'";}}'))
  payloadZip=work/'payload.zip'
  with zipfile.ZipFile(payloadZip,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
   for n,b in sorted(payload.items()):z.writestr(n,b)
  manifest=emit('payload.json',json.dumps({n:hashlib.sha256(b).hexdigest() for n,b in sorted(payload.items())},indent=2))
  identity=emit('Identity.cs','namespace DynastyCOTStandalone {internal static class Identity {internal const string Version='+json.dumps(version)+';internal const string Module='+json.dumps(module)+';internal const string Title='+json.dumps(title)+';internal static readonly string[] LegacyDlls=new[]{'+','.join(json.dumps(n) for n in legacy)+'};}}')
  entry=emit('StandaloneEntry.cs',template.replace('VERSION4',json.dumps(version+'.0')).replace('VERSION',json.dumps(version)).replace('TITLE',json.dumps(title)).replace('ACTION',action));dll=stage/(name+'.dll');sources+=[R/'Core/UserStorage.cs',R/'Packaging/Standalone/StandaloneRuntime.cs',identity,entry]
  subprocess.run([str(csc),'/nologo','/target:library','/platform:x64','/optimize+','/warnaserror+','/define:COMBINED_DYNASTY_SETTINGS;COT_BRANDED_RUNTIME','/out:'+str(dll),'/reference:'+str(sdk/'FrostyCore.dll'),'/reference:'+str(sdk/'FrostySdk.dll'),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/reference:System.IO.Compression.FileSystem.dll','/resource:'+str(payloadZip)+',Standalone.payload.zip','/resource:'+str(manifest)+',Standalone.payload.json',*(f'/resource:{p},{n}' for p,n in additional),*map(str,sources)],check=True)
  oldStage=Path(original['stage']).resolve()
  for p in oldStage.iterdir():
   if p.suffix.lower() in ['.txt','.md','.pdf']:shutil.copyfile(p,stage/p.name)
  tracked=[p for p in [*sources,*helperSources] if not p.is_relative_to(work)]
  if name=='BeyondTheField':tracked += [p for p in (R/'Modules/DynastyEnhancer/Source').rglob('*') if p.is_file() and p.suffix in ['.cpp','.h','.inc']]+[R/'Core/NativeFileIO.h',R/'Compatibility/PracticeTextSetter.generated.h']
  record=dict(name=name,version=version,privateCandidate='longpath2' if name=='BeyondTheField' else 'longpath1',stage=str(stage),work=str(work),dll=str(dll),dllSha256=sha(dll),publicBaseline=dict(version=original['version'],dllSha256=original['dllSha256']),payload=json.loads(manifest.read_text()),changedPayload=[n for n,b in payload.items() if hashlib.sha256(b).hexdigest()!=original['payload'].get(n)],sourceHashes={p.relative_to(R).as_posix():sha(p) for p in tracked},nativeProof=nativeProof,managedSourceBuilt=True,helperSourceBuilt=name!='BeyondTheField',sdk=str(sdk),installed=False,published=False)
  (out/(name+'-candidate.json')).write_text(json.dumps(record,indent=2));results.append(record);print('BUILT',name,version,flush=True)
 if onlyBtf and (out/'candidates.json').exists():
  prior=json.loads((out/'candidates.json').read_text(encoding='utf8'));results=[results[0] if x['name']=='BeyondTheField' else x for x in prior]
 (out/'candidates.json').write_text(json.dumps(results,indent=2))
if __name__=='__main__':main()
