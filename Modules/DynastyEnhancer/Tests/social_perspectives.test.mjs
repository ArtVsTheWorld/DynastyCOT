import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import {chooseTemplate,contentPack,templateHistory} from '../Tools/story_templates.mjs';
import {rememberVariety,structure,templateFeatures} from '../Tools/story_variety.mjs';
import {accountVoice} from '../Tools/voice_style.mjs';
import {renderStories} from '../Tools/story_renderer.mjs';
import {buildFeed} from '../Tools/feed_engine.mjs';
import {freshEditorialEvents} from '../Tools/editorial_policy.mjs';
import {applyWeeklyCoverage} from '../Tools/weekly_coverage.mjs';
import {publishWeek} from '../Tools/feed_selection.mjs';
import {perspectiveId} from './SocialVariety/content_ids.mjs';
const pack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/perspectives-pack.json',import.meta.url),'utf8'));
const top20={'roster.commit':10,'world.upset':10,'beat.commit':10,'beat.performance':10,'team.upcoming':10,'world.award':10,'cycle.portal-target':10,'roster.interest':10,'roster.announce':10,'reply.fan.upset':20,'late.rivalry-team':10,'model.line':8,'portal.entrant':10,'beat.standings':10,'cycle.team-welcome':8,'reply.player.offer':10,'cycle.player-reset':10,'reply.fan.player_good':10,'buzz.watcher-player-Unpredictable':10,'history.coach-counter':10};
const values={school:'Harbor State',nickname:'Captains',playerName:'Evan Moss',position:'QB',stars:5,starDescription:'5-star ',starDetail:'5-star ',prospectType:'high school ',originDetail:' from Texas',recruitRankDetail:'No. 12 nationally.',coachDetail:'',parentHandle:'@Reporter',parentDetail:'Harbor State',tagHandle:'@EvanMoss',hashtag:'#Captains',schools:'Harbor State, Valley and Mesa',winner:'Harbor State',loser:'Mesa',winnerLabel:'Unranked Harbor State',loserLabel:'No. 3 Mesa',score:'31–28',opponent:'Mesa',statLine:'250 passing yards and 2 TD',resultLine:'A team win to go with it.',award:'Offensive Player of the Week',awardWeek:8,favorite:'Harbor State',spread:3.5,conference:'Coastal',leaders:'Harbor State, Valley and Mesa',home:'Harbor State',away:'Mesa',eventLabel:'National Championship',coachName:'Jordan Wells',ordinal:'100th',achievement:'win',rank:12,previousRank:15,pollName:'AP poll',nationalRecruitRank:12,record:'8–4',expectationNote:'A winning season.',count:3,seniorLabel:'seniors',passingYards:250,rushingYards:120,firstDowns:20,giveaways:1,giveawayLabel:'giveaway',watchDetail:'',draftPick:35,draftRound:2,roundPick:3,nflTeam:'Bears',rankChange:'rises to No. 3',formerSchool:'Valley',turningPoint:'Harbor State led 14–7 at halftime.',detail:'Evan Moss: 250 passing yards.',recordContext:'8–4'};
test('historically displayed and current baseline top-twenty families have at least three times the usable structures',()=>{
 const reviewed={...top20,'cycle.signed-spotlight':8,'portal.unsigned-local':10,'beat.poll_same':10,'injury.report':10,'player.performance.Entertainer':10,'team.win':20,'history.player-career':10};
 const complete={...values,injuryDetail:' Reported issue: lower body.',timetable:'Estimated return: 2–3 weeks.',threshold:1000,total:1100,metric:'passing yards',rankMovement:'Unchanged at No. 12.'};
 for(const [family,baseline] of Object.entries(reviewed)){let valid=[];chooseTemplate(family,complete,'audit',{onCandidates:x=>valid=x.candidates});assert.ok(new Set(valid.map(t=>structure(contentPack.families[family].find(x=>x.id===t.id).body))).size>=baseline*3,family);}
});
test('adding or reordering authored alternatives cannot renumber existing wording',()=>{
 const a={body:'{school} wins.'},b={body:'A win for {school}.'},extra={body:'Final: {school} wins.'};
 const original=new Map([a,b].map(t=>[t.body,perspectiveId('fixture',t)])),extended=[extra,b,a];for(const t of extended.filter(t=>t!==extra))assert.equal(perspectiveId('fixture',t),original.get(t.body));assert.notEqual(perspectiveId('fixture',{...a,body:'Changed copy.'}),original.get(a.body));
});
test('new conditional perspectives require their evidence and missing context leaves a factual shorter alternative',()=>{
 let candidates=[];const v={winner:'Harbor',loser:'Mesa',score:'31–28',detail:'',turningPoint:null};const chosen=chooseTemplate('narrative.game',v,'sparse',{onCandidates:x=>candidates=x.candidates});assert.ok(chosen);assert.ok(!candidates.some(c=>contentPack.families['narrative.game'].find(t=>t.id===c.id)?.when?.length));assert.ok(!/undefined|null|interview|source says/.test(chosen.body));
 const minimal=chooseTemplate('roster.commit',{school:'Harbor',playerName:'Evan Moss',position:'QB'},'minimal');assert.ok(minimal);assert.match(minimal.body,/Harbor/);assert.match(minimal.body,/Evan Moss/);
});
test('a team giveaway detail is never described as an individual player performance',()=>{
 const s=fixture(),facts={gameId:'g',winner:'Harbor State',loser:'Mesa',home:'Harbor State',away:'Mesa',margin:3,tie:false,gameStory:{winner:'Harbor State',loser:'Mesa',winnerId:1,loserId:2,score:'31–28',turningPoint:'Harbor State led 17–14 at halftime.',detail:'Giveaways: Harbor State 1, Mesa 2.',player:'p'}};
 const e={id:'result:g',category:'results',facts,playerId:null,teamIds:[1,2],scopes:['team','national'],priority:90,thread:'game:g'};let candidates=[];
 renderStories(s,[e],{onTemplates:x=>{if(x.family==='narrative.game')candidates=x.candidates;}});assert.ok(candidates.length);assert.ok(!candidates.some(c=>c.angle==='performance'));
 facts.gameStory.detail='Evan Moss: 250 passing yards.';renderStories(s,[e],{onTemplates:x=>{if(x.family==='narrative.game')candidates=x.candidates;}});assert.ok(candidates.some(c=>c.angle==='performance'));
});
test('long names retain news within both native budgets without changing facts',()=>{
 const long={...values,school:'Northwestern Metropolitan University of the Great Lakes',nickname:'Northwestern Metropolitan Captains',playerName:'Christopher Alexander Montgomery-Washington III',opponent:'Southwestern Polytechnic Institute of Technology',winnerLabel:'No. 22 Northwestern Metropolitan University',loserLabel:'No. 3 Southwestern Polytechnic Institute'};
 const complete={...long,injuryDetail:' Reported issue: lower body.',timetable:'Estimated return: 2–3 weeks.',threshold:1000,total:1100,metric:'passing yards',rankMovement:'Unchanged at No. 12.'};
 for(const family of [...Object.keys(top20),'injury.report','cycle.signed-spotlight','portal.unsigned-local','beat.poll_same','player.performance.Entertainer','team.win','history.player-career']){const p=chooseTemplate(family,complete,'long');assert.ok(p,family);assert.ok([...p.body].length<=280&&Buffer.byteLength(p.body)<=512,family);assert.ok(!p.body.includes('undefined'));}
});
test('global repetition memory crosses accounts, families, season boundaries and stays Dynasty-isolated',()=>{
 const a=chooseTemplate('world.upset',values,'same'),publication={leagueId:10,userTeamIndex:0,season:2028,phase:'PreSeason',week:0};
 const previous={...publication,season:2027,phase:'OffSeason',week:8,stories:[{...a,handle:'@Other'}]},usage=templateHistory([previous],publication,0);
 assert.ok(usage.memory.forms.has(a.variety.structure));const b=chooseTemplate('world.upset',values,'same',{usage,history:usage.history,publication,week:0,handle:'@New'});assert.notEqual(a.variety.structure,b.variety.structure);
 const foreign=templateHistory([{...previous,userTeamIndex:1}],publication,0);assert.equal(foreign.memory.forms.size,0);
 rememberVariety(foreign.memory,a.variety);assert.ok(foreign.memory.forms.has(a.variety.structure));
});
test('exhausted wording remains publishable and account character preferences are stable and substantive',()=>{
 const all=new Set();chooseTemplate('world.upset',values,'all',{onCandidates:x=>x.candidates.forEach(c=>all.add(c.body))});assert.ok(chooseTemplate('world.upset',values,'all',{avoidBodies:all}));
 const voices=new Set(Array.from({length:15},(_,styleSeed)=>accountVoice({styleSeed},'fan')));assert.equal(voices.size,5);assert.equal(accountVoice({styleSeed:1},'beat'),'local');assert.equal(accountVoice({styleSeed:999},'analyst',null,3),'evidence');
 const picks=new Set();for(const voice of voices)picks.add(chooseTemplate('reply.fan.upset',values,'voice',{voice}).body);assert.ok(picks.size>=3);
});
test('an unseen near-copy does not beat a structurally unused alternative merely because its ID is new',()=>{
 const family='test.near-copy',a={id:'old-form',body:'{school} records {score} against {opponent}. A team win.'},b={id:'new-id',body:'{school} records {score} against {opponent}. A team win tonight.'},c={id:'different',body:'Against {opponent}, the final belongs to {school}: {score}.'};
 contentPack.families[family]=[b,c];const oldFamily='test.near-source';contentPack.families[oldFamily]=[a];
 try{const used=chooseTemplate(oldFamily,values,'a'),publication={leagueId:10,userTeamIndex:1,season:2027,phase:'RegularSeason',week:2},history=[{...publication,week:1,stories:[{...used,handle:'@other'},{handle:'@writer',templateId:'different'}]}];
  // Include the reference form in the family, as real packs retain old variants.
  contentPack.families[family].push(a);const selected=chooseTemplate(family,values,'same',{publication,week:2,history,handle:'@writer'});assert.equal(selected.templateId,'different');
 }finally{delete contentPack.families[family];delete contentPack.families[oldFamily];}
});
test('compact archived signatures and older full signatures rebuild identical wording memory',()=>{
 const a=chooseTemplate('world.upset',values,'storage'),t=contentPack.families['world.upset'].find(t=>t.id===a.templateId),full=templateFeatures(t);
 assert.ok(JSON.stringify(a.variety).length<JSON.stringify(full).length);
 const pub={leagueId:99,userTeamIndex:1,season:2027,phase:'RegularSeason',week:5};
 const make=variety=>[1,2,3,4].map(week=>({...pub,week,stories:week===1?[{...a,handle:'@old',variety}]:[]}));
 const old=make(full),compact=JSON.parse(JSON.stringify(make(a.variety))),one=templateHistory(old,pub,5),two=templateHistory(compact,pub,5);
 assert.deepEqual(one.memory.phrases,two.memory.phrases);assert.deepEqual(one.memory.forms,two.memory.forms);assert.ok(two.memory.forms.has(a.variety.structure));assert.equal(two.byHandle.has('@old'),false);
 assert.deepEqual(chooseTemplate('world.upset',values,'next',{history:old,publication:pub,week:5,handle:'@new',usage:one}),chooseTemplate('world.upset',values,'next',{history:compact,publication:pub,week:5,handle:'@new',usage:two}));
});
const fixture=(brand='B')=>({format:2,gameYear:27,sourceKind:'runtime',source:'fixture',sourceUnchanged:true,sha256:'a'.repeat(64),leagueId:99,userTeamIndex:1,
 season:{CurrentYear:0,CurrentSeasonYear:2027,CurrentWeekType:'RegularSeason',CurrentStage:'NFLSeason',CurrentWeek:4},
 teams:[{id:1,name:'Harbor State',nickname:'Captains',brandExposure:brand,mediaRank:8,logo:1,rivals:[2],wins:3,losses:1},{id:2,name:'Mesa',nickname:'Hawks',brandExposure:'B',logo:2,rivals:[1],wins:2,losses:2}],
 players:[{id:'p',name:'Evan Moss',firstName:'Evan',team:1,position:'QB',personality:'Leader',injury:'Uninjured'}],games:[{id:'g',year:0,week:3,phase:'RegularSeason',home:1,away:2,homeScore:31,awayScore:28,published:true,status:'HomeWon'}],performances:[],offers:[],commitments:[],heisman:[]});
test('new publications are deterministic and upgrading copy cannot rewrite already published cards',()=>{
 const s=fixture(),a=buildFeed(s),old=structuredClone(a);for(const p of old.stories)p.contentRevision='previous-published-content';const bytes=JSON.stringify(old);
 const again=renderStories(s,a.events,{sameWeek:old});assert.deepEqual(again.stories,old.stories);assert.equal(JSON.stringify(old),bytes);
 assert.deepEqual(buildFeed(structuredClone(s)),a);
});
test('three program sizes retain coverage and deterministic refresh over every phase, including sparse editions',()=>{
 for(const brand of ['F','B','A+']){const history=[];const phases=[['PreSeason',0],...Array.from({length:17},(_,w)=>['RegularSeason',w]),['BowlSeason1',17],['BowlSeason2',18],['BowlSeason3',19],['NationalChampionship',20],...Array.from({length:9},(_,w)=>['OffSeason',w])];
  for(const [phase,week] of phases){const s=fixture(brand);s.season.CurrentWeekType=phase;s.season.CurrentWeek=week;s.games=[];s.players=week%2?[]:s.players;const a=buildFeed(s,null,{history}),b=buildFeed(s,null,{history});assert.deepEqual(a,b);assert.ok(a.posts.length<=30);for(const p of a.stories){assert.ok([...p.body].length<=280&&Buffer.byteLength(p.body)<=512);if(p.replyTo)assert.ok(a.stories.some(x=>x.id===p.replyTo));assert.ok(!/undefined|NaN/.test(p.body));}history.push(a);}
 }
});
test('developing storylines require a changed supported state, independent of extra copy',()=>{
 const pub={leagueId:99,userTeamIndex:1,season:2027,phase:'OffSeason',week:1},e={id:'portal-interest',teamIds:[1],category:'transfer',playerId:'p',priority:90,facts:{family:'cycle.portal-target',school:'Harbor',schools:'Harbor, Mesa, Valley'}};
 const history=[{...pub,week:0,events:[e],stories:[{eventId:e.id}]}];assert.deepEqual(freshEditorialEvents([e],pub,history),[]);assert.equal(freshEditorialEvents([{...e,facts:{...e.facts,schools:'Harbor, Valley, City'}}],pub,history).length,1);
});
test('dense verified honors survive flavor exhaustion without extending native default view requirements',()=>{
 const s=fixture('F');s.players=Array.from({length:35},(_,i)=>({id:'honor'+i,name:'Prospect '+i,team:1,position:'CB',injury:'Uninjured',personality:'Neutral'}));
 const events=s.players.map((p,i)=>({id:'honor:'+i,category:'awards',teamIds:[1],playerId:p.id,scopes:['team','national'],priority:84,thread:'honor:'+i,facts:{family:'preseason.honor.beat',preseason:true,school:'Harbor State',playerName:p.name,award:'first-team preseason All-Coastal'}}));
 const rendered=renderStories(s,events),kept=applyWeeklyCoverage(rendered.stories,s);assert.equal(kept.filter(p=>!p.replyTo&&p.voice==='beat').length,35);
 const feed=publishWeek({format:1,key:'99/2027/PreSeason/0',leagueId:99,userTeamIndex:1,season:2027,phase:'PreSeason',week:0,events,stories:kept});assert.ok(feed.posts.length<=30);assert.equal(feed.views['team:required'].length,0);assert.ok(feed.stories.filter(p=>p.essentialNews).every(p=>p.body.includes('All-Coastal')));
});
test('authored perspectives contain no attributed sourcing or unsafe literal stats; injury language stays in injury events',()=>{
 for(const [family,lines] of Object.entries(pack.families))for(const t of lines){assert.ok(!/sources? (?:say|tell)|in an interview|told me|\b\d+ (?:yards|TD|wins|losses)\b/i.test(t.body),t.id);if(!t.id.startsWith('injury.report.'))assert.ok(!/injur(?:ed|y)/i.test(t.body),t.id);
  if(['cycle.team-welcome','cycle.signed-spotlight'].includes(family))assert.ok(t.body.includes('{tagHandle}'),t.id);
  if(family==='roster.major'){assert.equal((t.body.match(/5-star/g)??[]).length,1,t.id);assert.equal((t.body.match(/\{position\}/g)??[]).length,1,t.id);}
 }
});
