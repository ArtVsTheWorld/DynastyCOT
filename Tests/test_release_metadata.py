"""Release metadata provenance and sanitized public-evidence regression checks."""
from pathlib import Path
import datetime,hashlib,importlib.util,json,subprocess
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
def load(name,path):
    spec=importlib.util.spec_from_file_location(name,path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module

def main():
    base=ROOT/'Build'/('ReleaseToolsChecks-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));base.mkdir()
    for folder in ['Build','Release','docs/release']:(base/folder).mkdir(parents=True)
    version='1.2.3-test';archive=base/'Release'/f'DynastyCOT-{version}.zip';archive.write_bytes(b'synthetic unit archive');exe=base/'Release/setup.exe';exe.write_bytes(b'synthetic unit setup')
    sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();write=lambda p,d:(base/p).write_text(json.dumps(d))
    product=dict(version=version,distribution=dict(repository='ArtVsTheWorld/DynastyCOT',visibility='public',compatibilityNotes='Live game acceptance pending'))
    write('cot.product.json',product);(base/f'docs/release/{version}.md').write_text('Public test notes')
    verification=base/'Build/verified.json';verification.write_text(json.dumps(dict(version=version,archiveSha256=sha(archive),archiveBytes=archive.stat().st_size,files=12,coreChecks=50,liveGameAcceptance='pending',fixtureRoot='C:\\Users\\PRIVATE_USER\\checkpoint')))
    write('Build/current-package.json',dict(archive=str(archive),verification=str(verification)))
    write('Build/current-setup.json',dict(version=version,exe=str(exe),sha256=sha(exe)))
    setup=dict(version=version,uiChecks=29,liveInstall=False,fixtureRoundTrip=True,fixtureRoot='C:\\PRIVATE_USER',scaling=[100,125,150,{'path':'C:\\PRIVATE_USER\\capture.png'},'PRIVATE_USER'])
    updater=dict(version=version,oldVersion='1.2.2-test',checks=95,liveInstall=False,baselineOnly=False,crossVersion=True,failureCases=['bad-hash'],fixtureRoot='C:\\PRIVATE_USER')
    write('Build/setup-verification.json',setup);write('Build/updater-verification.json',updater)
    write('Build/updater-published-verification.json',dict(version=version,archiveSha256=sha(archive),anonymousGitHub=True,liveInstall=False,checks=8,fixtureRoot='C:\\PRIVATE_USER',secret='PRIVATE_USER'))
    metadata=load('metadata',ROOT/'Tools/create_update_metadata.py');metadata.ROOT=base;exporter=load('exporter',ROOT/'Tools/create_release_evidence.py');exporter.ROOT=base
    checks=0
    def check(condition,message):
        nonlocal checks
        assert condition,message;checks+=1
    def reject(call,message):
        try:call()
        except ValueError:return check(True,message)
        check(False,message)
    metadata.main();doc=json.loads((base/'Release/cot-update.json').read_text());check(doc['archive']['sha256']==sha(archive),'archive hash');check(doc['releaseNotesSha256']==hashlib.sha256(b'Public test notes').hexdigest(),'notes pin');check(doc['archive']['url'].endswith('/v'+version+'/'+archive.name),'versioned public URL')
    with patch.object(exporter.subprocess,'check_output',side_effect=['a'*40+'\n','']):exporter.main()
    public=(base/f'Release/release-evidence-{version}.json').read_text();result=json.loads(public);check('PRIVATE_USER' not in public and str(base) not in public,'private paths leaked');check(result['setup']['layoutScaleCaptures']==[100,125,150],'unsafe layout objects leaked');check(result['publishedUpdater']['checks']==8,'published evidence missing');check(result['sourceRevision']=='a'*40 and result['sourceTreeClean'],'source provenance');check(result['liveGameAcceptance']=='pending','live acceptance mislabeled')
    sums=(base/f'Release/SHA256SUMS-{version}.txt').read_text();check(len(sums.splitlines())==5,'asset checksum set incomplete')
    archive.write_bytes(b'changed');reject(metadata.main,'changed finalized ZIP accepted');archive.write_bytes(b'synthetic unit archive')
    updater['baselineOnly']=True;write('Build/updater-verification.json',updater)
    with patch.object(exporter.subprocess,'check_output',side_effect=['a'*40+'\n','']):reject(exporter.main,'baseline-only checks exported')
    updater['baselineOnly']=False;updater['version']='0.0.1';write('Build/updater-verification.json',updater)
    with patch.object(exporter.subprocess,'check_output',side_effect=['a'*40+'\n','']):reject(exporter.main,'stale candidate checks exported')
    product['distribution']['repository']='invalid repo';write('cot.product.json',product);reject(metadata.main,'invalid repository accepted')
    print('PASS:',checks,'release metadata/provenance/privacy checks; synthetic Build fixtures only.')
if __name__=='__main__':main()
