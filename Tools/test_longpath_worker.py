from pathlib import Path
import json,subprocess,zipfile,os,datetime,hashlib
r=Path.cwd();w=r/'Build/LongPathAudit'/('Worker-'+datetime.datetime.now().strftime('%H%M%S'));w.mkdir();t=json.load(open(r/'Build/LongPathAudit/native-toolchain.json'));exe=w/'LongPathWorkerChecks.exe';subprocess.run([t['cl'],'/nologo','/O2','/MT','/W4','/WX','/EHsc','/std:c++17','/utf-8',*[f'/I{x}' for x in t['includes']],str(r/'Tests/LongPathWorkerChecks.cpp'),'/Fe:'+str(exe),'/link',*[f'/LIBPATH:{x}' for x in t['libs']]],check=True,cwd=w);m=json.load(open(r/'Build/LongPathAudit/BeyondTheField-candidate.json'));results=[]
for suffix in ['short','l'*120+'/'+'m'*120]:
 f=w/suffix;os.makedirs('\\\\?\\'+str(f));node=f/'node.exe'
 with zipfile.ZipFile(Path(m['work'])/'payload.zip') as z:data=z.read('COT-Runtime/Shared/node.exe')
 assert hashlib.sha256(data).hexdigest()==m['payload']['COT-Runtime/Shared/node.exe'];Path('\\\\?\\'+str(node)).write_bytes(data)
 script=f/'export.mjs';Path('\\\\?\\'+str(script)).write_text("import fs from 'node:fs';const [input,output]=process.argv.slice(2);fs.writeFileSync(output,fs.readFileSync(input));",encoding='utf8');config=f/'input.json';Path('\\\\?\\'+str(config)).write_text('{"history":"preserved words"}',encoding='utf8');out=f/'output.json';p=subprocess.run([str(exe),str(node),str(script),str(config),str(out)],capture_output=True,text=True,timeout=30);print(p.stdout+p.stderr,flush=True);results.append(dict(length=len(str(node)),code=p.returncode,result=p.stdout+p.stderr));assert Path('\\\\?\\'+str(out)).read_text()=='{"history":"preserved words"}'
(r/'Build/LongPathAudit/worker-results.json').write_text(json.dumps(dict(work=str(w),results=results),indent=2));assert all(x['code']==0 for x in results)
