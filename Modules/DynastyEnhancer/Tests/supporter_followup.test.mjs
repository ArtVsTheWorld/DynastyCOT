import test from 'node:test';import assert from 'node:assert/strict';import fs from 'node:fs/promises';import os from 'node:os';import path from 'node:path';
import {sharedEdition} from '../Tools/shared_editions.mjs';
import {updateRuntime,scopeKey} from '../Tools/runtime_lifecycle.mjs';
import {loadArchives,readHistory,writeHistory} from '../Tools/runtime_archive.mjs';
import {detectEvents} from '../Tools/story_events.mjs';
import {recoverWeek} from '../Tools/recover_weeks.mjs';
const fixture=(team=1,week=4)=>({format:2,gameYear:27,sourceKind:'runtime',source:'runtime:1:'+team,sourceUnchanged:true,sha256:'a'.repeat(64),leagueId:1,userTeamIndex:team,season:{CurrentSeasonYear:2026,CurrentYear:0,CurrentWeek:week,CurrentWeekType:'RegularSeason',CurrentStage:'NFLSeason'},teams:[{id:1,name:'One',logo:1},{id:2,name:'Two',logo:2}],players:[],games:[{id:'g',home:1,away:2,year:0,week:1,phase:'RegularSeason',status:'HomeWon',homeScore:20,awayScore:14,published:true}],performances:[],offers:[],commitments:[],heisman:[],conferences:[]});
test('new school recovers existing public copy without old private context or changed identities',async t=>{
 const dir=await fs.mkdtemp(path.join(os.tmpdir(),'cot-school-'));t.after(()=>fs.rm(dir,{recursive:true,force:true}));const s=fixture(1,2),old=updateRuntime(s);const file=path.join(dir,'RuntimeHistory',scopeKey(s),'history.json');await fs.mkdir(path.dirname(file),{recursive:true});await writeHistory(file,old.state);const bytes=await fs.readFile(file.replace('.json','.index.json'));
 const next=fixture(2,4),archives=await loadArchives(dir,next,{crossSchool:true});assert.ok(archives.some(f=>f.week===2&&f.recoveredFromTeam===1));const recovered=archives.find(f=>f.week===2);assert.deepEqual(recovered.stories.map(p=>[p.id,p.author,p.handle,p.body]),old.catalog.stories.map(p=>[p.id,p.author,p.handle,p.body]));assert.ok(!recovered.stories.some(p=>p.userRelevant));
 const result=updateRuntime(next,undefined,{archives});assert.equal(result.state.editions.find(e=>e.week===2).snapshot,undefined);assert.ok(result.catalog.weeks.some(e=>e.week===2));assert.deepEqual(await fs.readFile(file.replace('.json','.index.json')),bytes);
 assert.equal(sharedEdition(recovered,{...next,leagueId:99}),null);assert.equal(sharedEdition(recovered,{...next,season:{...next.season,CurrentWeek:1}}),null);
});
test('immutable history writes detect edits, preserve references, and corrupted/missing blobs reject',async t=>{
 const dir=await fs.mkdtemp(path.join(os.tmpdir(),'cot-blobs-'));t.after(()=>fs.rm(dir,{recursive:true,force:true}));const file=path.join(dir,'history.json');await writeHistory(file,updateRuntime(fixture()).state);const state=await readHistory(file,{activeWeek:4}),index=JSON.parse(await fs.readFile(file.replace('.json','.index.json')));await writeHistory(file,state);const same=JSON.parse(await fs.readFile(file.replace('.json','.index.json')));assert.deepEqual(same,index);
 state.editions.at(-1).feed.stories[0].body='Changed in isolated test';await writeHistory(file,state);const changed=JSON.parse(await fs.readFile(file.replace('.json','.index.json')));assert.notEqual(changed.editions.at(-1).feedFile,index.editions.at(-1).feedFile);
 const blob=path.join(dir,'.history-data',changed.editions.at(-1).feedFile);await fs.appendFile(blob,' ');await assert.rejects(readHistory(file),/checksum/);await fs.unlink(blob);await assert.rejects(readHistory(file),/ENOENT/);
});
test('initial recovery is bounded to public completed same-season facts and rewinds prune future knowledge',()=>{
 const s=fixture(),r=updateRuntime(s);assert.deepEqual(r.state.editions.map(e=>e.week),[2,4]);const recovered=r.state.editions[0];assert.equal(recovered.snapshot,undefined);assert.ok(recovered.feed.events.every(e=>['results','team-stats','player-stats'].includes(e.category)));
 const rewind=fixture(1,1),back=updateRuntime(rewind,r.state);assert.ok(back.catalog.weeks.every(e=>e.week<=1));
 const missing=fixture();missing.games[0].published=false;assert.deepEqual(updateRuntime(missing).catalog.weeks.map(e=>e.week),[4]);
});
test('bounded recovery detection equals full game facts and never mutates its input',()=>{
 const current=fixture(),s=structuredClone(current);s.reconstructed=true;s.season.CurrentWeek=2;
 s.teams=s.teams.map(t=>({...t,mediaRank:null,mediaLastRank:null,coachesRank:null,coachesLastRank:null,cfpRank:null,cfpLastRank:null,seasonStats:null}));
 assert.deepEqual(detectEvents(s,null,{gameFactsOnly:true}),detectEvents(s).filter(e=>['results','team-stats','player-stats'].includes(e.category)));
 assert.throws(()=>detectEvents(current,null,{gameFactsOnly:true}),/requires a reconstruction/);
 const freeze=o=>{if(o&&typeof o==='object'){Object.freeze(o);for(const v of Object.values(o))freeze(v);}return o;};
 const expected=structuredClone(current);freeze(current);const feed=recoverWeek(current,2).feed;assert.equal(feed.contentStatus,'reconstructed');assert.deepEqual(current,expected);
});
