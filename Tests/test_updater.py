"""Credential-free updater: isolated HTTP discovery through transactional rollback.

No live installation or public API calls. Uses the current verified source ZIP;
--baseline exercises the same path against the frozen comparison ZIP while the
candidate is being built. Its report is explicitly marked baseline-only.
"""
from pathlib import Path
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import argparse, datetime, hashlib, io, json, subprocess, threading, zipfile
ROOT=Path(__file__).resolve().parents[1]

def sha(b):return hashlib.sha256(b).hexdigest()

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--candidate-only',action='store_true',help='Exercise current payload only; does not prove cross-version rollback');parser.add_argument('--baseline',action='store_true');parser.add_argument('--published',metavar='EXACT_VERSION',help='Read anonymous GitHub; all installation writes stay in an isolated marked fixture');args=parser.parse_args()
    meta=json.loads((ROOT/'Build/current-package.json').read_text())
    archive=Path(meta['archive'])
    if args.baseline:archive=ROOT/'Release/DynastyCOT-0.1.5-private.zip'
    version=json.loads(zipfile.ZipFile(archive).read('package-manifest.json'))['version']
    older=ROOT/'Release/DynastyCOT-0.1.12-private.zip'
    if not older.exists():older=ROOT/'Release/DynastyCOT-0.1.11-private.zip'
    if not older.exists():older=ROOT/'Release/DynastyCOT-0.1.10-private.zip'
    if not older.exists():older=ROOT/'Release/DynastyCOT-0.1.8-private.zip'
    if not older.exists():older=ROOT/'Release/DynastyCOT-0.1.6-private.zip'
    if not older.exists():older=ROOT/'Release/DynastyCOT-0.1.5-private.zip'
    # An older published ZIP is retained locally for the cross-version fixture.
    # Fresh checkouts without it use current twice; the report states that limit.
    if not older.exists():older=archive
    if args.candidate_only:older=archive
    oldversion=json.loads(zipfile.ZipFile(older).read('package-manifest.json'))['version']
    base=ROOT/'Build'/('UpdaterChecks-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f'));base.mkdir()
    fixture=base/"MMC's updater & fixture";fixture.mkdir();(fixture/'.cot-fixture').write_text('isolated');(fixture/'MMCModManager.exe').write_text('fixture marker, never executed');(fixture/'Plugins').mkdir();(fixture/'Plugins/OtherOverhaul.dll').write_text('unrelated overhaul')
    payload=archive.read_bytes();oldpayload=older.read_bytes();requests=[];origin=''
    notes='Candidate notes: public test prerelease. Updates remain optional.';compat='Live acceptance pending; no startup-only compatibility claim.'
    def metadata(data=payload,v=version):
        return dict(format=1,product='DynastyCOT',version=v,releaseNotes=notes,releaseNotesSha256=sha(notes.encode()),compatibilityNotes=compat,compatibilityNotesSha256=sha(compat.encode()),archive=dict(name=f'DynastyCOT-{v}.zip',url=origin+f'/download/{v}/DynastyCOT-{v}.zip',bytes=len(data),sha256=sha(data)))
    badzip={}
    def zipbytes(entries):
        stream=io.BytesIO()
        with zipfile.ZipFile(stream,'w',zipfile.ZIP_DEFLATED) as z:
            for name,data in entries:z.writestr(name,data)
        return stream.getvalue()
    badzip['traversal']=zipbytes([('../escape.txt','x')])
    badzip['duplicate']=zipbytes([('Install.ps1','x'),('INSTALL.PS1','y')])
    badzip['missing-manifest']=zipbytes([('Install.ps1','x')])
    for case,manifestversion,extra,filehash in [('manifest-version','99.0.0',False,sha(b'x')),('payload-hash',version,False,'0'*64),('unlisted-payload',version,True,sha(b'x'))]:
        manifest=dict(product='DynastyCOT',version=manifestversion,files=[dict(path='Install.ps1',sha256=filehash)])
        badzip[case]=zipbytes([('Install.ps1','x'),('package-manifest.json',json.dumps(manifest))]+([('unexpected.exe','x')] if extra else []))
    badzip['not-zip']=b'not a ZIP archive'
    failures=['version','product','format','notes','compatibility','http-external','filename','size','checksum','wrong-size','truncated','download-missing','redirect-outside',*badzip]
    class Handler(BaseHTTPRequestHandler):
        def log_message(self,*a):pass
        def send(self,code,data=b'',headers=None):
            self.send_response(code)
            for k,v in (headers or {}).items():self.send_header(k,v)
            self.send_header('Content-Length',str(len(data)));self.end_headers();self.wfile.write(data)
        def do_GET(self):
            requests.append(self.path)
            if self.path in ['/forbidden','/missing','/server-error']:return self.send({'/forbidden':403,'/missing':404,'/server-error':500}[self.path])
            if self.path=='/invalid-json':return self.send(200,b'invalid JSON')
            if self.path=='/redirect-outside':return self.send(302,headers={'Location':'http://example.com/payload'})
            if self.path=='/redirect-loop':return self.send(302,headers={'Location':'/redirect-loop'})
            if self.path=='/releases':
                rows=[]
                for i,v in enumerate([version,oldversion]):rows.append(dict(draft=False,tag_name='v'+v,name=v,body='public notes',published_at='2026-11-0'+str(7-i)+'T12:00:00Z',html_url=origin+'/release/'+v,prerelease=i==0,assets=[dict(name='cot-update.json',browser_download_url=origin+'/meta/'+('current' if i==0 else 'old'))]))
                rows.append(dict(draft=True));rows.append(dict(draft=False,tag_name='v0.0.1',assets=[]))
                return self.send(200,json.dumps(rows).encode())
            if self.path.startswith('/meta/'):
                case=self.path.split('/')[-1];m=metadata(oldpayload,oldversion) if case=='old' else metadata()
                if case in badzip:m=metadata(badzip[case]);m['archive']['url']=origin+'/bad/'+case+'/DynastyCOT-'+version+'.zip'
                if case=='version':m['version']='88.0.0'
                if case=='product':m['product']='OtherProduct'
                if case=='format':m['format']=2
                if case=='notes':m['releaseNotes']+=' altered'
                if case=='compatibility':m['compatibilityNotes']+=' altered'
                if case=='http-external':m['archive']['url']='http://example.com/DynastyCOT-'+version+'.zip'
                if case=='filename':m['archive']['name']='other.zip'
                if case=='size':m['archive']['bytes']=1024**3
                if case=='checksum':m['archive']['sha256']='0'*64
                if case=='wrong-size':m['archive']['bytes']+=1
                if case in ['truncated','download-missing','redirect-outside']:m['archive']['url']=origin+'/'+case+'/DynastyCOT-'+version+'.zip'
                return self.send(200,json.dumps(m).encode())
            if self.path.startswith('/download-missing/'):return self.send(404)
            if self.path.startswith('/redirect-outside/'):return self.send(302,headers={'Location':'http://example.com/payload'})
            if self.path.startswith('/truncated/'):
                self.send_response(200);self.send_header('Content-Length',str(len(payload)));self.end_headers();self.wfile.write(payload[:20]);self.close_connection=True;return
            if self.path.startswith('/bad/'):return self.send(200,badzip[self.path.split('/')[2]])
            if self.path.startswith('/download/'):
                return self.send(200,oldpayload if self.path.split('/')[2]==oldversion else payload)
            return self.send(404)
    server=ThreadingHTTPServer(('127.0.0.1',0),Handler);origin=f'http://127.0.0.1:{server.server_port}'
    thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start()
    config=base/'config.json';config.write_text(json.dumps(dict(origin=origin,version=version,oldVersion=oldversion,failures=failures)))
    info=base/'SetupInfo.cs';info.write_text('namespace DynastyCOT.Setup {internal static class SetupInfo {internal const string Version='+json.dumps(version)+';internal const string PayloadSha256='+json.dumps(sha(payload))+';}}')
    exe=base/'UpdaterChecks.exe';compiler=r'C:\Windows\Microsoft.NET\Framework64\v4.0.30319\csc.exe'
    subprocess.run([compiler,'/nologo','/platform:x64','/out:'+str(exe),'/reference:System.Web.Extensions.dll','/reference:System.IO.Compression.dll','/resource:'+str(archive)+',DynastyCOT.Package.zip',str(info),str(ROOT/'Packaging/Setup/InstallerEngine.cs'),str(ROOT/'Packaging/Setup/UpdateClient.cs'),str(ROOT/'Tests/UpdaterChecks.cs')],check=True)
    try:
        command=[str(exe),'--published',str(fixture),str(base),args.published] if args.published else [str(exe),str(fixture),str(base),str(config)]
        result=subprocess.run(command,capture_output=True,text=True,timeout=300)
        (base/'checks.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr
        report=json.loads((base/'result.json').read_text());report.update(baselineOnly=args.baseline,candidateOnly=args.candidate_only,crossVersion=version!=oldversion,requests=len(requests),fixtureRoot=str(base),archiveSha256=sha(payload),failureCases=failures)
        (ROOT/('Build/updater-candidate-only-verification.json' if args.candidate_only else 'Build/updater-published-verification.json' if args.published else 'Build/updater-verification.json')).write_text(json.dumps(report,indent=2)+'\n')
        print(result.stdout.strip());print('Version',version,'older',oldversion,'requests',len(requests),'baselineOnly',args.baseline)
    finally:server.shutdown();server.server_close()
if __name__=='__main__':main()
