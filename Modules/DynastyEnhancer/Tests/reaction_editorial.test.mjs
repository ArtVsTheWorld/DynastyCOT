import test from 'node:test';import assert from 'node:assert/strict';
import {freshEditorialEvents} from '../Tools/editorial_policy.mjs';
import {deduplicateConversations} from '../Tools/conversation_topics.mjs';
import {milestoneTotalDetail} from '../Tools/milestone_text.mjs';
import {contentPack,chooseTemplate} from '../Tools/story_templates.mjs';
import {historyMilestoneEvents} from '../Tools/history_milestones.mjs';
const pub=(phase='PreSeason',week=2)=>({leagueId:1,userTeamIndex:130,season:2026,phase,week});
const event=standard=>({id:'cycle:bar:130',category:'preseason',teamIds:[130],facts:{family:'cycle.program-bar',school:'JMU',standard}});
const old=e=>({...pub(),events:[e],stories:[{id:'root',eventId:e.id}]});
test('expectations are season-long across spring/fall, only a changed supported standard reopens them',()=>{
 const e=event('Conference contention');for(const p of [pub('PreSeason',3),pub('PreSeason',4),pub('RegularSeason',0),pub('RegularSeason',1)])assert.equal(freshEditorialEvents([e],p,[old(e)]).length,0);
 assert.equal(freshEditorialEvents([event('Championship contention')],pub('PreSeason',3),[old(e)]).length,1);
 for(const change of [{season:2027},{leagueId:2},{userTeamIndex:2},{reconstructed:true}])assert.equal(freshEditorialEvents([e],pub('PreSeason',3),[{...old(e),...change}]).length,1);
 assert.equal(freshEditorialEvents([e],pub('PreSeason',3),[{...old(e),stories:[]}]).length,1);
 assert.equal(freshEditorialEvents([e],pub('PreSeason',3),[old(e)],{...old(e),...pub('PreSeason',3)}).length,1); // already published current edition survives upgrade
});
test('cross-account identical chatter is reduced; published wording and parent ancestry survive',()=>{
 const a={id:'a',eventId:'e',handle:'@one',body:'Ready for football.',voice:'recruit'},b={...a,id:'b',handle:'@two'},reply={...a,id:'c',replyTo:'b',body:'Me too'};
 assert.deepEqual(deduplicateConversations([a,b,reply]).map(p=>p.id),['a']);
 assert.deepEqual(deduplicateConversations([a,b,reply],[],new Set(['a','b','c'])),[a,b,reply]);
 assert.equal(deduplicateConversations([a,{...b,essentialNews:true}]).length,2);
});
test('a shared game thread bounds routine flavor while essential and distinct production survive',()=>{
 const es=Array.from({length:9},(_,i)=>({id:'e'+i,thread:'game:42',facts:{}}));const ps=es.map((e,i)=>({id:'p'+i,eventId:e.id,body:'Different perspective '+i,voice:i%2?'fan':'official',category:'schedule'}));
 const essential={...ps[0],id:'essential',essentialNews:true,category:'results'},stats={...ps[0],id:'stats',body:'Player One: 100 receiving yards.',category:'player-stats'};
 const got=deduplicateConversations([essential,...ps,stats],es);assert.equal(got.length,5);assert.ok(got.includes(essential)&&got.includes(stats));
 assert.equal(deduplicateConversations(ps,es,new Set(ps.map(p=>p.id))).length,9);
});
test('all milestone variants state thresholds once, with correct distinct totals and units',()=>{
 for(const family of ['history.coach-wins','history.team-wins','history.player-career'])for(const total of [150,151]){
  const values={school:'Georgia',coachName:'Kirby Smart',playerName:'Player One',threshold:150,total,metric:'passing touchdowns'};values.totalDetail=milestoneTotalDetail(family,values);
  for(const t of contentPack.families[family]){const body=t.body.replace(/\{(\w+)\}/g,(_,k)=>values[k]);assert.equal((body.match(/\b150\b/g)??[]).length,1,t.id+': '+body);assert.equal(body.includes('151'),total===151);assert.ok(!/surpassed|beyond|past 150/.test(body));assert.ok(body.length<=280);}
 }
 assert.equal(milestoneTotalDetail('history.player-career',{threshold:'1,000',total:'1,001',metric:'rushing yards'}),'');assert.match(milestoneTotalDetail('history.player-career',{threshold:'1,000',total:'1,050',metric:'rushing yards'}),/1,050 rushing yards/);
 assert.equal(milestoneTotalDetail('history.coach-wins',{threshold:150,total:149}),'');
 const v={school:'Georgia',coachName:'Kirby Smart',threshold:150,total:150,totalDetail:''};assert.ok(chooseTemplate('history.coach-wins',v,'fixture').body.includes('150'));
 const long={...v,school:'Long School '.repeat(7).trim(),coachName:'Long Coach '.repeat(7).trim(),totalDetail:''};assert.ok(chooseTemplate('history.coach-wins',long,'long').body.length<=280);
});
test('milestone detection still reports the right crossing, total and coach attribution',()=>{
 const s={season:{CurrentStage:'NFLSeason',CurrentSeasonYear:2026},teams:[{id:1,name:'Georgia',headCoach:{id:'coach',name:'Kirby Smart',careerStats:{Wins:150}}}],players:[],milestoneBaseline:{version:1,year:2026,teams:{},players:{},coaches:{coach:{team:1,Wins:149}}}};
 for(const total of [150,151]){s.teams[0].headCoach.careerStats.Wins=total;const e=historyMilestoneEvents(s).find(e=>e.facts.family==='history.coach-wins');assert.equal(e.facts.threshold,150);assert.equal(e.facts.total,total);assert.equal(e.facts.coachName,'Kirby Smart');}
 s.milestoneBaseline.coaches.coach.Wins=150;assert.equal(historyMilestoneEvents(s).length,0);
 s.teams[0].headCoach.careerStats.Wins=149;s.milestoneBaseline.coaches.coach.Wins=148;assert.equal(historyMilestoneEvents(s).length,0);
});
