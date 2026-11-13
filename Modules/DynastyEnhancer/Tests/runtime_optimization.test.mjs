import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs/promises';
import os from 'node:os';
import path from 'node:path';
import {runtimeRecords} from '../Tools/runtime_wire.mjs';
import {readRuntime} from '../Tools/read_runtime.mjs';
import {detectEvents} from '../Tools/story_events.mjs';
import {awaitPublicationApproval} from '../Tools/publication_gate.mjs';
function raw(){
 const records={};const add=(key,type,v,capacity=1)=>records[key]={key,type,v,capacity};
 add('season','SeasonInfo',{CurrentSeasonYear:2026,CurrentYear:0,CurrentWeek:2,CurrentWeekType:'RegularSeason',CurrentStage:'NFLSeason'});
 add('league','Franchise',{LeagueID:10});add('flow','FranchiseServer_CareerFlow',{IsStageActive:true,IsStageAdvancing:false});
 add('user','FranchiseUser',{Team:'home',AdminLevel:'Owner',IsRetired:false});
 for(const [key,id] of [['home',1],['away',2]])add(key,'Team',{TeamIndex:id,DisplayName:key,TEAM_LOGO:11+id,HomeWin:0,RoadWin:0,HomeLoss:0,RoadLoss:0},143);
 add('player','Player',{FirstName:'Fixture',LastName:'Player',TeamIndex:1,Position:'QB',InjuryStatus:'Uninjured',GameStats:'stats'});
 add('stats','GameOffensiveStats[]',{Member0:'springstat',Member1:'regularstat'});
 for(const [key,phase] of [['spring','PreSeason'],['regular','RegularSeason']]){
  add(key,'SeasonGame',{SeasonYear:0,SeasonWeek:1,SeasonWeekType:phase,IsPractice:false,GameStatus:'HomeWon',HasBeenPublished:true,HomeTeam:'home',AwayTeam:'away',HomeScore:31,AwayScore:26},250);
  add(key+'stat','GameOffensiveStats',{SeasonGame:key,OpposingTeam:'away',PASSATTEMPTS:30,PASSCOMPLETED:20,PASSYARDS:350,PASSTDS:4,PASSINTS:0});
 }
 return {format:1,sourceKind:'runtime',records};
}
function compact(raw){
 const types=[],dictionary=new Map();
 const rows=Object.values(raw.records).map(row=>{const fields=Object.keys(row.v),shape=JSON.stringify([row.type,fields]);if(!dictionary.has(shape)){dictionary.set(shape,types.length);types.push([row.type,fields]);}return [row.key,dictionary.get(shape),row.capacity??null,fields.map(k=>row.v[k])];});
 const {records,...rest}=raw;return {...rest,format:2,types,rows};
}
test('retired coach metadata never resolves stale native references; active references stay validated',async()=>{
 const r=raw();r.records.retired={key:'retired',type:'FranchiseUser',v:{IsRetired:true,AdminLevel:'Owner',Team:'deleted-team',UserEntity:'deleted-coach'}};
 const s=await readRuntime(r);assert.equal(s.userTeamIndex,1);assert.equal(s.userCoaches.length,0);
 r.records.retired.v.IsRetired=false;await assert.rejects(readRuntime(r),/Unresolved live record/);
});
test('lossless wire preserves normalized facts/hash, nulls, zero, false, field absence and references',async()=>{
 const source=raw();source.records['optional']={key:'optional',type:'Optional',v:{A:0,B:false,C:null}};
 const encoded=compact(source);assert.deepEqual(runtimeRecords(encoded),Object.values(source.records));
 assert.deepEqual(await readRuntime(encoded),await readRuntime(source));
});
test('corrupt schemas/rows, duplicates and unresolved references fail closed',async()=>{
 const good=compact(raw());
 for(const mutate of [r=>r.types[0][1].push('bad'),r=>r.rows.push(r.rows[0]),r=>r.rows[0][1]=999,r=>r.rows[0][2]=-1,r=>r.types[0][1][0]='__proto__']){
  const bad=structuredClone(good);mutate(bad);assert.throws(()=>runtimeRecords(bad));
 }
 const bad=raw();bad.records.user.v.Team='missing';await assert.rejects(readRuntime(bad),/Unresolved/);
});
test('Extension spring/drill/marker games never become competitive results or performances',async()=>{
 const s=await readRuntime(raw());assert.deepEqual(s.games.map(g=>g.id),['regular']);assert.deepEqual(s.performances.map(p=>p.game),['regular']);
 const events=detectEvents(s);assert.ok(events.some(e=>e.id==='result:regular'));assert.ok(!events.some(e=>e.id.includes('spring')));
 for(let week=1;week<=4;++week){const r=raw();r.records.season.v.CurrentWeekType='PreSeason';r.records.season.v.CurrentWeek=week;const spring=await readRuntime(r);assert.ok(!spring.games.some(g=>g.phase==='PreSeason'));assert.ok(!detectEvents(spring).some(e=>e.id.includes('spring')));}
});
test('phase guard also protects normalized legacy inputs without dropping a prior bowl result',async()=>{
 const s=await readRuntime(raw());const regular=s.games[0];s.games.push({...regular,id:'synthetic',phase:'PreSeason'});
 assert.ok(!detectEvents(s).some(e=>e.id==='result:synthetic'));
 s.season.CurrentWeekType='BowlSeason2';s.season.CurrentWeek=18;s.games=[{...regular,id:'bowl',phase:'BowlSeason1',week:17}];s.performances=[];
 assert.ok(detectEvents(s).some(e=>e.id==='result:bowl'));
});
test('advancing flow is rejected in both transports',async()=>{
 const r=raw();r.records.flow.v.IsStageAdvancing=true;
 await assert.rejects(readRuntime(r),/advancing/);await assert.rejects(readRuntime(compact(r)),/advancing/);
});
test('publication gate requires matching one-shot approval; timeout/denial cannot authorize history',async()=>{
 const dir=await fs.mkdtemp(path.join(os.tmpdir(),'cot-gate-')),output=path.join(dir,'output.json');
 try{
  const gate=output+'.gate-1-2-3',pending=awaitPublicationApproval(gate,output,{league:1},{timeoutMs:1000});
  while(true){try{await fs.access(gate+'.ready');break;}catch{await new Promise(r=>setTimeout(r,5));}}
  await fs.writeFile(gate+'.allow',JSON.stringify({format:1,request:path.basename(gate),approved:true}));await pending;
  const denied=output+'.gate-1-2-4';await fs.writeFile(denied+'.allow',JSON.stringify({format:1,request:path.basename(denied),approved:false}));
  await assert.rejects(awaitPublicationApproval(denied,output,{},{timeoutMs:50}),/refused/);
  await assert.rejects(awaitPublicationApproval(output+'.gate-1-2-5',output,{},{timeoutMs:30}),/timed out/);
  await assert.rejects(awaitPublicationApproval(path.join(dir,'..','foreign.gate-1-2-3'),output,{}),/Invalid/);
 }finally{await fs.rm(dir,{recursive:true,force:true});}
});

test('Extensions singleton SeasonGame tables are exhibition context, not competitive rows',async()=>{
 const r=raw();r.records.spring.capacity=1;r.records.spring.v.SeasonGameNum=4;r.records.season.v.CurrentWeekType='PreSeason';r.records.season.v.CurrentStage='PreSeason';
 const s=await readRuntime(r);assert.equal(s.exhibitions.active,true);assert.equal(s.exhibitions.fixtures.length,1);assert.ok(!s.games.some(g=>g.phase==='PreSeason'));
 r.records.spring.v.IsPractice=true;assert.equal((await readRuntime(r)).exhibitions.active,false);
});
