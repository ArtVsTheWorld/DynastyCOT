"""Build the private COT host and stage payloads from source/build outputs only."""
from pathlib import Path
import argparse,hashlib,json,re,shutil,subprocess,datetime,zipfile
from build_legacy_payloads import entries as owned_entries
ROOT=Path(__file__).resolve().parents[1]
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def clean_adapter(source):
    return re.sub(r'^\[assembly:.*\]\s*\n','',source,flags=re.M)

def analytics_build_id(social,fog):
    s=re.fullmatch(r'DynastyEnhancerNative-cot-([0-9]+)[.]dll',Path(social).name)
    f=re.fullmatch(r'DynastyEnhancerRecruitFog-p([0-9]+)[.]dll',Path(fog).name)
    if not s or not f or s[1]!=f[1] or not 0<int(s[1])<2**64:
        raise RuntimeError('Analytics requires a matched pair of source-built native payloads')
    return s[1]
def main():
    p=argparse.ArgumentParser()
    p.add_argument('--mmc-sdk',required=True)
    p.add_argument('--node',default=r'C:\Program Files\nodejs\node.exe')
    p.add_argument('--social-native')
    p.add_argument('--fog-native')
    a=p.parse_args();sdk=Path(a.mmc_sdk).resolve();node=Path(a.node).resolve()
    inputs=json.loads((ROOT/'cot.build-inputs.json').read_text())
    if inputs.get('socialRenderMode')!='native-post-containers':raise RuntimeError('Rebuild with the supported native Social Media post renderer')
    for rel,expected in inputs['sourceHashes'].items():
        if digest(ROOT/rel)!=expected:raise RuntimeError('Rebuild native payloads after source change: '+rel)
    a.social_native=a.social_native or inputs['socialNative']
    a.fog_native=a.fog_native or inputs['fogNative']
    analytics_build=analytics_build_id(a.social_native,a.fog_native)
    product=json.loads((ROOT/'cot.product.json').read_text())
    if not re.fullmatch(r'\d+\.\d+\.\d+-private',product['version']):raise RuntimeError('Expected a private product version')
    for name in ['FrostyCore.dll','FrostySdk.dll']:
        if not (sdk/name).is_file():raise RuntimeError('Missing MMC SDK '+name)
    node_version=subprocess.check_output([str(node),'--version'],text=True).strip()
    license_file=ROOT/'Vendor/Node'/('LICENSE-'+node_version+'.txt')
    if not license_file.is_file():raise RuntimeError('Supply matching Node notices before bundling '+node_version)
    stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    work=ROOT/'Build'/('COT-'+stamp);stage=work/'Package';generated=work/'Generated'
    generated.mkdir(parents=True);stage.mkdir();sources=[];entries=[]
    def emit(name,text):
        dest=generated/name;dest.write_text(text,encoding='utf-8',newline='\n');return str(dest)
    def copy(src,relative,module):
        src=Path(src)
        if not src.is_file():raise RuntimeError('Missing build input: '+str(src))
        dest=stage/relative;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(src,dest)
        entries.append(dict(module=module,path=relative,sha256=digest(dest)))
    de=ROOT/'Modules/DynastyEnhancer';dga=ROOT/'Modules/GamedayAtmosphere';extra=ROOT/'Modules/ExtraDynastySettings';practice=ROOT/'Modules/DrivePractice'
    def version(module):
        text=(module/'Plugin/AssemblyInfo.cs').read_text()
        return re.search(r'PluginVersion\("([^"]+)"\)',text).group(1)
    extra_version=version(extra);practice_version=version(practice)
    atmosphere_version=json.loads((dga/'Source/Product.json').read_text())['version']
    practice_helper=practice/('Release/SituationalPractice-'+practice_version+'/SituationalPractice/SituationalPractice.Runtime.exe')
    contract=json.loads((ROOT/'Compatibility/practice-contract.json').read_text())
    if digest(practice_helper)!=contract['helperSha256']:
        raise RuntimeError('Regenerate and verify the Practice hook contract, then rebuild Enhancer')
    # No previous MMC runtime is a source input.
    runtime='COT-Runtime/DynastyEnhancer/'
    copy(ROOT/a.social_native,runtime+'DynastyEnhancerNative-v103.dll','DynastyEnhancer')
    copy(ROOT/a.fog_native,runtime+'DynastyEnhancerRecruitFog-v1.dll','DynastyEnhancer')
    copy(de/'Build/LoadDynastyEnhancer.exe',runtime+'LoadDynastyEnhancer.exe','DynastyEnhancer')
    for path in sorted((de/'Tools').glob('*.mjs')):
        # Read-only checkpoint research needs private schema/parser inputs and
        # is a development tool, never part of the installed runtime.
        if path.name=='audit_rc016_checkpoint.mjs':continue
        copy(path,runtime+'Tools/'+path.name,'DynastyEnhancer')
    for path in sorted((de/'Content/Runtime').glob('*.json')):copy(path,runtime+'Content/Runtime/'+path.name,'DynastyEnhancer')
    config=stage/runtime/'dynasty-enhancer.json'
    config.write_text(json.dumps(dict(node='../Shared/node.exe',exporter='Tools/export_runtime.mjs'),indent=2)+'\n')
    entries.append(dict(module='DynastyEnhancer',path=runtime+'dynasty-enhancer.json',sha256=digest(config)))
    copy(node,'COT-Runtime/Shared/node.exe','DynastyEnhancerNews')
    copy(license_file,'LICENSES/Node-'+node_version+'.txt','DynastyEnhancerNews')
    for path in (de/'LICENSES').glob('*.txt'):copy(path,'LICENSES/'+path.name,'DynastyEnhancer')
    copy(de/'Source/Analytics.NOTICES.txt','LICENSES/PlayerAnalytics-NOTICES.txt','DynastyEnhancer')
    copy(de/'Vendor/json.LICENSE.txt','LICENSES/nlohmann-json-MIT.txt','DynastyEnhancer')
    # Keep the established external-helper data layouts; only native DLLs need to
    # live outside the recursive Plugins scan.
    copy(extra/('Release/OfflineDynastySettings-'+extra_version+'/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe'),
         'Plugins/OfflineDynastySettings/OfflineDynastySettings.Persistence.exe','ExtraDynastySettings')
    copy(practice_helper,
         'Plugins/SituationalPractice/SituationalPractice.Runtime.exe','DrivePractice')
    copy(practice_helper.with_suffix('.pdb'),
         'Plugins/SituationalPractice/SituationalPractice.Runtime.pdb','DrivePractice')
    # Enhancer's legacy resource hash list is scoped to its complete source-built payload.
    de_manifest=emit('enhancer-payload.txt',''.join(
        e['sha256']+'|'+e['path'][len(runtime):]+'\n'
        for e in entries if e['path'].startswith(runtime) and not e['path'].endswith('dynasty-enhancer.json')))
    copy(ROOT/'Packaging/Install.ps1','Install.ps1','Core')
    copy(ROOT/'Packaging/README.txt','README.txt','Core')
    copy(ROOT/'LIVE-ACCEPTANCE.md','LIVE-ACCEPTANCE.md','Core')
    adapter=clean_adapter((de/'Source/MmcRecruitingPlugin.cs').read_text())
    adapter=adapter.replace('Path.Combine(PluginsFolder,"DynastyEnhancer","Runtime-v103")','DynastyCOT.CotPaths.Runtime("DynastyEnhancer")')
    old='var issue=ConfigurationIssue();if(issue.Length!=0){if(issue!=previousIssue)Note(issue);previousIssue=issue;Thread.Sleep(1000);continue;}previousIssue=null;'
    assert old in adapter;adapter=adapter.replace(old,'// Generator availability is evaluated for Social Media only.')
    old='       attempts[key]=count+1;retryAt[key]=DateTime.UtcNow.AddSeconds(10);'
    assert old in adapter
    adapter=adapter.replace(old,'''       if(nativeFile==NativeFile){var issue=ConfigurationIssue();if(issue.Length!=0){retryAt[key]=DateTime.UtcNow.AddSeconds(10);if(issue!=previousIssue)Note(issue);previousIssue=issue;continue;}previousIssue=null;}
'''+old,1)
    adapter=adapter.replace('var config=new JavaScriptSerializer()', 'DynastyCOT.LaunchAction.VerifyModule("DynastyEnhancerNews");\n    var config=new JavaScriptSerializer()')
    sources.append(emit('EnhancerAdapter.cs',adapter))
    sources.append(emit('AtmosphereAdapter.cs',clean_adapter((dga/'Source/MmcPlugin.cs').read_text())))
    for name in ['LaunchDiagnostics.cs','PluginSettings.cs','PluginStorage.cs','ManagedRuntimeLoader.cs']:
        sources.append(str(dga/'Source'/name))
    dga_build=dga/('Build/Release'+atmosphere_version.replace('.',''))
    sources.append(str(dga_build/'BuildInfo.cs'))
    # The original build script's explicitly declared production source list,
    # including its generated exact helper identity.
    build=(extra/'Plugin/build.ps1').read_text()
    block=build.split('$sources = @(',1)[1].split('\n)',1)[0]
    for owner,rel in re.findall(r"\(Join-Path \$(here|workspace) '([^']+)'\)",block):
        if rel=='AssemblyInfo.cs':continue
        sources.append(str((extra/'Plugin' if owner=='here' else extra)/rel.replace('\\','/')))
    sources.append(str(extra/'Evidence/ExpectedRuntime.cs'))
    for name in ['PracticeAction.cs','RuntimeFiles.cs','Diagnostics.cs']:
        sources.append(str(practice/'Plugin'/name))
    sources.append(str(practice/'Evidence/ExpectedRuntime.cs'))
    sources.extend(str(ROOT/'Core'/name) for name in ['CotConfiguration.cs','CotHost.cs'])
    sources.append(emit('ProductInfo.cs','namespace DynastyCOT { internal static class ProductInfo { internal const string Version='+json.dumps(product['version'])+'; internal const string AssemblyVersion='+json.dumps(product['version'].split('-')[0]+'.0')+'; }}\n'))
    resource_files=[
        (dga_build/('DynamicGamedayAtmosphereNative-'+atmosphere_version+'.dll'),'Attendance.DynamicGamedayAtmosphereNative-'+atmosphere_version+'.dll'),
        (dga/'Source/DynamicGamedayAtmosphere.ini','Attendance.DynamicGamedayAtmosphere.ini'),
        (dga_build/'DgaDiagnostics.exe','Attendance.DgaDiagnostics.exe')]
    for file,name in resource_files:entries.append(dict(module='GamedayAtmosphere',path='resource:'+name,sha256=digest(file)))
    cot_manifest=emit('cot-payload.txt',''.join(e['module']+'|'+e['sha256']+'|'+e['path']+'\n' for e in entries))
    resources=[f'/resource:{file},{name}' for file,name in resource_files]+[
        f'/resource:{de_manifest},DynastyEnhancer.payload-sha256.txt',f'/resource:{cot_manifest},DynastyCOT.payload.txt']
    dll=stage/'Plugins/DynastyCOT.dll';dll.parent.mkdir(parents=True,exist_ok=True)
    compiler=Path(r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe')
    command=[str(compiler),'/nologo','/target:library','/platform:x64','/optimize+',
        '/define:COMBINED_DYNASTY_SETTINGS;DIRECT_RUNTIME_LOADING','/out:'+str(dll),
        '/reference:'+str(sdk/'FrostyCore.dll'),'/reference:'+str(sdk/'FrostySdk.dll'),
        '/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/reference:System.IO.Compression.FileSystem.dll']
    subprocess.run(command+resources+sources,check=True)
    entries.append(dict(module='Core',path='Plugins/DynastyCOT.dll',sha256=digest(dll)))
    settings=stage/'Templates/settings.json';settings.parent.mkdir()
    settings.write_text(json.dumps(dict(format=1,modules={id:True for id in [
        'DynastyEnhancer','GamedayAtmosphere','ExtraDynastySettings','DrivePractice']}),indent=2)+'\n')
    entries.append(dict(module='Core',path='Templates/settings.json',sha256=digest(settings)))
    ownership=stage/'Templates/owned-payloads.json'
    embedded_doc=dict(files=[],embedded=[e for e in entries if e['path'].startswith('resource:')])
    ownership.write_text(json.dumps(dict(format=1,current=list(owned_entries(embedded_doc)),
        legacy=json.loads((ROOT/'Packaging/LegacyPayloads.json').read_text())['files']),indent=2)+'\n')
    entries.append(dict(module='Core',path='Templates/owned-payloads.json',sha256=digest(ownership)))
    manifest=dict(format=1,product='DynastyCOT',version=product['version'],nodeVersion=node_version,
        createdUtc=stamp,files=[e for e in entries if not e['path'].startswith('resource:')],
        embedded=[e for e in entries if e['path'].startswith('resource:')],liveAcceptance='pending',socialRenderMode=inputs['socialRenderMode'],analyticsBridge=dict(abi=1,buildId=analytics_build),
        nativeInputs={key:dict(path=path,sha256=digest(ROOT/path)) for key,path in [('social',a.social_native),('fog',a.fog_native)]},
        sourceInputs={str(Path(f).relative_to(ROOT)):digest(Path(f)) for f in sources if Path(f).is_relative_to(ROOT)})
    (stage/'package-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    metadata=dict(stage=str(stage),work=str(work),dll=str(dll),manifest=str(stage/'package-manifest.json'),status='staged; installer and package acceptance pending')
    (ROOT/'Build/current-package.json').write_text(json.dumps(metadata,indent=2)+'\n')
    print('Staged private COT host: '+str(stage))
    print('One adapter compiled; no installation or publishing performed.')
if __name__=='__main__':main()
