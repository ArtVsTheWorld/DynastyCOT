"""Verify the staged/private COT package without starting any engine."""
from pathlib import Path,PurePosixPath
import hashlib,json,shutil,subprocess,sys,datetime
ROOT=Path(__file__).resolve().parents[1]
def hash_file(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def verify(stage):
    stage=Path(stage).resolve()
    doc=json.loads((stage/'package-manifest.json').read_text())
    assert doc['format']==1 and doc['product']=='DynastyCOT'
    seen=set()
    for entry in doc['files']:
        rel=PurePosixPath(entry['path'])
        if rel.is_absolute() or '..' in rel.parts or ':' in str(rel) or str(rel).casefold() in seen:raise ValueError('Unsafe/duplicate package path')
        seen.add(str(rel).casefold());target=stage.joinpath(*rel.parts).resolve()
        if not target.is_relative_to(stage):raise ValueError('Package path escapes root')
        if hash_file(target)!=entry['sha256']:raise ValueError('Payload differs: '+str(rel))
        if any(x.casefold() in ['runtimehistory','recruitresumehistory','dynastyenhancementsettings','evidence','research','private'] for x in rel.parts):
            raise ValueError('Private or generated data in package')
        if rel.suffix.lower() in ['.ast','.fbproject','.pe','.obj','.lib','.exp']:raise ValueError('Development/game asset in package')
        if rel.name in ['FrostyCore.dll','FrostySdk.dll','MMCModManager.exe']:raise ValueError('SDK or host distributed')
    actual={p.relative_to(stage).as_posix().casefold() for p in stage.rglob('*') if p.is_file()}
    if actual-seen-{'package-manifest.json'}:raise ValueError('Unmanifested package files')
    return doc
def main():
    sdk=Path(sys.argv[1]).resolve();meta=json.loads((ROOT/'Build/current-package.json').read_text());stage=Path(meta['stage'])
    doc=verify(stage)
    assert doc.get('socialRenderMode')=='native-post-containers','Unsupported Social Media rendering mode'
    subprocess.run([sys.executable,str(ROOT/'Tests/test_analytics_packaging.py'),str(stage)],check=True)
    runtime_sources=0
    for item in doc['files']:
        rel=item['path'];source=None
        if rel.startswith('COT-Runtime/DynastyEnhancer/Tools/') and rel.endswith('.mjs'):source=ROOT/'Modules/DynastyEnhancer/Tools'/Path(rel).name
        elif rel.startswith('COT-Runtime/DynastyEnhancer/Content/Runtime/') and rel.endswith('.json'):source=ROOT/'Modules/DynastyEnhancer/Content/Runtime'/Path(rel).name
        if source is not None:
            assert hash_file(source)==item['sha256'],'Rebuild package after runtime source change: '+str(source)
            runtime_sources+=1
    fixture=ROOT/'Build'/('PackageCheck-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'))
    shutil.copytree(stage,fixture)
    compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe');exe=fixture.parent/(fixture.name+'.exe')
    subprocess.run([str(compiler),'/nologo','/platform:x64','/out:'+str(exe),str(ROOT/'Tests/PackageHostChecks.cs')],check=True)
    subprocess.run([str(exe),str(fixture),str(sdk)],check=True)
    core_exe=fixture.parent/(fixture.name+'-Core.exe')
    subprocess.run([str(compiler),'/nologo','/platform:x64','/reference:System.Web.Extensions.dll','/out:'+str(core_exe),str(ROOT/'Core/CotConfiguration.cs'),str(ROOT/'Tests/CotCoreChecks.cs')],check=True)
    subprocess.run([str(core_exe)],check=True)

    # Verify exact manifests reject changed bytes and traversal before any install.
    target=fixture/'Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe';original=target.read_bytes()
    target.write_bytes(original+b'changed')
    try:verify(fixture)
    except ValueError:pass
    else:raise AssertionError('Package verifier accepted tamper')
    target.write_bytes(original)
    manifest=fixture/'package-manifest.json';saved=manifest.read_text();bad=json.loads(saved);bad['files'][0]['path']='../outside'
    manifest.write_text(json.dumps(bad))
    try:verify(fixture)
    except ValueError:pass
    else:raise AssertionError('Package verifier accepted traversal')
    manifest.write_text(saved);verify(fixture)
    result=dict(atUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(),stage=str(stage),
        manifestSha256=hash_file(stage/'package-manifest.json'),coreChecks=106,files=len(doc['files']),runtimeSourceChecks=runtime_sources,singleMmcEntry=True,hostChecks='passed',tamperRejection=True,pathRejection=True,
        liveGameAccess=False,installerChecks='pending')
    (ROOT/'Build/package-verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print('PASS: staged package manifest, actual host, helpers, resources, tamper and traversal. No installation or game access.')
if __name__=='__main__':main()
