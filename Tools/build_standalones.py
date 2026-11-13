"""Source-build remaining one-DLL standalone distributions; never install/publish."""
from pathlib import Path
import argparse,datetime,hashlib,json,re,shutil,subprocess,zipfile
from build_package import clean_adapter,analytics_build_id
ROOT=Path(__file__).resolve().parents[1]
DEFS={
 'BeyondTheField':('DynastyEnhancer','Beyond the Field','1.0.23','DynastyEnhancer.LaunchAction',['DynastyEnhancer.dll','PlayerAnalytics.dll']),
 'ExtraSettings':('ExtraDynastySettings','Extra Settings','0.2.11','OfflineDynastySettings.CombinedSettingsAction',['OfflineDynastySettings.dll','DynastyPlayClock.dll','OfflinePlaycall.dll']),
 'ExpandedPractice':('DrivePractice','Expanded Practice','0.8.23','SituationalPractice.PracticeAction',['SituationalPractice.dll']),
}
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def build_social(work,inputs):
 # Only a configuration filename changes; all captured hook contracts and ABI
 # remain byte-identical source inputs. Existing data directories remain shared.
 native=work/'Native';de=native/'Modules/DynastyEnhancer';src=de/'Source'
 shutil.copytree(ROOT/'Modules/DynastyEnhancer/Source',src)
 shutil.copytree(ROOT/'Modules/DynastyEnhancer/Vendor',de/'Vendor')
 (native/'Compatibility').mkdir();shutil.copyfile(ROOT/'Compatibility/PracticeTextSetter.generated.h',native/'Compatibility/PracticeTextSetter.generated.h')
 p=src/'RuntimeFeed.h';old=p.read_bytes();needle=b'folder/L"dynasty-enhancer.json"';assert old.count(needle)==1
 p.write_bytes(old.replace(needle,b'folder/L"beyond-the-field.json"'))
 vs=Path(subprocess.check_output([r'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe','-latest','-products','*','-requires','Microsoft.VisualStudio.Component.VC.Tools.x86.x64','-property','installationPath'],text=True).strip())
 tool=sorted((vs/'VC/Tools/MSVC').iterdir())[-1];kits=Path(r'C:\Program Files (x86)\Windows Kits\10')
 sdk=sorted(p for p in (kits/'Include').iterdir() if (p/'ucrt').is_dir() and (kits/'Lib'/p.name/'um/x64').is_dir())[-1]
 cl=tool/'bin/Hostx64/x64/cl.exe';inc=[tool/'include',*(sdk/x for x in ['ucrt','shared','um','winrt'])]
 libs=[tool/'lib/x64',kits/'Lib'/sdk.name/'ucrt/x64',kits/'Lib'/sdk.name/'um/x64']
 build_id=re.search(r'cot-(\d+)\.dll$',inputs['socialNative'])[1]
 common=[str(cl),'/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8',*(f'/I{x}' for x in inc)]
 output=work/'BeyondTheFieldNative-v103.dll'
 subprocess.run([*common,'/DCOT_ANALYTICS_BUILD_ID='+build_id+'ull','/DSOCIAL_READ_ONLY_FEED','/DSOCIAL_DUAL_DROPDOWN','/LD',str(src/'NativeSocialMedia.cpp'),'/Fe:'+str(output),'/link',*(f'/LIBPATH:{x}' for x in libs),'/INCREMENTAL:NO'],cwd=work,check=True)
 test=work/'NativeSocialMediaTests.exe'
 (de/'Tests').mkdir()
 for name in [x.name for x in (ROOT/'Modules/DynastyEnhancer/Tests').iterdir() if x.suffix in ['.cpp','.h']]:shutil.copyfile(ROOT/'Modules/DynastyEnhancer/Tests'/name,de/'Tests'/name)
 subprocess.run([*common,'/DCOT_ANALYTICS_BUILD_ID='+build_id+'ull','/DSOCIAL_READ_ONLY_FEED','/DSOCIAL_DUAL_DROPDOWN',str(de/'Tests/NativeSocialMediaTests.cpp'),'/Fe:'+str(test),'/link',*(f'/LIBPATH:{x}' for x in libs)],cwd=work,check=True)
 subprocess.run([str(test)],cwd=work,check=True)
 brand_test=work/'GpiBrandTests.exe'
 subprocess.run([*common,str(de/'Tests/GpiBrandTests.cpp'),'/Fe:'+str(brand_test),'/link',*(f'/LIBPATH:{x}' for x in libs)],cwd=work,check=True)
 subprocess.run([str(brand_test)],cwd=work,check=True)
 fog=work/'BeyondTheFieldRecruitFog-v1.dll'
 subprocess.run([*common,'/DRECRUIT_NOTES_UI','/DCOT_ANALYTICS_BUILD_ID='+build_id+'ull','/LD',str(src/'RecruitingFog.cpp'),'/Fe:'+str(fog),'/link',*(f'/LIBPATH:{x}' for x in libs),'/INCREMENTAL:NO'],cwd=work,check=True)
 for name in ['UserRuntimePathsTests','RecruitingFogTests','RecruitInterestTests','RecruitingStartupTests']:
  exe=work/(name+'.exe');subprocess.run([*common,'/DRECRUIT_NOTES_UI','/DCOT_ANALYTICS_BUILD_ID='+build_id+'ull',str(de/'Tests'/(name+'.cpp')),'/Fe:'+str(exe),'/link',*(f'/LIBPATH:{x}' for x in libs)],cwd=work,check=True);subprocess.run([str(exe)],cwd=work,check=True)
 return output,dict(fogSourceBuilt=True,fogSha256=sha(fog),originalHeaderSha256=hashlib.sha256(old).hexdigest(),generatedHeaderSha256=sha(p),replacement='dynasty-enhancer.json -> beyond-the-field.json',analyticsBuildId=build_id,nativeTests='passed')
def main():
 p=argparse.ArgumentParser();p.add_argument('--mmc-sdk',required=True);p.add_argument('--module',choices=DEFS,action='append');a=p.parse_args();sdk=Path(a.mmc_sdk).resolve()
 reference=ROOT/'Release/DynastyCOT-1.0.2.zip';inputs=json.loads((ROOT/'cot.build-inputs.json').read_text(encoding='utf-8'))
 for rel,digest in inputs['sourceHashes'].items():
  if sha(ROOT/rel)!=digest:raise ValueError('Native source changed: '+rel)
 results=[]
 for name in a.module or DEFS:
  module,title,version,action,legacy=DEFS[name];stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f')
  work=ROOT/'Build'/('Standalone-'+name+'-'+stamp);stage=work/'Package';generated=work/'Generated';stage.mkdir(parents=True);generated.mkdir();payload={};sources=[];branding=None
  with zipfile.ZipFile(reference) as ref:
   manifest=json.loads(ref.read('package-manifest.json'))
   for item in manifest['files']:
    if item['module'] not in ([module,'DynastyEnhancerNews'] if name=='BeyondTheField' else [module]):continue
    relative=item['path']
    if relative.endswith('.pdb'):continue
    content=ref.read(relative)
    if hashlib.sha256(content).hexdigest()!=item['sha256']:raise ValueError('Reference integrity failure')
    payload[relative]=content
  def emit(file,text):
   path=generated/file;path.write_text(text,encoding='utf-8',newline='\n');return path
  if name=='BeyondTheField':
   analytics_build_id(inputs['socialNative'],inputs['fogNative'])
   for key in ['socialNative','fogNative']:
    if sha(ROOT/inputs[key])!=inputs['nativeHashes'][key]:raise ValueError('Source-built native integrity differs: '+key)
   native,branding=build_social(work,inputs);payload['COT-Runtime/DynastyEnhancer/BeyondTheFieldNative-v103.dll']=native.read_bytes()
   # Keep the tested native pair together rather than importing old fog bytes.
   payload['COT-Runtime/DynastyEnhancer/BeyondTheFieldRecruitFog-v1.dll']=(work/'BeyondTheFieldRecruitFog-v1.dll').read_bytes()
   # Generator/content must come from the current source, not the comparison
   # bundle. Remove obsolete managed filenames from the new embedded payload;
   # saved account/publication keys and user-data paths stay unchanged.
   scoped='COT-Runtime/DynastyEnhancer/'
   payload={path:content for path,content in payload.items() if not path.startswith((scoped+'Tools/',scoped+'Content/Runtime/'))}
   for folder,pattern in [('Tools','*.mjs'),('Content/Runtime','*.json')]:
    for source in sorted((ROOT/'Modules/DynastyEnhancer'/folder).glob(pattern)):
     if source.name=='audit_rc016_checkpoint.mjs':continue
     payload[scoped+folder+'/'+source.name]=source.read_bytes()
   payload['LICENSES/PlayerAnalytics-NOTICES.txt']=(ROOT/'Modules/DynastyEnhancer/Source/Analytics.NOTICES.txt').read_bytes()
   payload['COT-Runtime/DynastyEnhancer/beyond-the-field.json']=payload.pop('COT-Runtime/DynastyEnhancer/dynasty-enhancer.json')
   src=ROOT/'Modules/DynastyEnhancer/Source';adapter=clean_adapter((src/'MmcRecruitingPlugin.cs').read_text(encoding='utf-8'))
   adapter=adapter.replace('Path.Combine(PluginsFolder,"DynastyEnhancer","Runtime-v103")','Path.Combine(DynastyCOTStandalone.Runtime.Mmc,"COT-Runtime","DynastyEnhancer")').replace('dynasty-enhancer.json','beyond-the-field.json').replace('DynastyEnhancer.payload-sha256.txt','BeyondTheField.payload-sha256.txt')
   old='var issue=ConfigurationIssue();if(issue.Length!=0){if(issue!=previousIssue)Note(issue);previousIssue=issue;Thread.Sleep(1000);continue;}previousIssue=null;';assert old in adapter
   adapter=adapter.replace(old,'// Social Media configuration is checked independently below.')
   old='       attempts[key]=count+1;retryAt[key]=DateTime.UtcNow.AddSeconds(10);';assert old in adapter
   adapter=adapter.replace(old,'       if(nativeFile==NativeFile){var issue=ConfigurationIssue();if(issue.Length!=0){retryAt[key]=DateTime.UtcNow.AddSeconds(10);if(issue!=previousIssue)Note(issue);previousIssue=issue;continue;}previousIssue=null;}\n'+old,1)
   sources=[emit('BeyondTheFieldAdapter.cs',adapter),ROOT/'Modules/GamedayAtmosphere/Source/ManagedRuntimeLoader.cs',ROOT/'Modules/GamedayAtmosphere/Source/LaunchDiagnostics.cs']
   scoped='COT-Runtime/DynastyEnhancer/'
   de_manifest=emit('BeyondTheField.payload-sha256.txt','')
   # Include every generator/content artifact, excluding only the config file.
   de_manifest.write_text(''.join(hashlib.sha256(content).hexdigest()+'|'+path[len(scoped):]+'\n' for path,content in sorted(payload.items()) if path.startswith(scoped) and path!=scoped+'beyond-the-field.json'),encoding='utf-8')
   additional=[(de_manifest,'BeyondTheField.payload-sha256.txt')]
  elif name=='ExtraSettings':
   extra=ROOT/'Modules/ExtraDynastySettings';src=extra/'Plugin';build=(src/'build.ps1').read_text(encoding='utf-8');block=build.split('$sources = @(',1)[1].split('\n)',1)[0]
   sources=[(src if owner=='here' else extra)/rel.replace('\\','/') for owner,rel in re.findall(r"\(Join-Path \$(here|workspace) '([^']+)'\)",block) if rel!='AssemblyInfo.cs'];additional=[]
   helper_sources=[src/x for x in ['PersistenceHelper.cs','DynastyClockProfiles.cs','PersistenceContracts.cs','ClockPreferences.cs','DiagnosticLog.cs']]+[ROOT/'Core/UserStorage.cs']
   helper=work/'ExtraSettings.Persistence.exe'
   subprocess.run([str(Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe')),'/nologo','/target:winexe','/platform:x64','/optimize+','/define:COT_BRANDED_RUNTIME','/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/reference:System.IO.Compression.FileSystem.dll','/out:'+str(helper),*(str(x) for x in helper_sources)],check=True)
   payload['Plugins/OfflineDynastySettings/ExtraSettings.Persistence.exe']=helper.read_bytes()
   sources.append(emit('ExpectedRuntime.cs','namespace OfflineDynastySettings {internal static class ExpectedRuntime {internal const string Hash="'+sha(helper)+'";}}'))
  else:
   src=ROOT/'Modules/DrivePractice/Plugin';sources=[src/file for file in ['PracticeAction.cs','RuntimeFiles.cs','Diagnostics.cs']];additional=[]
   helper_sources=[src/x for x in ['DriveEngine.cs','NativeRuntime.cs','NativeCompatibility.cs','DynastyControllerCompatibility.cs','NativeDrivePolicy.cs','PracticeRunner.cs','SessionStats.cs','DefenseChoice.cs','Diagnostics.cs']]+[ROOT/'Core/UserStorage.cs']
   helper=work/'ExpandedPractice.Runtime.exe'
   subprocess.run([str(Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe')),'/nologo','/target:winexe','/platform:x64','/optimize+','/define:COT_BRANDED_RUNTIME','/reference:System.IO.Compression.dll','/out:'+str(helper),*(str(x) for x in helper_sources)],check=True)
   payload['Plugins/SituationalPractice/ExpandedPractice.Runtime.exe']=helper.read_bytes()
   sources.append(emit('ExpectedRuntime.cs','namespace SituationalPractice {internal static class ExpectedRuntime {internal const string Hash="'+sha(helper)+'";}}'))
  # No mod settings/history paths are included in this extraction allowlist.
  payload_zip=work/'payload.zip'
  with zipfile.ZipFile(payload_zip,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
   for path,content in sorted(payload.items()):z.writestr(path,content)
  payload_manifest=emit('payload.json',json.dumps({path:hashlib.sha256(content).hexdigest() for path,content in sorted(payload.items())},indent=2))
  identity=emit('Identity.cs','namespace DynastyCOTStandalone {internal static class Identity {internal const string Version='+json.dumps(version)+';internal const string Module='+json.dumps(module)+';internal const string Title='+json.dumps(title)+';internal static readonly string[] LegacyDlls=new[]{'+','.join(json.dumps(x) for x in legacy)+'};}}')
  entry=emit('StandaloneEntry.cs','''using System;using System.Diagnostics;using System.Reflection;using System.Threading;using Frosty.Core;using Frosty.Core.Attributes;using FrostySdk.Interfaces;
[assembly: AssemblyTitle(TITLE)] [assembly: AssemblyProduct(TITLE)] [assembly: AssemblyVersion(VERSION4)] [assembly: AssemblyFileVersion(VERSION4)] [assembly: PluginVersion(VERSION)] [assembly: PluginAuthor("Ace")] [assembly: PluginDisplayName(TITLE)] [assembly: RegisterExecutionAction(typeof(DynastyCOTStandalone.Entry))]
namespace DynastyCOTStandalone {public sealed class Entry:ExecutionAction {static int starting;static ExecutionAction action;
 public Entry(){using(var process=Process.GetCurrentProcess())if(process.ProcessName=="MMCModManager")Start(null,PluginManagerType.ModManager,CancellationToken.None);}
 public override Action<ILogger,PluginManagerType,CancellationToken> PreLaunchAction{get{return Start;}}
 public override Action<ILogger,PluginManagerType,CancellationToken> PostLaunchAction{get{return Start;}}
 static void Start(ILogger log,PluginManagerType type,CancellationToken token){if(Interlocked.CompareExchange(ref starting,1,0)!=0)return;
 new Thread(()=>{try{if(Runtime.Initialize()){if(action==null)action=new ACTION();action.PreLaunchAction(log,type,token);Runtime.Note("Matching embedded payload verified; module startup requested. Native activation is reported separately.");}}
 catch(Exception error){Runtime.Note(error.ToString());if(log!=null)try{log.Log("["+Identity.Title+"] "+error.Message,new object[0]);}catch{}}
 finally{Interlocked.Exchange(ref starting,0);}}){IsBackground=true,Name=Identity.Title+" package preparation"}.Start();}
}}'''.replace('VERSION4',json.dumps(version+'.0')).replace('VERSION',json.dumps(version)).replace('TITLE',json.dumps(title)).replace('ACTION',action))
  output=stage/(name+'.dll');csc=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe')
  source_list=[*sources,ROOT/'Core/UserStorage.cs',ROOT/'Packaging/Standalone/StandaloneRuntime.cs',identity,entry]
  subprocess.run([str(csc),'/nologo','/target:library','/platform:x64','/optimize+','/define:COMBINED_DYNASTY_SETTINGS;COT_BRANDED_RUNTIME','/out:'+str(output),'/reference:'+str(sdk/'FrostyCore.dll'),'/reference:'+str(sdk/'FrostySdk.dll'),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/reference:System.IO.Compression.FileSystem.dll','/resource:'+str(payload_zip)+',Standalone.payload.zip','/resource:'+str(payload_manifest)+',Standalone.payload.json',*(f'/resource:{path},{resource}' for path,resource in additional),*(str(path) for path in source_list)],check=True)
  docs=ROOT/'Packaging/Standalone'/name
  for file in [x.name for x in docs.iterdir() if x.name in ['README.txt','FEATURES.md','LICENSES.txt']]:shutil.copyfile(docs/file,stage/file)
  tracked=[path for path in source_list if not path.is_relative_to(work)]+[Path(__file__),docs/'README.txt',docs/'FEATURES.md']
  if (docs/'LICENSES.txt').exists():tracked.append(docs/'LICENSES.txt')
  if name!='BeyondTheField':tracked+=helper_sources
  if name=='BeyondTheField':
   tracked += [p for p in src.iterdir() if p.suffix in ['.h','.cpp','.cs']]
   tracked += [ROOT/rel for rel in inputs['sourceHashes']]+[ROOT/'cot.build-inputs.json',*[p for p in (ROOT/'Modules/DynastyEnhancer/Tests').iterdir() if p.suffix in ['.cpp','.h']]]
   tracked += [p for folder,pattern in [('Tools','*.mjs'),('Content/Runtime','*.json')] for p in (ROOT/'Modules/DynastyEnhancer'/folder).glob(pattern) if p.name!='audit_rc016_checkpoint.mjs']
  result=dict(name=name,module=module,title=title,version=version,stage=str(stage),work=str(work),dll=str(output),dllSha256=sha(output),referenceArchiveSha256=sha(reference),payload=json.loads(payload_manifest.read_text(encoding='utf-8')),branding=branding,sourceHashes={str(x.relative_to(ROOT).as_posix()):sha(x) for x in tracked},sdk=str(sdk),installed=False,published=False)
  (ROOT/'Build'/('current-standalone-'+name+'.json')).write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');results.append(result)
 print('Built standalones:',', '.join(x['name'] for x in results))
if __name__=='__main__':main()
