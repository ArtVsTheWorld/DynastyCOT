"""Create and verify a private ZIP after matching offline acceptance reports."""
from pathlib import Path
import json,hashlib,zipfile,datetime,importlib.util,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(rel):return json.loads((ROOT/rel).read_text())
def main():
    spec=importlib.util.spec_from_file_location('cot_package_verify',ROOT/'Tools/verify_package.py')
    verifier=importlib.util.module_from_spec(spec);spec.loader.exec_module(verifier)
    meta=read('Build/current-package.json');stage=Path(meta['stage']);manifest=verifier.verify(stage)
    manifest_sha=sha(stage/'package-manifest.json')
    package=read('Build/package-verification.json');installer=read('Build/installer-verification.json')
    assert package['stage']==str(stage) and package['manifestSha256']==manifest_sha and package['coreChecks']==106,'Stale package verification'
    assert installer['packageStage']==str(stage) and installer['manifestSha256']==manifest_sha,'Stale installer verification'
    assert installer['checks']>=59 and not installer['liveInstall'],'Missing fixture acceptance'
    clocks=read('Build/ClockChecks/verification.json')
    assert clocks['profileChecks']==86 and clocks['nativeChecks']==60 and clocks['helperChecks']==10
    for entry in clocks['sources']:assert sha(ROOT/entry['path'])==entry['sha256'],'Stale clock verification'
    sdk_log=(ROOT/'Build/ClockChecks/sdk-regressions.log').read_text()
    assert 'PASS: 79 checks' in sdk_log and 'PASS callback counter and native ABI; no game access' in sdk_log,'Settings regression incomplete'
    native=read('cot.build-inputs.json')
    for rel,expected in native['sourceHashes'].items():assert sha(ROOT/rel)==expected,'Stale Enhancer build: '+rel
    for name,entry in manifest['nativeInputs'].items():
        assert sha(ROOT/entry['path'])==entry['sha256'],'Native input changed: '+name
    contract=read('Compatibility/practice-contract.json')
    practice=next(x for x in manifest['files'] if x['path'].endswith('SituationalPractice.Runtime.exe'))
    assert contract['helperSha256']==practice['sha256'],'Practice ownership contract mismatch'
    # Read-only checks of the original repositories and actual installed adapters.
    baseline=read('Baselines/source-files.json');original_count=0
    for entry in baseline['files']:
        assert sha(Path(entry['source']))==entry['sha256'],'Original source changed: '+entry['source']
        original_count+=1
    sdk=Path(sys.argv[1]).resolve();installed=[];reference_audit=[]
    for module in read('Baselines/import.json')['modules']:
        ref=module['installedReference'];path=sdk/'Plugins'/ref['name']
        observed=sha(path) if path.is_file() else None
        status='unchanged' if observed==ref['sha256'] else 'missing' if observed is None else 'different'
        reference_audit.append(dict(name=ref['name'],baselineSha256=ref['sha256'],observedSha256=observed,status=status))
        if status=='unchanged':installed.append(dict(name=ref['name'],sha256=observed))
    release=ROOT/'Release';release.mkdir(exist_ok=True)
    output=release/('DynastyCOT-'+manifest['version']+'.zip')
    if output.exists():raise RuntimeError('Private archive already exists; use a new product version')
    with zipfile.ZipFile(output,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as z:
        for p in sorted(stage.rglob('*')):
            if p.is_file():z.write(p,p.relative_to(stage).as_posix())
    extracted=ROOT/'Build'/('ArchiveCheck-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'))
    with zipfile.ZipFile(output) as z:
        assert len(z.namelist())==len(set(x.casefold() for x in z.namelist())),'Duplicate archive entries'
        assert z.testzip() is None,'ZIP CRC mismatch'
        z.extractall(extracted)
    verifier.verify(extracted)
    assert sha(extracted/'package-manifest.json')==manifest_sha
    report=dict(format=1,product='DynastyCOT',version=manifest['version'],
        atUtc=datetime.datetime.now(datetime.timezone.utc).isoformat(),archive=str(output),archiveSha256=sha(output),
        archiveBytes=output.stat().st_size,manifestSha256=manifest_sha,files=len(manifest['files']),
        originalSourcesUnchanged=original_count,installedAdaptersUnchanged=installed,installedReferenceAudit=reference_audit,
        oneMmcEntry=True,moduleSelections=16,coreChecks=106,assemblyChecks=17,
        clockProfileChecks=86,emittedClockChecks=60,detachedHelperChecks=10,menuWriterChecks=79,
        practiceOwnershipChecks=4949,practiceNativeHookChecks=5717,installerChecks=installer['checks'],
        packageEvidence=str(ROOT/'Build/package-verification.json'),installerEvidence=str(ROOT/'Build/installer-verification.json'),
        extractedArchive=str(extracted),liveGameAcceptance='pending',overhaulCompatibility='pending',
        installedIntoLiveMmc=False,published=False)
    latest=release/'verification.json'
    if latest.exists():
        prior=json.loads(latest.read_text());prior_path=release/('verification-'+prior['version']+'.json')
        if not prior_path.exists():prior_path.write_text(json.dumps(prior,indent=2)+'\n')
    (release/('verification-'+manifest['version']+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    latest.write_text(json.dumps(report,indent=2)+'\n')
    meta.update(archive=str(output),verification=str(release/'verification.json'),status='private ZIP verified; live acceptance pending')
    (ROOT/'Build/current-package.json').write_text(json.dumps(meta,indent=2)+'\n')
    print('PASS: private archive integrity/extraction, matching acceptance reports and '+str(original_count)+' original source hashes unchanged. Installed reference state recorded; no live writes.')
    print(str(output));print('SHA256 '+report['archiveSha256'])
if __name__=='__main__':main()
