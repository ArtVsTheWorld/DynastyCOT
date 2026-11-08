"""Exercise the shipped setup EXE's real WinForms buttons using fixtures only."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def snapshot(root):return {p.relative_to(root).as_posix():sha(p) for p in root.rglob('*') if p.is_file() and 'Backups' not in p.parts}
def main():
    meta=json.loads((ROOT/'Build/current-setup.json').read_text());exe=Path(meta['exe'])
    assert sha(exe)==meta['sha256']
    base=ROOT/'Build'/('SetupChecks-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));base.mkdir()
    # Apostrophes, spaces and shell punctuation must remain literal folder names.
    root=base/"MMC's COT & folder";root.mkdir();(root/'.cot-fixture').write_text('isolated')
    def put(path,text):
        p=root/path;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(text)
    put('MMCModManager.exe','fixture marker, never executed')
    put('Plugins/DynastyCOT.dll','previous COT adapter')
    put('Plugins/DynastyCOT/settings.json','{"format":1,"modules":{"DrivePractice":false}}')
    put('Plugins/OtherOverhaul.dll','unrelated overhaul')
    put('Plugins/DynastyEnhancer.dll','previous standalone adapter')
    put('Plugins/DynastyEnhancer/Runtime-v103/RuntimeHistory/101/news.json','news history')
    before=snapshot(root);report=base/'ui-result.json'
    result=subprocess.run([str(exe),'--verify-payload'],capture_output=True,text=True,timeout=60)
    (base/'payload.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr
    result=subprocess.run([str(exe),'--smoke-test',str(root),str(base),str(report)],capture_output=True,text=True,timeout=240)
    (base/'ui.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr
    ui=json.loads(report.read_text());assert ui['checks']>=29 and not ui['liveInstall']
    assert snapshot(root)==before,'GUI install/rollback changed original fixture files'
    result=subprocess.run([str(exe),'--smoke-test',str(base/'not-a-fixture'),str(base),str(base/'rejected.json')],capture_output=True,text=True,timeout=20)
    assert result.returncode!=0 and not (base/'not-a-fixture').exists(),'unsafe fixture accepted'
    layout=base/'layout'
    result=subprocess.run([str(exe),'--layout-test',str(root),str(base),str(layout)],capture_output=True,text=True,timeout=90)
    (base/'layout.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr
    import struct
    dimensions={}
    layouts=json.loads((layout/'layout-report.json').read_text());assert len(layouts)==5
    for item in layouts:
        assert item['width']<=item['workingWidth']-32 and item['height']<=item['workingHeight']-32
        assert item['footerVisible'] and item['updateActionsVisible'] and item['allModulesReachable']
        for page in ['install','updates','backups','activity']:
            image=layout/f"setup-{page}-{item['suffix']}.png";data=image.read_bytes();assert data[:8]==b'\x89PNG\r\n\x1a\n'
            dimensions[image.name]=struct.unpack('>II',data[16:24]);assert dimensions[image.name]==(item['width'],item['height'])
    assert {x['workingHeight'] for x in layouts if '-work' in x['suffix']}=={1080,1152}
    assert snapshot(root)==before,'layout capture changed fixture files'
    report_data=dict(version=meta['version'],exe=str(exe),setupSha256=sha(exe),payloadSha256=meta['payloadSha256'],
        uiChecks=ui['checks'],payloadVerified=True,fixtureRoundTrip=True,unsafeFixtureRejected=True,
        boundedWorkingAreas=[1080,1152],footerAndUpdateActionsVisible=True,allModulesReachable=True,layoutCaptures=dimensions,layoutDirectory=str(layout),scaling=[100,125,150],visualReview='Separate manual review of captures required',quoteAndSpacePaths=True,selectedAndFullUninstall=True,managerFolderDetection=True,fixtureRoot=str(base),liveInstall=False,sourceHashes=meta['sources'])
    (ROOT/'Build/setup-verification.json').write_text(json.dumps(report_data,indent=2)+'\n')
    (ROOT/'Release/setup-verification.json').write_text(json.dumps(report_data,indent=2)+'\n')
    print('PASS: single-file setup payload,',ui['checks'],'real GUI/engine checks, selected/all uninstall and rollback, Mod Manager detection, quoted paths and unsafe fixture rejection. No live installation.')
if __name__=='__main__':main()
