import test from 'node:test';
import assert from 'node:assert/strict';
import {pffRankings} from '../Tools/pff_rankings.mjs';
import {pffEvents} from '../Tools/pff_events.mjs';
import {detectEvents} from '../Tools/story_events.mjs';
import {renderStories} from '../Tools/story_renderer.mjs';
import {freshEditorialEvents} from '../Tools/editorial_policy.mjs';
import {chooseTemplate,contentPack} from '../Tools/story_templates.mjs';
import {nativeViewIds,NATIONAL_CARD_LIMIT} from '../Tools/feed_visibility.mjs';
export function rankingFixture(){
 const s={format:2,gameYear:27,sourceUnchanged:true,sha256:'a'.repeat(64),source:'runtime:10:1',sourceKind:'runtime',leagueId:10,userTeamIndex:2,
  season:{CurrentSeasonYear:2027,CurrentYear:1,CurrentWeek:8,CurrentWeekType:'RegularSeason',CurrentStage:'NFLSeason'},
  teams:[1,2,3,4,5].map(id=>({id,name:'School '+id,shortName:'S'+id,logo:id,rivals:[],mediaRank:99,mediaLastRank:99})),
  players:[],games:[],performances:[],heisman:[],offers:[],commitments:[],recruitStatuses:[]};
 const specs=[['QB','QB'],['HB','RB'],['WR','WR'],['WR','WR'],['WR','WR'],['TE','TE'],['LT','OL'],['LG','OL'],['C','OL'],['RG','OL'],['RT','OL'],['LEDG','DL'],['REDG','DL'],['DT','DL'],['MIKE','LB'],['WILL','LB'],['CB','CB'],['CB','CB'],['FS','S'],['SS','S']];
 const a={format:1,modelVersion:'0.2.0-production',leagueId:10,year:1,season:2027,week:6,weekType:'RegularSeason',weekly:[],seasonRows:[],coverage:{format:1,fbsTeams:[1,2,3,4,5],weeklyTeams:[1,2,3,4,5],trackedPositions:[...new Set(specs.map(r=>r[0]))],positions:[...new Set(specs.map(r=>r[0]))],scheduleComplete:true}};
 for(const team of [1,2,3,4,5])for(const [i,[position,group]] of specs.entries()){
  const player=`p${team}-${i}`,off=i<11,grade=off?(team===1?88:60+team):(team===3?86:60+team);
  s.players.push({id:player,name:`Player${team} Surname${i}`,firstName:`Player${team}`,lastName:`Surname${i}`,position,team,personality:'TeamPlayer',injury:'Uninjured'});
  const r={player,team,position,group,grade:team===1&&i===0?99:team===3&&i===16?98:grade,snaps:60,opportunities:60};
  a.weekly.push(r);a.seasonRows.push({...r,grade:70+team*3,snaps:500,opportunities:500,gradedGames:6});
 }
 s.playerAnalytics=a;return s;
}
test('position Top 3 contains exact eligible players/grades and weekly players are unique highest qualifiers',()=>{
 const s=rankingFixture(),events=pffRankings(s,s.playerAnalytics),board=events.find(e=>e.facts.family==='pff.position-top-three');assert.ok(board);
 assert.equal(board.facts.position,'RG');assert.deepEqual(board.facts.leaderboard.map(r=>r.team),[5,4,3]);assert.equal(board.facts.topThree.split('\n').length,3);
 assert.equal(events.find(e=>e.facts.family==='pff.offensive-player-of-week').options.player,'p1-0');
 assert.equal(events.find(e=>e.facts.family==='pff.defensive-player-of-week').options.player,'p3-16');
 const saved=structuredClone(s);assert.deepEqual(pffRankings(s,s.playerAnalytics),events);assert.deepEqual(s,saved);
});
test('unit weeks use snap-weighted complete lineups and never promote an incomplete runner-up',()=>{
 const s=rankingFixture();s.playerAnalytics.week=7;const events=pffRankings(s,s.playerAnalytics);
 const off=events.find(e=>e.facts.family==='pff.offense-of-week'),def=events.find(e=>e.facts.family==='pff.defense-of-week');assert.equal(off.options.team,1);assert.equal(def.options.team,3);
 assert.equal(off.facts.teamGrade,'89.0');assert.equal(def.facts.teamGrade,'87.3');
 const bad=structuredClone(s);bad.playerAnalytics.weekly=bad.playerAnalytics.weekly.filter(r=>!(r.team===5&&r.position==='LT'));
 assert.ok(pffRankings(bad,bad.playerAnalytics).every(e=>e.facts.family!=='pff.offense-of-week'));
});
test('ties are labeled inside a three-player board, but an ambiguous cutoff/winner is omitted',()=>{
 const s=rankingFixture();s.playerAnalytics.coverage.positions=['RG'];s.playerAnalytics.seasonRows.forEach(r=>{if(r.position==='RG'&&[4,5].includes(r.team))r.grade=90;});
 let events=pffRankings(s,s.playerAnalytics),board=events.find(e=>e.facts.family==='pff.position-top-three');assert.match(board.facts.topThree,/T1\./);
 s.playerAnalytics.seasonRows.find(r=>r.position==='RG'&&r.team===2).grade=79;
 assert.ok(pffRankings(s,s.playerAnalytics).every(e=>e.facts.family!=='pff.position-top-three'));
 const tied=rankingFixture();tied.playerAnalytics.weekly.find(r=>r.player==='p2-0').grade=99;tied.playerAnalytics.weekly.find(r=>r.player==='p4-16').grade=98;
 assert.ok(pffRankings(tied,tied.playerAnalytics).every(e=>!['pff.offensive-player-of-week','pff.defensive-player-of-week'].includes(e.facts.family)));
});
test('unverified/missing/duplicate/foreign data cannot create national grade honors',()=>{
 for(const change of [a=>delete a.coverage,a=>a.coverage.scheduleComplete=false,a=>a.coverage.fbsTeams.push(77),a=>a.coverage.weeklyTeams=[],a=>a.weekly.push(a.weekly[0]),a=>a.seasonRows[0].team=255,a=>a.weekly[0].grade=null]){
  const s=rankingFixture();change(s.playerAnalytics);assert.deepEqual(pffRankings(s,s.playerAnalytics),[]);
 }
 const s=rankingFixture();s.playerAnalytics.coverage.positions=s.playerAnalytics.coverage.positions.filter(p=>p!=='LT');
 assert.ok(pffRankings(s,s.playerAnalytics).every(e=>!['pff.offensive-player-of-week','pff.offense-of-week'].includes(e.facts.family)));
 s.playerAnalytics.week=s.season.CurrentWeek;assert.deepEqual(pffEvents(s),[]);
});
test('structured posts share the four-root budget, preserve all related teams, and do not become native awards',()=>{
 const s=rankingFixture(),events=detectEvents(s).filter(e=>e.facts.family?.startsWith('pff.')),posts=renderStories(s,events).stories;
 assert.ok(events.length<=4&&posts.length<=4);assert.ok(events.some(e=>e.facts.family==='pff.position-top-three'));
 for(const p of posts){assert.equal(p.author,'PFF');assert.equal(p.handle,'@PFF_College');assert.equal(p.category,'analytics');assert.ok(!p.replyTo&&!p.essentialNews&&!p.coverageExempt);assert.ok([...p.body].length<=280&&Buffer.byteLength(p.body)<=512);}
 const board=events.find(e=>e.facts.family==='pff.position-top-three');assert.deepEqual(board.teamIds,[5,4,3]);
 const publication={leagueId:10,userTeamIndex:2,season:2027,phase:'RegularSeason',week:9},feed={...publication,week:8,events,stories:posts};
 assert.ok(!freshEditorialEvents(events,publication,[feed]).some(e=>e.facts.family==='pff.position-top-three'));
});
test('all 15 supplied proposal wordings have correct slots/voice and length-safe rendering',()=>{
 const values={playerName:'Christopher Alexander-Washington III',school:'Western Metropolitan State University',position:'QB',grade:'98.1',teamGrade:'86.4',weekLabel:'National Championship Week 20',sampleLabel:'110 recorded snaps',topThree:'1. A. Smith (UT) 98.1\n2. B. Jones (OU) 96.3\n3. C. Reed (SMU) 95.0'};
 const families=['position-top-three','offensive-player-of-week','defensive-player-of-week','offense-of-week','defense-of-week'];
 for(const suffix of families){const family='pff.'+suffix;let candidates;assert.ok(chooseTemplate(family,values,'long',{voice:'evidence',onCandidates:x=>candidates=x.candidates}));assert.equal(candidates.length,3);for(const c of candidates)assert.ok([...c.body].length<=280&&Buffer.byteLength(c.body)<=512);assert.ok(contentPack.families[family].every(t=>t.voices.includes('evidence')));}
});
test('long Top 3 boards use native school abbreviations/initials; impossible boards are omitted intact',()=>{
 const s=rankingFixture();s.playerAnalytics.coverage.positions=['RG'];
 for(const t of s.teams)t.name='University of Western Metropolitan State with a Very Long Name';
 for(const p of s.players){p.firstName='ChristopherAlexander';p.lastName='WashingtonAnderson';p.name=p.firstName+' '+p.lastName;}
 const board=pffRankings(s,s.playerAnalytics).find(e=>e.facts.family==='pff.position-top-three');assert.ok(board);assert.match(board.facts.topThree,/C\. WashingtonAnderson \(S5\)/);
 for(const t of s.teams)delete t.shortName;
 assert.ok(pffRankings(s,s.playerAnalytics).every(e=>e.facts.family!=='pff.position-top-three'));
});
test('narrow native positions retain explicit board labels instead of claiming a broader group lead',()=>{
 const s=rankingFixture();s.season.CurrentWeek=17;s.playerAnalytics.coverage.positions=['FS'];
 const board=pffRankings(s,s.playerAnalytics).find(e=>e.facts.family==='pff.position-top-three');assert.equal(board.facts.position,'Free Safety');
 const e=detectEvents(s).find(e=>e.facts.family==='pff.position-top-three'),posts=renderStories(s,[e]).stories;
 assert.ok(posts[0].body.includes('Free Safety'));assert.ok(!posts[0].body.includes('the S grade board'));
});
test('National fills 300, keeps all 224 draft picks, and carries relevant parent/reply chains',()=>{
 assert.equal(NATIONAL_CARD_LIMIT,300);const stories=[];
 for(let i=1;i<=224;i++)stories.push({id:'d'+i,voice:'nflDraft',category:'draft',draftPick:i,transaction:true,essentialNews:true,priority:80});
 for(let i=0;i<180;i++)stories.push({id:'n'+i,voice:'beat',category:'results',priority:80});
 stories.push({id:'reply',voice:'fan',category:'results',replyTo:'n0',priority:81});const feed={stories,views:{national:stories.map(p=>p.id)}};
 const ids=nativeViewIds(feed,'national');assert.equal(ids.length,300);for(let i=1;i<=224;i++)assert.ok(ids.includes('d'+i));if(ids.includes('reply'))assert.ok(ids.includes('n0'));
 assert.deepEqual(nativeViewIds(feed,'national'),ids);
});
