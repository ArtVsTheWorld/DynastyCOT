import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs/promises';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import {spawn} from 'node:child_process';
import {setTimeout} from 'node:timers/promises';
import {atomicJson} from '../Tools/runtime_archive.mjs';
const exporter=fileURLToPath(new URL('../Tools/export_runtime.mjs',import.meta.url));
const build=fileURLToPath(new URL('../../../Build/',import.meta.url));
function fixture(week=1,league=12){const records={};const add=(key,type,v,capacity=1)=>records[key]={key,type,v,capacity};
 add('s','SeasonInfo',{CurrentSeasonYear:2026,CurrentYear:0,CurrentWeek:week,CurrentWeekType:'RegularSeason',CurrentStage:'NFLSeason'});
 add('l','Franchise',{LeagueID:league});add('f','FranchiseServer_CareerFlow',{IsStageActive:true,IsStageAdvancing:false});add('u','FranchiseUser',{Team:'t',AdminLevel:'Owner'});
 add('t','Team',{TeamIndex:1,DisplayName:'Fixture College',TEAM_LOGO:1,HomeWin:0,RoadWin:0,HomeLoss:0,RoadLoss:0},143);
 return {format:1,sourceKind:'runtime',records};}
async function job(t){const folder=await fs.mkdtemp(path.join(build,'PublicationCase-'));t.after(()=>fs.rm(folder,{recursive:true,force:true}));return folder;}
function launch(folder,gate=true){
 const input=path.join(folder,'runtime-raw.json'),output=path.join(folder,'output.json'),approval=gate?output+'.gate-1-2-'+Date.now():null;
 const child=spawn(process.execPath,[exporter,input,output,...(approval?[approval]:[])],{windowsHide:true});let stdout='',stderr='';child.stdout.on('data',b=>stdout+=b);child.stderr.on('data',b=>stderr+=b);
 const done=new Promise(resolve=>child.on('close',(code,signal)=>resolve({code,signal,stdout,stderr})));
 return {child,done,approval,output};
}
async function ready(r){for(let i=0;i<500;++i){try{return JSON.parse(await fs.readFile(r.approval+'.ready','utf8'));}catch(e){if(e.code!=='ENOENT')throw e;}await setTimeout(10);}throw Error('Fixture generator did not request approval');}
async function approve(r,value=true){await atomicJson(r.approval+'.allow',{format:1,request:path.basename(r.approval),approved:value});}
async function missing(p){await assert.rejects(fs.access(p),{code:'ENOENT'});}
test('native approval precedes all actual exporter history writes, then identical facts reuse exact output',async t=>{
 const folder=await job(t);await atomicJson(path.join(folder,'runtime-raw.json'),fixture());const r=launch(folder);await ready(r);await missing(path.join(folder,'RuntimeHistory'));
 await approve(r);const result=await r.done;assert.equal(result.code,0,result.stderr);const first=JSON.parse(await fs.readFile(r.output,'utf8'));
 const again=launch(folder,false),rerun=await again.done;assert.equal(rerun.code,0,rerun.stderr);assert.deepEqual(JSON.parse(await fs.readFile(r.output,'utf8')),first);assert.match(rerun.stdout,/'?reusedEdition"?:true/);
});
test('denial and a worker crash leave history untouched; restart recovers normally',async t=>{
 const folder=await job(t);await atomicJson(path.join(folder,'runtime-raw.json'),fixture());const denied=launch(folder);await ready(denied);await approve(denied,false);assert.notEqual((await denied.done).code,0);await missing(path.join(folder,'RuntimeHistory'));
 const crashed=launch(folder);await ready(crashed);crashed.child.kill();await crashed.done;await missing(path.join(folder,'RuntimeHistory'));
 const resumed=launch(folder,false),result=await resumed.done;assert.equal(result.code,0,result.stderr);assert.equal(JSON.parse(await fs.readFile(resumed.output,'utf8')).week,1);
});
test('rapid advances cancel the old proposal before commit and keep league/week histories isolated',async t=>{
 const folder=await job(t);await atomicJson(path.join(folder,'runtime-raw.json'),fixture(1));const first=launch(folder);await ready(first);await approve(first,false);await first.done;
 await atomicJson(path.join(folder,'runtime-raw.json'),fixture(3));const next=launch(folder,false);assert.equal((await next.done).code,0);let feed=JSON.parse(await fs.readFile(next.output,'utf8'));assert.equal(feed.week,3);assert.equal(feed.weeks.length,1);
 await atomicJson(path.join(folder,'runtime-raw.json'),fixture(0,99));const switched=launch(folder,false);assert.equal((await switched.done).code,0);feed=JSON.parse(await fs.readFile(switched.output,'utf8'));assert.equal(feed.leagueId,99);assert.ok(feed.weeks.every(w=>w.leagueId===99&&w.week===0));
 await fs.access(path.join(folder,'RuntimeHistory','12','1','2026','RegularSeason','history.index.json'));
});
test('corrupt history and advancing input fail instead of publishing a stale cached feed',async t=>{
 const folder=await job(t);await atomicJson(path.join(folder,'runtime-raw.json'),fixture());let r=launch(folder,false);assert.equal((await r.done).code,0);
 const index=path.join(folder,'RuntimeHistory','12','1','2026','RegularSeason','history.index.json');await fs.writeFile(index,'corrupt');r=launch(folder,false);assert.notEqual((await r.done).code,0);assert.equal(await fs.readFile(index,'utf8'),'corrupt');
 const advancing=fixture();advancing.records.f.v.IsStageAdvancing=true;await atomicJson(path.join(folder,'runtime-raw.json'),advancing);r=launch(folder,false);assert.notEqual((await r.done).code,0);assert.equal(await fs.readFile(index,'utf8'),'corrupt');
});
