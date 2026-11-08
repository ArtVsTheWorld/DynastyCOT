import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs/promises';
import os from 'node:os';
import path from 'node:path';
import {calendarEvents,editionCalendar,gameDate} from '../Tools/calendar_events.mjs';
import {observeRecruitGames,recruitGameEvents} from '../Tools/recruit_game_records.mjs';
import {casualEvents} from '../Tools/casual_events.mjs';
import {worldEvents} from '../Tools/world_events.mjs';
import {renderStories} from '../Tools/story_renderer.mjs';
import {draftBoard,suppliedDraftOrder} from '../Tools/draft_results.mjs';
import {conferenceGrammar} from '../Tools/conference_language.mjs';
import {writeEditionHistory,readEditionHistory} from '../Tools/history_store.mjs';
import {freshEditorialEvents} from '../Tools/editorial_policy.mjs';
const fixture=()=>({format:2,gameYear:27,sourceKind:'runtime',sourceUnchanged:true,sha256:'a'.repeat(64),leagueId:70616,userTeamIndex:111,season:{CurrentStage:'NFLSeason',CurrentYear:0,CurrentSeasonYear:2026,CurrentWeekType:'RegularSeason',CurrentWeek:9},teams:[{id:111,name:'W. Michigan',logo:111,prestigeStars:1,mediaRank:99,mediaLastRank:99,rivals:[]},{id:49,name:'Michigan',logo:49,prestigeStars:4.5,mediaRank:20,mediaLastRank:14,rivals:[50]},{id:50,name:'Michigan State',logo:50,prestigeStars:3.5,mediaRank:99,rivals:[49]}],players:[],games:[{id:'schedule',year:0,week:9,phase:'RegularSeason',status:'Unplayed',month:10,day:31,weekday:'Saturday',home:111,away:49}],performances:[],heisman:[],commitments:[],offers:[],recruitStatuses:[],recruitInterests:[]});
test('RC016 is exactly the approved finite 464 alternatives; facts and roles remain guarded',async()=>{
 const p=JSON.parse(await fs.readFile(new URL('../Content/Runtime/beyond-field-pack.json',import.meta.url),'utf8'));assert.equal(Object.values(p.families).flat().length,464);assert.deepEqual(p.boundedExpansion,{casual:192,historical:48,seasonal:80,contextual:96,recruit:48});
 for(const [family,lines] of Object.entries(p.families))for(const line of lines){assert.ok(line.id&&line.angle);if(/^(casual|calendar|archive)\./.test(family))assert.ok(line.when?.some(k=>k.startsWith('role')));assert.ok(!/changes the conversation|insider|sources say|interviewed/i.test(line.body));}
});
test('calendar maps actual dates, fourth Thursday, Week13, rollover and missing anchors',()=>{
 const s=fixture();assert.equal(gameDate(s,s.games[0]),Date.UTC(2026,9,31));assert.equal(calendarEvents(s)[0].facts.calendarKey,'halloween');
 Object.assign(s.games[0],{month:11,day:26,weekday:'Thursday'});assert.equal(calendarEvents(s)[0].facts.calendarDate,'2026-11-26');
 Object.assign(s.season,{CurrentWeekType:'BowlSeason1',CurrentWeek:17});Object.assign(s.games[0],{phase:'BowlSeason1',week:17,month:12,day:25,weekday:'Friday'});assert.equal(calendarEvents(s)[0].facts.calendarKey,'christmas');
 Object.assign(s.games[0],{month:12,day:31,weekday:'Thursday'});const first=calendarEvents(s);assert.equal(first[0].id,'calendar:2027:newyear');Object.assign(s.games[0],{month:1,day:1,weekday:'Friday'});assert.equal(calendarEvents(s)[0].id,first[0].id);
 const previous={leagueId:s.leagueId,userTeamIndex:s.userTeamIndex,season:2026,phase:'BowlSeason1',week:16,stories:[{eventId:first[0].id}],events:first.map(e=>({...e,teamIds:[111]}))};assert.equal(freshEditorialEvents(first.map(e=>({...e,teamIds:[111]})),{leagueId:s.leagueId,userTeamIndex:s.userTeamIndex,season:2026,phase:'BowlSeason1',week:17},[previous]).length,0);
 s.games=[];assert.equal(editionCalendar(s),null);assert.deepEqual(calendarEvents(s),[]);Object.assign(s.season,{CurrentWeekType:'RegularSeason',CurrentWeek:13});assert.equal(calendarEvents(s)[0].facts.calendarKey,'rivalry');
 s.games=[{year:0,phase:'RegularSeason',week:13,month:11,day:31}];assert.equal(gameDate(s,s.games[0]),null);
});
test('captured bowl date ranges include Christmas/NewYear despite stale native weekday labels',()=>{
 const s=fixture();Object.assign(s.season,{CurrentWeekType:'BowlSeason1',CurrentWeek:17});s.games=[{year:0,phase:'BowlSeason1',week:17,month:12,day:15,weekday:'Tuesday'},{year:0,phase:'BowlSeason1',week:17,month:12,day:28,weekday:'Saturday'}];
 assert.ok(editionCalendar(s).weekdayMismatches>0);assert.equal(calendarEvents(s).find(e=>e.facts.calendarKey==='christmas')?.facts.calendarDate,'2026-12-25');assert.ok(!calendarEvents(s).some(e=>e.facts.calendarKey==='newyear'));
 Object.assign(s.season,{CurrentWeekType:'BowlSeason2',CurrentWeek:18});s.games=[{year:0,phase:'BowlSeason2',week:18,month:12,day:29,weekday:'Tuesday'},{year:0,phase:'BowlSeason2',week:18,month:1,day:3,weekday:'Friday'}];assert.ok(editionCalendar(s).weekdayMismatches>0);assert.equal(calendarEvents(s).filter(e=>e.facts.calendarKey==='newyear').length,1);assert.equal(calendarEvents(s)[0].facts.calendarDate,'2027-01-01');
});
test('a holiday between games uses bounded neighboring regular-edition dates without inventing an anchor',()=>{
 const s=fixture();Object.assign(s.season,{CurrentYear:1,CurrentSeasonYear:2027,CurrentWeek:10});s.games=[{year:1,phase:'RegularSeason',week:9,month:10,day:30,weekday:'Saturday'},{year:1,phase:'RegularSeason',week:10,month:11,day:2,weekday:'Tuesday'},{year:1,phase:'RegularSeason',week:10,month:11,day:6,weekday:'Saturday'}];
 assert.equal(calendarEvents(s).find(e=>e.facts.calendarKey==='halloween')?.facts.calendarDate,'2027-10-31');assert.equal(editionCalendar(s).source,'captured-neighbor-and-current-schedule');
 s.games=s.games.slice(1);assert.ok(!calendarEvents(s).some(e=>e.facts.calendarKey==='halloween'));assert.equal(editionCalendar(s).source,'captured-schedule-range');
 s.games.unshift({year:1,phase:'RegularSeason',week:9,month:11,day:3});assert.equal(editionCalendar(s).source,'captured-schedule-range');s.games[0].month=10;s.games[0].day=16;assert.equal(editionCalendar(s).source,'captured-schedule-range');
});
test('WMU31–26 over #14 Michigan has celebration, frustration, scrutiny and a supported rival',()=>{
 const s=fixture();s.season.CurrentWeek=2;s.games=[{id:'fixture-wmu-michigan',year:0,phase:'RegularSeason',week:1,status:'AwayWon',published:true,home:49,away:111,homeName:'Michigan',awayName:'W. Michigan',homeScore:26,awayScore:31}];s.pregameGames={'fixture-wmu-michigan':{ranks:{111:99,49:14}}};
 const event=worldEvents(s).find(e=>e.category==='upsets');assert.ok(event.facts.huge&&event.facts.programDisparity);const e={...event,...event.options,teamIds:[111,49],scopes:['national','team'],priority:event.options.priority,thread:event.options.thread};
 const result=renderStories(s,[e]);for(const family of ['world.upset','reply.fan.upset','reply.fan.upset-loss','context.upset-rival','context.upset-expectation'])assert.ok(result.stories.some(p=>p.templateFamily===family),family);
 assert.equal(result.stories.filter(p=>p.replyTo).length,2);assert.ok(result.stories.every(p=>[...p.body].length<=280&&Buffer.byteLength(p.body)<=512));assert.ok(result.stories.every(p=>!p.body.includes('interview')&&!p.body.includes('injury')));
 const same=renderStories(s,[e],{authors:result.authors,sameWeek:{stories:result.stories}});assert.deepEqual(same.stories,result.stories);
});
test('recruit game records are persistent, separate, weak/noisy and unknown traits are neutral',()=>{
 const s=fixture();for(let i=0;i<10000;i++){s.players.push({id:'p'+i,name:'Prospect '+i,presentationId:i+1,position:'QB',team:255,starRating:3,developmentTrait:'Normal'});s.recruitStatuses.push({player:'p'+i,recruitClass:'HighSchool',stage:'Top10',nationalRank:i+1,topSchools:[111]});}
 const native=JSON.stringify(s),normal=observeRecruitGames(s);assert.equal(JSON.stringify(s),native);assert.ok(normal.records.length>800&&normal.records.length<1350);assert.deepEqual(observeRecruitGames(s,normal.ledger),normal);assert.ok(normal.records.every(r=>r.values.tackles>=r.values.sacks&&r.values.kickMade<=r.values.kickAttempts&&r.values.receiveYards>=r.values.catches));
 const elite=structuredClone(s);for(const p of elite.players)p.developmentTrait='College_Elite';const higher=observeRecruitGames(elite);const delta=(higher.records.length-normal.records.length)/s.players.length;assert.ok(delta>.01&&delta<.05,delta);
 const nmax=Math.max(...normal.records.map(r=>r.values.passYards)),nmin=Math.min(...normal.records.map(r=>r.values.passYards));assert.ok(higher.records.filter(r=>r.values.passYards>=nmin&&r.values.passYards<=nmax).length/higher.records.length>.95);
 const unknown=structuredClone(s);for(const p of unknown.players)p.developmentTrait='not-captured';assert.deepEqual(observeRecruitGames(unknown),normal);const retained=observeRecruitGames(elite,normal.ledger);for(const r of normal.records)assert.deepEqual(retained.ledger.records[r.key],r);
 s.recruitGameRecords=normal.records;s.recruitInterests=normal.records.slice(0,2).map(r=>r.player);const events=recruitGameEvents(s);assert.ok(events.length<=3&&events.length>0);assert.ok(events.every(e=>e.facts.recordSource==='mod-modeled-hs'&&!('developmentTrait' in e.facts)));
 const foreign=structuredClone(s);foreign.leagueId++;assert.notEqual(observeRecruitGames(foreign,normal.ledger).ledger.scope,normal.ledger.scope);
 const old=normal.records[0];s.players.find(p=>p.id===old.player).name='New Record Owner';assert.ok(observeRecruitGames(s,normal.ledger).records.every(r=>r.key!==old.key));
});
test('specialist modeled lines never give punters field goals or kickers punts',()=>{
 for(const position of ['K','P']){const s=fixture();s.players=[{id:'k',name:'A Very Long Specialist Name With A Suffix Jr.',position,team:255,personality:'Leader'}];const e={id:'film',category:'recruit-film',teamIds:[111],playerId:'k',scopes:['team'],priority:69,facts:{family:'recruit.game.specialist',voice:'recruit',kickMade:2,kickAttempts:3,punts:5,puntAverage:42,isKicker:position==='K',isPunter:position==='P'}};const post=renderStories(s,[e]).stories[0],body=post.body;assert.equal(post.stamp,'COT HS Game');assert.ok(position==='K'?/field.goal/.test(body)&&!body.includes('punts'):/punt/.test(body)&&!body.includes('field goal'));}
});
test('modeled game ledger round trips through immutable history without changing old blobs',async()=>{
 const dir=await fs.mkdtemp(path.join(os.tmpdir(),'cot-rc016-')),file=path.join(dir,'history.json');try{const s=fixture(),state={format:1,authors:{},editions:[],recruitGameLedger:{...observeRecruitGames(s).ledger,records:{fixture:{player:'synthetic',week:9,values:{passYards:212}}}}};await writeEditionHistory(file,state);assert.deepEqual((await readEditionHistory(file)).recruitGameLedger,state.recruitGameLedger);assert.equal((await readEditionHistory(file,{archiveOnly:true})).recruitGameLedger,undefined);
  const indexPath=path.join(dir,'history.index.json'),index=JSON.parse(await fs.readFile(indexPath,'utf8'));delete index.recruitGameLedger;await fs.writeFile(indexPath,JSON.stringify(index));assert.deepEqual((await readEditionHistory(file)).recruitGameLedger,state.recruitGameLedger,'An older writer dropping unknown index fields must not lose modeled records');
  const wrong={...state.recruitGameLedger,scope:'999|111|2026'};index.recruitGameLedger=wrong;await fs.writeFile(indexPath,JSON.stringify(index));await assert.rejects(readEditionHistory(file),/another Dynasty/);
 }finally{await fs.rm(dir,{recursive:true,force:true});}
});
test('supplied 257-slot reference maps224 native outcomes only at the verified first offseason',()=>{
 const s=fixture();Object.assign(s.season,{CurrentStage:'OffSeason',CurrentWeekType:'OffSeason',CurrentWeek:2,CurrentOffseasonStage:3});s.players=Array.from({length:224},(_,i)=>({id:'d'+i,team:111,position:i%7?'WR':'QB',overall:60+i%35}));s.draftResults=s.players.map((p,i)=>({player:p.id,round:1+Math.floor(i/32)}));const original=JSON.stringify(s),board=draftBoard(s),reference=suppliedDraftOrder(s);assert.equal(board.length,224);assert.equal(reference.suppliedPicks.length,257);assert.equal(JSON.stringify(s),original);for(const p of board){assert.equal(p.nflTeam,reference.rounds[p.round-1][p.roundPick-1]);assert.equal(s.draftResults.find(d=>d.player===p.player).round,p.round);}assert.deepEqual(draftBoard(s,board),board);
 for(const changed of [{CurrentYear:1},{CurrentSeasonYear:2027},{CurrentOffseasonStage:2},{CurrentWeek:1}]){const other=structuredClone(s);Object.assign(other.season,changed);assert.equal(suppliedDraftOrder(other),null);}
});
test('conference prose uses customary article without duplicated articles',()=>{
 assert.equal(conferenceGrammar('East Point joins C-USA.'),'East Point joins the C-USA.');assert.equal(conferenceGrammar('East Point joins the the Conference USA.'),'East Point joins the C-USA.');assert.equal(conferenceGrammar('Leaving SEC for Big 12.'),'Leaving the SEC for Big 12.');assert.equal(conferenceGrammar('Massachusetts is a name.'),'Massachusetts is a name.');
});
test('casual candidates are bounded standalone opinions, deterministic and do not require game facts',()=>{
 const s=fixture();s.games=[];const a=casualEvents(s);assert.ok(a.length>=2&&a.length<=3);assert.ok(a.every(e=>e.facts.opinion&&e.options.priority<=44&&!e.facts.replyTo));assert.deepEqual(casualEvents(s),a);assert.deepEqual(casualEvents({...s,reconstructed:true}),[]);
});
