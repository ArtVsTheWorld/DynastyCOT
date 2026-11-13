// Immutable edition blobs plus an atomic index. Original monolithic archives
// remain available; migration never overwrites them or rewrites past wording.
import fs from 'node:fs/promises';
import path from 'node:path';
import {createHash} from 'node:crypto';
const REFERENCE=Symbol('edition archive reference');
const hash=text=>createHash('sha256').update(text).digest('hex');
const indexFile=file=>file.replace(/\.json$/,'.index.json');
const recruitLedgerFile=file=>file.replace(/\.json$/,'.recruit-games.json');
const blobPath=(dir,name)=>{if(!/^(feed|snapshot|carry)-[a-f0-9]{64}\.json$/.test(name))throw Error('Invalid archive blob reference');return path.join(dir,'.history-data',name);};
async function boundedMap(values,limit,fn){const result=Array(values.length);let next=0;await Promise.all(Array.from({length:Math.min(limit,values.length)},async()=>{for(;;){const i=next++;if(i>=values.length)return;result[i]=await fn(values[i]);}}));return result;}
async function optionalJson(file){try{return JSON.parse(await fs.readFile(file,'utf8'));}catch(e){if(e.code==='ENOENT')return null;throw e;}}
async function recruitLedger(file,state){
 const ledger=await optionalJson(recruitLedgerFile(file)),indexed=state?.recruitGameLedger;
 if(!ledger)return indexed;
 if(ledger.format!==1||typeof ledger.scope!=='string'||!ledger.records||typeof ledger.records!=='object'||Array.isArray(ledger.records))throw Error('Invalid recruit game ledger sidecar');
 const scope=state?.editions?.[0]?.scope?.split('/').slice(0,3).join('|');
 if(scope&&scope!==ledger.scope||indexed&&indexed.scope!==ledger.scope)throw Error('Recruit game ledger belongs to another Dynasty');
 return {...ledger,records:{...ledger.records,...(indexed?.records??{})}};
}
async function blob(dir,name){const file=blobPath(dir,name),text=await fs.readFile(file,'utf8');if(hash(text)!==name.slice(name.indexOf('-')+1,-5))throw Error('Archive blob checksum mismatch');return JSON.parse(text);}
export async function atomicJsonFile(file,data){const tmp=`${file}.${process.pid}.tmp`;try{await fs.writeFile(tmp,JSON.stringify(data));await fs.rename(tmp,file);}finally{await fs.rm(tmp,{force:true}).catch(()=>{});}}
export async function readEditionHistory(file,{activeWeek=null,archiveOnly=false}={}){
 const index=await optionalJson(indexFile(file));
 if(!index){const old=await optionalJson(file);if(old&&(old.format!==1||!Array.isArray(old.editions)))throw Error('Invalid runtime history');if(old&&!archiveOnly){const ledger=await recruitLedger(file,old);if(ledger)old.recruitGameLedger=ledger;}return old;}
 if(index.format!==2||!Array.isArray(index.editions)||index.editions.length>48)throw Error('Invalid edition archive index');
 const dir=path.dirname(file),keys=new Set();
 for(const meta of index.editions){if(typeof meta.key!=="string"||keys.has(meta.key))throw Error("Invalid edition archive identity");keys.add(meta.key);}
 keys.clear();
 const previous=Number.isInteger(activeWeek)?index.editions.filter(e=>!e.reconstructed&&e.snapshotFile&&e.week<activeWeek).sort((a,b)=>b.week-a.week)[0]?.key:null;
 const latest=index.editions.filter(e=>e.snapshotFile).sort((a,b)=>b.week-a.week)[0]?.key;
 const editions=[];
 const editionsData=await boundedMap(index.editions,4,async meta=>{
  if(typeof meta.key!=='string'||keys.has(meta.key)||typeof meta.scope!=='string'||!Number.isInteger(meta.week))throw Error('Invalid edition archive identity');keys.add(meta.key);
  const feed=await blob(dir,meta.feedFile);if(feed.key!==meta.key||feed.week!==meta.week||meta.scope!==`${feed.leagueId}/${feed.userTeamIndex}/${feed.season}/${feed.phase}`)throw Error('Edition blob/index identity mismatch');
  const full=!archiveOnly&&(!Number.isInteger(activeWeek)||meta.week===activeWeek||meta.key===previous);
  const snapshot=meta.snapshotFile?(full?await blob(dir,meta.snapshotFile):{...meta.snapshotSummary,...(archiveOnly&&meta.key===latest&&meta.carryFile?await blob(dir,meta.carryFile):{})}):undefined;
  const entry={scope:meta.scope,week:meta.week,key:meta.key,...(meta.reconstructed?{reconstructed:true}:{}),...(snapshot?{snapshot}:{}),feed};
  entry[REFERENCE]={dir,meta,full};return entry;
 });
 editions.push(...editionsData);
 const ledger=archiveOnly?null:await recruitLedger(file,index);
 if(index.sharedSchools!==undefined&&(!Array.isArray(index.sharedSchools)||index.sharedSchools.length>255||index.sharedSchools.some(id=>!Number.isInteger(id)||id<0||id>=255)))throw Error('Invalid shared-school archive references');
 return {format:1,editions,sharedSchools:index.sharedSchools??[],authors:index.authors??{},...(ledger?{recruitGameLedger:ledger}:{})};
}
async function writeBlob(dir,kind,data,verifiedName=null){
 const text=JSON.stringify(data),name=`${kind}-${hash(text)}.json`,file=blobPath(dir,name);
 if(name===verifiedName)return name; // Already checksum-verified on this read; exact serialized content unchanged.
 try{const old=await fs.readFile(file,'utf8');if(old!==text)throw Error('Existing archive blob differs from its checksum');}
 catch(e){if(e.code!=='ENOENT')throw e;const tmp=`${file}.${process.pid}.tmp`;try{await fs.writeFile(tmp,text);await fs.rename(tmp,file);}finally{await fs.rm(tmp,{force:true}).catch(()=>{});}}
 return name;
}
export async function writeEditionHistory(file,state){
 if(state.format!==1||!Array.isArray(state.editions)||state.editions.length>48)throw Error('Invalid runtime history for archive write');
 const dir=path.dirname(file);await fs.mkdir(path.join(dir,'.history-data'),{recursive:true});const editions=[];
 for(const entry of state.editions){
  const reference=entry[REFERENCE],prior=reference?.meta;
  if(reference&&!reference.full&&reference.dir!==dir)throw Error('A partial snapshot cannot migrate to another archive');
  const reuse=reference&&reference.dir===dir&&prior.key===entry.key;
  const snapshotFile=entry.snapshot?(reuse&&!reference.full?prior.snapshotFile:await writeBlob(dir,'snapshot',entry.snapshot,reuse?prior.snapshotFile:null)):null;
  const summary=entry.snapshot?Object.fromEntries(['sha256','recentDraftClass'].filter(k=>entry.snapshot[k]!==undefined).map(k=>[k,entry.snapshot[k]])):null;
  const carryFile=entry.snapshot?(reuse&&!reference.full?prior.carryFile:await writeBlob(dir,'carry',Object.fromEntries(['milestoneTotals','postseasonPublications','seasonEndPublications'].filter(k=>entry.snapshot[k]!==undefined).map(k=>[k,entry.snapshot[k]])))):null;
  editions.push({scope:entry.scope,week:entry.week,key:entry.key,reconstructed:entry.reconstructed===true,feedFile:await writeBlob(dir,'feed',entry.feed,reuse?prior.feedFile:null),snapshotFile,carryFile,snapshotSummary:summary});
 }
 // Older releases rewrite their known index fields. A separately scoped,
 // atomic sidecar survives that writer and restores modeled records after a
 // later upgrade. Existing edition blobs and legacy history stay untouched.
 if(state.recruitGameLedger)await atomicJsonFile(recruitLedgerFile(file),state.recruitGameLedger);
 await atomicJsonFile(indexFile(file),{format:2,authors:state.authors??{},sharedSchools:state.sharedSchools??[],editions,...(state.recruitGameLedger?{recruitGameLedger:state.recruitGameLedger}:{})});
}
