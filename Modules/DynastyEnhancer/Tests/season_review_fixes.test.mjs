import test from 'node:test';
import assert from 'node:assert/strict';
import {sameLeague,canonicalLeagueId} from '../Tools/league_identity.mjs';
import {teamRecord} from '../Tools/team_record.mjs';
import {proofreadNewText} from '../Tools/prose_cleanup.mjs';
import {weeklyBudget,enforceWeeklyBudget} from '../Tools/weekly_budget.mjs';
import {editorialRoots,conversationOrder,buildCatalog,publishWeek} from '../Tools/feed_selection.mjs';
import {deduplicateConversations} from '../Tools/conversation_topics.mjs';
import {contentPack,chooseTemplate} from '../Tools/story_templates.mjs';
import {renderStories} from '../Tools/story_renderer.mjs';
import {pffEvents,pffObservation} from '../Tools/pff_events.mjs';
test('native 32-bit LeagueID aliases match without broadening Dynasty isolation',()=>{
 assert.ok(sameLeague(-1552061962,2742905334));assert.equal(canonicalLeagueId(-1552061962),2742905334);
 for(const id of [undefined,null,'2742905334',NaN,Infinity,2742905334.5,-1552061963,7037872630,-5847029258])assert.ok(!sameLeague(-1552061962,id));
 assert.ok(!sameLeague(null,undefined));
});
test('native totals include neutral wins, losses and ties, with bounded old-capture fallback',()=>{
 assert.deepEqual(teamRecord({HomeWin:4,RoadWin:4,HomeLoss:2,RoadLoss:1},{WINS:9,LOSSES:3,TIES:0}),{wins:9,losses:3,ties:0});
 assert.deepEqual(teamRecord({HomeWin:1,RoadWin:2,NeutralWin:1,NeutralLoss:2,NeutralTie:1}),{wins:4,losses:2,ties:1});
 assert.deepEqual(teamRecord({HomeWin:0,RoadWin:0},{WINS:-1,LOSSES:0,TIES:0}),{wins:0,losses:0,ties:0});
});
test('new wording agrees with zero/one/many while preserving units, names, scores and handles',()=>{
 assert.equal(proofreadNewText('0 points, 1 points, 2 points. 1 yards, 1 games, 1 losses.'),'0 points, 1 point, 2 points. 1 yard, 1 game, 1 loss.');
 assert.equal(proofreadNewText('With a 11–1 record and a $80K change. The CFP rankings has Ohio State.'),'With an 11–1 record and an $80K change. The CFP rankings have Ohio State.');
 for(const body of ['21 points','11 points','1.1 points','11–1 points','@OnePoints 1-point win','A 110–1 record','a $100K change'])assert.equal(proofreadNewText(body),body);
});
test('one verified calendar root survives exhausted routine budgets without increasing the routine limit',()=>{
 const budget=weeklyBudget(),routine={teamIds:[1],category:'analytics',facts:{}};
 for(let i=0;i<15;i++)budget.record(routine);assert.ok(!budget.allows(routine,'beat'));
 const holiday={...routine,id:'calendar:2026:halloween',category:'holiday'};assert.ok(budget.allows(holiday,'fan'));
 const post={id:'holiday',eventId:holiday.id,teamIds:[1],category:'holiday',voice:'fan'};
 assert.ok(enforceWeeklyBudget([post],[holiday]).includes(post));
});
const post=(id,fields={})=>({id,priority:150,category:'matchup-preview',voice:'analyst',thread:'same-game',scopes:['national'],...fields});
test('different desks making the same pick cannot bury the actual local result',()=>{
 const roots=[1,2,3].map(i=>post('pick'+i,{handle:'@desk'+i,templateFamily:'buzz.pick-'+i}));
 roots.push(post('own-result',{category:'results',priority:112,scopes:['team'],handle:'@local',thread:'own-game'}));
 const result=editorialRoots(roots);assert.equal(result[0].id,'own-result');assert.equal(result.length,4);
});
test('draft announcements keep numerical pick order and real replies keep their parent',()=>{
 const roots=[9,224,8,10,4].map(p=>post('pick'+p,{voice:'nflDraft',category:'draft',draftPick:p,handle:'@draft',thread:'draft'+p}));
 roots.push(post('reply',{replyTo:'pick8',category:'reply'}));
 const result=conversationOrder(roots);assert.deepEqual(result.filter(p=>p.voice==='nflDraft').map(p=>p.draftPick),[4,8,9,10,224]);assert.equal(result.findIndex(p=>p.id==='reply'),result.findIndex(p=>p.id==='pick8')+1);
});
test('same fan does not repeat a prediction reaction beneath every desk in a matchup',()=>{
 const pool=[post('a'),post('b'),post('r1',{replyTo:'a',voice:'fan',handle:'@fan',templateFamily:'buzz.pick-reply',body:'I disagree'}),post('r2',{replyTo:'b',voice:'fan',handle:'@fan',templateFamily:'buzz.pick-reply',body:'Respectfully no'})];
 assert.deepEqual(deduplicateConversations(pool).map(p=>p.id),['a','b','r1']);
});
test('catalog uses immutable published selections and does not rerank prior editions on content upgrades',()=>{
 const f=publishWeek({format:1,engineVersion:2,leagueId:1,userTeamIndex:1,season:2026,phase:'RegularSeason',week:1,key:'1/2026/RegularSeason/1',events:[],stories:[post('old',{category:'results'})]});
 f.views.national=['old'];const saved=structuredClone(f);const catalog=buildCatalog(f);
 assert.deepEqual(catalog.weeks[0].views,f.views);assert.deepEqual(f,saved);
});
test('finite corrections preserve IDs, guarded family metadata and good rivalry humor',()=>{
 const lines=Object.values(contentPack.families).flat();assert.equal(new Set(lines.map(t=>t.id)).size,lines.length);
 assert.equal(Object.keys(contentPack.families).length,594,'Existing coverage families disappeared');assert.equal(lines.length,8037,'A wording correction removed or added variants');
 assert.ok(lines.some(t=>/Please respect our privacy/.test(t.body)));
 assert.ok(!lines.some(t=>/I said \{pick\}, and I meant it|\{area\} improved for|he \{lead\}\./.test(t.body)));
});
test('every leader alternative composes sole and tied leaders without a dangling or duplicated preposition',()=>{
 for(const [family,lines] of Object.entries(contentPack.families).filter(([f])=>/^analysis\.player\..*-leader$/.test(f)))for(const lead of ['leads','shares the lead in']){
  let candidates;chooseTemplate(family,{school:'Ohio State',playerName:'Julian Sayin',field:'the Big Ten',value:15,metricLabel:'passing TD',lead,games:7},'proofread',{onCandidates:r=>candidates=r.candidates});
  assert.equal(candidates.length,lines.length,family);
  for(const c of candidates){assert.ok(!/\bin in\b|\blead in[.,;:]|\bhe leads[.,;:]|which shares the lead in the group|for Ohio State, (?:leads|shares)/i.test(c.body),c.body);}
 }
});
test('PFF accepts native signed/unsigned attachment, then suppresses unchanged samples',()=>{
 const s={leagueId:-1552061962,userTeamIndex:1,season:{CurrentYear:0,CurrentSeasonYear:2026,CurrentWeek:8,CurrentWeekType:'RegularSeason',CurrentStage:'NFLSeason'},teams:[{id:1,name:'Ohio State'}],players:[{id:'p1',team:1,name:'Longfirstname Longlastname-Suffix',position:'LT'}]};
 s.playerAnalytics={format:1,modelVersion:'0.2.0-production',leagueId:2742905334,year:0,season:2026,week:7,weekType:'RegularSeason',weekly:[],seasonRows:[{player:'p1',team:1,group:'OL',grade:89.4,snaps:600,gradedGames:7}]};
 assert.ok(pffEvents(s).length);const prior={playerAnalytics:pffObservation(s)};assert.deepEqual(pffEvents(s,prior),[]);
 for(const key of ['year','season','leagueId']){const bad=structuredClone(s);bad.playerAnalytics[key]++;assert.deepEqual(pffEvents(bad),[]);}
});
