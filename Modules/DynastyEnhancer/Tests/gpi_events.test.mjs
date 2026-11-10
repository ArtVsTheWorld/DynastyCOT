import test from 'node:test';
import assert from 'node:assert/strict';
import {gpiEvents,gpiObservation} from '../Tools/gpi_events.mjs';
import {detectEvents} from '../Tools/story_events.mjs';
import {renderStories} from '../Tools/story_renderer.mjs';
import {authorRegistry} from '../Tools/story_authors.mjs';
import {avatarFor} from '../Tools/story_avatars.mjs';
import {buildFeed} from '../Tools/feed_engine.mjs';
import {chooseTemplate} from '../Tools/story_templates.mjs';
const fixture=()=>{
 const s={format:2,gameYear:27,sourceUnchanged:true,sha256:'a'.repeat(64),source:'runtime:10:1',sourceKind:'runtime',leagueId:10,userTeamIndex:1,
  season:{CurrentSeasonYear:2027,CurrentYear:1,CurrentWeek:8,CurrentWeekType:'RegularSeason',CurrentStage:'NFLSeason'},
  teams:[1,2,3,4,5].map(id=>({id,name:'School '+id,logo:id,rivals:[],mediaRank:99,mediaLastRank:99})),
  players:[1,2,3,4,5].map(id=>({id:'p'+id,name:'Player '+id,team:id,position:'LT',personality:'TeamPlayer',injury:'Uninjured'})),
  games:[],performances:[],heisman:[],offers:[],commitments:[],recruitStatuses:[]};
 const row=i=>({player:'p'+i,team:i,position:'LT',group:'OL',grade:94-i,snaps:500,opportunities:500,gradedGames:7});
 s.playerAnalytics={format:1,modelVersion:'0.2.0-production',leagueId:10,year:1,season:2027,week:7,weekType:'RegularSeason',weekly:[1,2,3,4,5].map(row),seasonRows:[1,2,3,4,5].map(row)};return s;
};
test('GPI has its own persistent account and exact verified native icon without changing old accounts',()=>{
 const s=fixture(),registry=authorRegistry(),old=registry.get(s,'analyst',s.teams[0],null,0),saved=structuredClone(registry.entries);
 const account=registry.get(s,'analyst',s.teams[0],null,6);assert.equal(account.author,'GPI');assert.equal(account.handle,'@GPIndex');assert.deepEqual(avatarFor('analyst',s.teams[0],null,6),{imageLibrary:88,imageValue:116});
 assert.deepEqual(Object.values(saved)[0],old);assert.deepEqual(authorRegistry(registry.entries).get(s,'analyst',s.teams[1],null,6),account);
});
test('existing analytics account rebrands without changing its key, personality or saved input',()=>{
 const s=fixture(),key='10:pff:national:6',saved={[key]:{identityVersion:2,author:'PFF',handle:'@PFF_College',styleSeed:12345,faithLanguage:false,character:{version:1,voice:'evidence'}}};
 const snapshot=structuredClone(saved),registry=authorRegistry(saved),account=registry.get(s,'analyst',s.teams[0],null,6);
 assert.deepEqual(saved,snapshot);assert.deepEqual(Object.keys(registry.entries),[key]);
 assert.deepEqual(account,{...snapshot[key],author:'GPI',handle:'@GPIndex'});
 assert.deepEqual(authorRegistry(registry.entries).get(s,'analyst',s.teams[1],null,6),account);
});
test('an upgrade preserves published legacy analytics posts and stable event/template IDs',()=>{
 const s=fixture(),first=buildFeed(s);
 for(const p of first.stories.filter(p=>p.author==='GPI')){p.author='PFF';p.handle='@PFF_College';p.imageValue=125;}
 const snapshot=structuredClone(first),second=buildFeed(s,null,{sameWeek:first});
 assert.deepEqual(first,snapshot);
 assert.deepEqual(second.stories.filter(p=>p.templateFamily?.startsWith('pff.')),first.stories.filter(p=>p.templateFamily?.startsWith('pff.')));
 const next=fixture();next.season.CurrentWeek++;next.playerAnalytics.week++;next.playerAnalytics.seasonRows[0].gradedGames++;
 const posts=renderStories(next,detectEvents(next).filter(e=>e.facts.family?.startsWith('pff.'))).stories;
 assert.ok(posts.length);assert.ok(posts.every(p=>p.author==='GPI'&&p.handle==='@GPIndex'&&p.imageValue===116));
});
test('GPI events require same-Dynasty/year native model, sufficient opportunities and matching player team',()=>{
 const s=fixture();assert.ok(gpiEvents(s).length);const old=structuredClone(s);
 for(const change of [x=>x.playerAnalytics.leagueId++,x=>x.playerAnalytics.year++,x=>delete x.playerAnalytics,x=>x.playerAnalytics.season++,x=>x.season.CurrentWeekType='Offseason',x=>x.reconstructed=true,x=>x.playerAnalytics.week=8,x=>x.playerAnalytics.weekType='Unknown',x=>x.playerAnalytics.seasonRows=null]){const bad=structuredClone(old);change(bad);assert.deepEqual(gpiEvents(bad),[]);}
 s.playerAnalytics.weekly.forEach(r=>r.snaps=1);s.playerAnalytics.seasonRows.forEach(r=>r.snaps=1);assert.deepEqual(gpiEvents(s),[]);
 const wrong=fixture();wrong.players.forEach(p=>p.team=255);assert.deepEqual(gpiEvents(wrong),[]);
});
test('compact persisted observations suppress stale samples and support genuine movement',()=>{
 const s=fixture(),prior={playerAnalytics:gpiObservation(s)};assert.deepEqual(gpiEvents(s,prior),[]);
 assert.ok(!('weekly' in prior.playerAnalytics));assert.ok(!('opportunities' in prior.playerAnalytics.seasonRows[0]));
 s.playerAnalytics.seasonRows[0].gradedGames++;s.playerAnalytics.seasonRows[0].grade=81;
 assert.equal(gpiEvents(s,prior)[0].facts.family,'pff.movement');
});
test('weak weekly samples are not called standouts and comparisons do not cross Dynasty/team/position',()=>{
 const weak=fixture();weak.playerAnalytics.seasonRows=[];weak.playerAnalytics.weekly[0].grade=55;
 assert.ok(gpiEvents(weak).every(e=>e.options.team!==1));
 for(const change of [x=>x.leagueId++,x=>x.seasonRows[0].team++,x=>x.seasonRows[0].group='QB']){
  const s=fixture(),prior=structuredClone(s);prior.playerAnalytics.seasonRows[0].grade=40;prior.playerAnalytics.seasonRows[0].gradedGames=5;change(prior.playerAnalytics);
  assert.ok(gpiEvents(s,prior).every(e=>e.facts.family!=='pff.movement'));
 }
});
test('published GPI wording, account and captured grade survive same-edition model/content refresh',()=>{
 const s=fixture(),first=buildFeed(s),published=first.stories.filter(p=>p.author==='GPI');assert.ok(published.length);
 const saved=structuredClone(first);s.playerAnalytics.modelVersion='future-model';s.playerAnalytics.seasonRows.forEach(r=>r.grade-=20);s.playerAnalytics.weekly.forEach(r=>r.grade-=20);
 const second=buildFeed(s,null,{sameWeek:first});assert.deepEqual(second.stories.filter(p=>p.author==='GPI'),published);assert.deepEqual(first,saved);
 const other=fixture();other.leagueId++;other.playerAnalytics.leagueId++;assert.ok(buildFeed(other,null,{sameWeek:first}).events.filter(e=>e.facts.family?.startsWith('pff.')).every(e=>e.facts.modelVersion!=='future-model'));
});
test('at most four optional GPI stories, one per team, and unchanged samples do not recycle',()=>{
 const s=fixture(),events=gpiEvents(s);assert.ok(events.length<=4);assert.equal(new Set(events.map(e=>e.options.team)).size,events.length);assert.ok(events.every(e=>!e.options.coverageExempt));
 assert.deepEqual(gpiEvents(s,structuredClone(s)),[]);assert.deepEqual(gpiEvents(s),events);
});
test('movement requires more graded games in the same model, not a model-change jump',()=>{
 const s=fixture(),old=structuredClone(s);old.playerAnalytics.seasonRows[0].grade=80;old.playerAnalytics.seasonRows[0].gradedGames=6;
 const e=gpiEvents(s,old).find(e=>e.facts.family==='pff.movement');assert.ok(e);assert.equal(e.facts.previousGrade,'80.0');assert.equal(e.facts.grade,'93.0');assert.equal(e.facts.direction,'up');
 old.playerAnalytics.modelVersion='old';assert.ok(gpiEvents(s,old).every(e=>e.facts.family!=='pff.movement'));
});
test('rendered GPI uses its own evidence voice, relevant numerical units and stable refreshes',()=>{
 const s=fixture(),events=detectEvents(s),pff=events.filter(e=>e.facts.family?.startsWith('pff.'));assert.ok(pff.length);
 const a=renderStories(s,pff),b=renderStories(s,pff,{authors:a.authors});assert.deepEqual(a.stories,b.stories);assert.ok(a.stories.length);
 for(const p of a.stories){assert.equal(p.author,'GPI');assert.equal(p.handle,'@GPIndex');assert.equal(p.authorVoice,'evidence');assert.equal(p.imageLibrary,88);assert.equal(p.imageValue,116);assert.ok(p.body.includes('93.0')||/9[0-2]\.0/.test(p.body));assert.ok([...p.body].length<=320);assert.ok(!/interview|sources|pressure|film says/i.test(p.body));}
 const feed=buildFeed(s);assert.ok(feed.stories.filter(p=>p.author==='GPI').length<=4);assert.ok(!feed.stories.some(p=>p.author==='GPI'&&(p.essentialNews||p.coverageExempt)));
});
test('long native names, sparse data and no invented parent replies',()=>{
 const s=fixture();s.players[0].name='Alexanderson Longlastname-Suffix III';s.teams[0].name='Western Metropolitan State University';
 const a=renderStories(s,detectEvents(s).filter(e=>e.facts.family?.startsWith('pff.')));assert.ok(a.stories.some(p=>p.author==='GPI'));assert.ok(a.stories.every(p=>!p.replyTo&&[...p.body].length<=320));
 s.playerAnalytics.weekly=[];s.playerAnalytics.seasonRows=[];assert.deepEqual(gpiEvents(s),[]);
});
test('twelve editions never borrow generic positive player reactions for GPI low grades',()=>{
 for(let week=1;week<=12;week++){const s=fixture();s.season.CurrentWeek=week;s.playerAnalytics.week=week-1;s.playerAnalytics.seasonRows[0].grade=42;
  const posts=renderStories(s,detectEvents(s).filter(e=>e.facts.family?.startsWith('pff.'))).stories;
  assert.ok(posts.length);assert.ok(posts.every(p=>p.author==='GPI'&&!p.replyTo));
 }
});
test('all 48 revised GPI variants render with explicit units and native character/byte limits, including long names',()=>{
 for(const family of ['pff.weekly','pff.season-spotlight','pff.season-watch','pff.movement'])for(const long of [false,true]){
  const values={playerName:long?'Alexanderson Longlastname-Suffix III':'Jake Jones',school:long?'Western Metropolitan State University':'Test',position:'LT',grade:'89.4',previousGrade:'84.2',direction:'up',gradedGames:12,sampleLabel:'1200 recorded snaps',weekLabel:'BowlSeason1 Week 17'};
  let candidates;assert.ok(chooseTemplate(family,values,'limits',{voice:'evidence',onCandidates:x=>candidates=x.candidates}));assert.equal(candidates.length,12);
  for(const c of candidates){assert.ok([...c.body].length<=280&&Buffer.byteLength(c.body,'utf8')<=512);assert.ok(!/[{}\uFFFD]/u.test(c.body));}
 }
});
