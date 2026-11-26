"""Build only a private Extra Settings candidate. Never activate a plugin."""
import argparse,ast,datetime,hashlib,json,re,subprocess,zipfile
from pathlib import Path
import build_standalones as base
R=Path.cwd().resolve()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--version',default='0.2.13');p.add_argument('--output',default='Build/FormationSubsCandidate');p.add_argument('--label',default='kickoff1');a=p.parse_args()
 if not re.fullmatch(r'\d+\.\d+\.\d+',a.version):raise ValueError('Version format')
 if not re.fullmatch(r'[a-z0-9-]{1,32}',a.label):raise ValueError('Private label format')
 output=(R/a.output).resolve()
 if not output.is_relative_to(R/'Build'):raise ValueError('Private build output must remain under Build')
 output.mkdir(parents=True,exist_ok=True);stamp=datetime.datetime.now().strftime('%Y%m%dT%H%M%S');work=output/('SourceBuild-'+stamp);work.mkdir();generated=work/'Generated';generated.mkdir();stage=output/'Plugins';stage.mkdir(exist_ok=True)
 sdk=Path(r'C:/Apps/CFB 27/MMC/MMC_ModManager_v1.1.0.6');csc=Path(r'C:/Windows/Microsoft.NET/Framework64/v4.0.30319/csc.exe')
 original=json.loads((R/'Build/current-standalone-ExtraSettings.json').read_text());previous=Path(original['work']);source=R/'Modules/ExtraDynastySettings/Plugin';module=source.parent
 def emit(name,text):target=generated/name;target.write_text(text,encoding='utf8');return target
 diagnosticSource=source/'DiagnosticLog.cs'
 diagnosticText,count=re.subn(r'internal const string Version = "[^"]+";',lambda m:'internal const string Version = "'+a.version+'";',diagnosticSource.read_text())
 if count!=1:raise ValueError('Diagnostic/runtime version contract')
 diagnostic=emit('DiagnosticLog.cs',diagnosticText)
 helperSources=[source/n for n in ['PersistenceHelper.cs','DynastyClockProfiles.cs','PersistenceContracts.cs','ClockPreferences.cs','DiagnosticLog.cs']]+[R/'Core/UserStorage.cs']
 helperSources=[diagnostic if p==diagnosticSource else p for p in helperSources]
 helper=work/'ExtraSettings.Persistence.exe';subprocess.run([str(csc),'/nologo','/target:winexe','/platform:x64','/optimize+','/warnaserror+','/define:COT_BRANDED_RUNTIME','/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/reference:System.IO.Compression.FileSystem.dll','/out:'+str(helper),*map(str,helperSources)],check=True)
 with zipfile.ZipFile(previous/'payload.zip') as z:payload={n:z.read(n) for n in z.namelist()}
 payload['Plugins/OfflineDynastySettings/ExtraSettings.Persistence.exe']=helper.read_bytes()
 payloadZip=work/'payload.zip'
 with zipfile.ZipFile(payloadZip,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
  for name,data in sorted(payload.items()):z.writestr(name,data)
 manifest=emit('payload.json',json.dumps({n:hashlib.sha256(data).hexdigest() for n,data in sorted(payload.items())},indent=2))
 expected=emit('ExpectedRuntime.cs','namespace OfflineDynastySettings {internal static class ExpectedRuntime {internal const string Hash="'+sha(helper)+'";}}')
 block=(source/'build.ps1').read_text().split('$sources = @(',1)[1].split('\n)',1)[0]
 sources=[(source if owner=='here' else module)/rel.replace('\\','/') for owner,rel in re.findall(r"\(Join-Path \$(here|workspace) '([^']+)'\)",block) if rel!='AssemblyInfo.cs']+[expected]
 sources=[diagnostic if p==diagnosticSource else p for p in sources]
 template=next(n.value for n in ast.walk(ast.parse((R/'Tools/build_standalones.py').read_text())) if isinstance(n,ast.Constant) and isinstance(n.value,str) and '[assembly: AssemblyTitle(TITLE)]' in n.value and 'public sealed class Entry:' in n.value)
 moduleName,title,_,action,legacy=base.DEFS['ExtraSettings']
 identity=emit('Identity.cs','namespace DynastyCOTStandalone {internal static class Identity {internal const string Version='+json.dumps(a.version)+';internal const string Module='+json.dumps(moduleName)+';internal const string Title='+json.dumps(title)+';internal static readonly string[] LegacyDlls=new[]{'+','.join(json.dumps(x) for x in legacy)+'};}}')
 entry=emit('StandaloneEntry.cs',template.replace('VERSION4',json.dumps(a.version+'.0')).replace('VERSION',json.dumps(a.version)).replace('TITLE',json.dumps(title)).replace('ACTION',action))
 sources += [R/'Core/UserStorage.cs',R/'Packaging/Standalone/StandaloneRuntime.cs',identity,entry]
 dll=stage/'ExtraSettings.dll';subprocess.run([str(csc),'/nologo','/target:library','/platform:x64','/optimize+','/warnaserror+','/define:COMBINED_DYNASTY_SETTINGS;COT_BRANDED_RUNTIME','/out:'+str(dll),'/reference:'+str(sdk/'FrostyCore.dll'),'/reference:'+str(sdk/'FrostySdk.dll'),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/reference:System.IO.Compression.FileSystem.dll','/resource:'+str(payloadZip)+',Standalone.payload.zip','/resource:'+str(manifest)+',Standalone.payload.json',*map(str,sources)],check=True)
 record=dict(module='ExtraSettings',version=a.version,privateCandidate=a.label,dll=str(dll),dllSha256=sha(dll),work=str(work),publicBaseline=dict(version='0.2.21',sourceRevision='HEAD',note='Current integrated source including automatic kickoff/return; legacy 0.2.12 payload used only as a one-helper packaging template'),payload=json.loads(manifest.read_text()),sourceHashes={p.relative_to(R).as_posix():sha(p) for p in {*sources,*helperSources,diagnosticSource,Path(__file__)} if not p.is_relative_to(work)},generatedHashes={p.relative_to(work).as_posix():sha(p) for p in generated.iterdir() if p.is_file()},installed=False,published=False,gameplayAcceptance='pending disposable-save assignment test')
 (output/'candidate.json').write_text(json.dumps(record,indent=2));print(dll);print(record['dllSha256'])
if __name__=='__main__':main()
