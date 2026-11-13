import fs from 'node:fs/promises';
import path from 'node:path';
import {setTimeout} from 'node:timers/promises';
import {atomicJsonFile} from './history_store.mjs';
// One-shot native approval, not a resident worker/cache. No history changes
// occur before the native owner re-verifies the entire consumed-byte proof.
export async function awaitPublicationApproval(gate,output,identity,{timeoutMs=60000}={}){
 if(!gate)return;
 const root=path.resolve(path.dirname(output)),file=path.resolve(gate),request=path.basename(file);
 if(path.dirname(file)!==root||!request.startsWith(path.basename(output)+'.gate-')||!/\.gate-\d+-\d+-\d+$/.test(request))throw Error('Invalid publication approval path');
 await atomicJsonFile(file+'.ready',{format:1,request,identity});
 const began=Date.now();
 while(Date.now()-began<timeoutMs){
  try{
   const approval=JSON.parse(await fs.readFile(file+'.allow','utf8'));
   if(approval.format!==1||approval.request!==request||approval.approved!==true)throw Error('Publication approval refused');
   return;
  }catch(e){if(e.code!=='ENOENT')throw e;}
  await setTimeout(25);
 }
 throw Error('Publication approval timed out; history unchanged');
}
