import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import {chooseTemplate,contentPack,templateHistory} from '../Tools/story_templates.mjs';
import {renderStories} from '../Tools/story_renderer.mjs';
import {publishWeek} from '../Tools/feed_selection.mjs';
import {nativeViewPosts,NATIONAL_CARD_LIMIT} from '../Tools/feed_visibility.mjs';
import {templateFeatures,varietyMemory,rememberVariety,globalNearUse} from '../Tools/story_variety.mjs';
import {upsetFixture,supplementalEditions} from './EditorialCandidate/fixtures.mjs';
const pack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/editorial011-pack.json',import.meta.url),'utf8'));
const templates=new Map(Object.entries(contentPack.families).flatMap(([f,ts])=>ts.map(t=>[t.id,{f,t}])));

test('one bounded editorial expansion: 120 primary alternatives and 30 targeted exceptions',()=>{
 assert.equal(Object.values(pack.families).flat().length,150);assert.equal(Object.keys(pack.families).length,30);
 for(const [f,ts] of Object.entries(pack.families).slice(0,20)){assert.equal(ts.length,6,f);assert.equal(new Set(ts.map(t=>templateFeatures(t).structure)).size,6,f);}
 assert.equal(new Set(Object.values(pack.families).flat().map(t=>t.id)).size,150);
 for(const patch of pack.overrides){assert.equal(templates.get(patch.id).f,patch.family);assert.equal(templates.get(patch.id).t.body,patch.body);assert.ok(patch.originalBody);}
 assert.equal(NATIONAL_CARD_LIMIT,250);
});
test('reviewed copy corrects opponent attribution, complete result sentences and timing',()=>{
 for(const t of contentPack.families['beat.performance']){assert.ok(!t.body.includes('{playerName} ({opponent})'));assert.ok(!/\{resultLine\}\s+vs\./.test(t.body));}
 for(const t of contentPack.families['analysis.team.fourth-quarter-losses'])assert.ok(!/15 minutes|beaten after four|fourth-quarter leads/.test(t.body),t.id);
 for(const t of contentPack.families['recruiting.catchup.commit'])assert.ok(t.body.includes('since Week {observedSinceWeek}'),t.id);
 const chronology=/\b(today|tonight|yesterday|tomorrow|Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday)\b/gi;
 for(const patch of pack.overrides){const old=new Set([...patch.originalBody.matchAll(chronology)].map(m=>m[0].toLowerCase()));assert.ok([...patch.body.matchAll(chronology)].every(m=>old.has(m[0].toLowerCase())),patch.id);}
});
test('average units and allocation-group meanings survive shortening',()=>{
 for(const t of contentPack.families['context.disruption']){if(t.body.includes('{intpg}'))assert.ok(t.body.includes('{intpg} INT/G')||t.body.includes('per game'));if(t.body.includes('{frpg}'))assert.ok(t.body.includes('{frpg} FR/G')||t.body.includes('recoveries/G')||t.body.includes('per game'));}
 for(const t of contentPack.families['context.ball-security']){if(t.body.includes('{rate}'))assert.ok(t.body.includes('pass attempts intercepted'));if(t.body.includes('{lostpg}'))assert.ok(t.body.includes('lost/G')||t.body.includes('Per game'));}
 for(const t of pack.families['context.nil'])if(t.body.includes('{first}')||t.body.includes('{second}'))assert.match(t.body,/group/);
});
test('proposal does not overwrite authored personality/style metadata or strong rivalry jokes',()=>{
 assert.equal(templates.get('player.win.Entertainer.voices.2').t.style,'measured');
 assert.match(templates.get('player.win.Entertainer.voices.2').t.body,/teammates/);
 assert.match(templates.get('context.upset-rival.rc016.32d0a3aba83ab92b').t.body,/no dignity/);
 assert.match(templates.get('reply.fan.upset-loss.rc016.433a00eeca6581bd').t.body,/spare me/);
 assert.ok(pack.families['reply.player.award'].every(t=>t.style==='measured'&&!/[!?]/.test(t.body)));
 assert.ok(pack.families['reply.fan.upset'].some(t=>/annoying|personality/.test(t.body)));
});
test('minimal context excludes unsupported angles, while long values get complete short facts',()=>{
 let options=[];const p=chooseTemplate('narrative.game',{winner:'WMU',loser:'Michigan',score:'31–26'},'minimal',{onCandidates:x=>options=x.candidates});assert.ok(p);assert.ok(!options.some(t=>t.id==='narrative.game.editorial011.2'||t.id==='narrative.game.editorial011.4'));
 const values={school:'Northwestern Metropolitan University of the Great Lakes',playerName:'Christopher Alexander Montgomery-Washington III',opponent:'Southwestern Polytechnic Institute of Technology',shortStatLine:'219 passing yards and 3 TD',statLine:'219 passing yards and 3 TD, 111 rushing yards and 2 TD',resultLine:'The individual numbers came in a team loss.',total:'$22.4M',first:'offensive line',firstMoney:'$7.2M',firstShare:32,second:'defensive line',secondMoney:'$5M',secondShare:22,ypa:7.3,ypg:290,tdpg:2.1,intpg:0.7,share:55,givepg:1.2,lostpg:0.5,rate:2.3,sackpg:2.8,frpg:0.4,takepg:1.1};
 for(const f of ['beat.performance','context.passing','context.ball-security','context.disruption','context.nil']){const chosen=chooseTemplate(f,values,'long');assert.ok(chosen,f);assert.ok([...chosen.body].length<=280&&Buffer.byteLength(chosen.body)<=512);assert.ok(!/undefined|null|\.\s+vs\./.test(chosen.body));}
});
test('WMU upset voices take different positions and replies engage with parent facts',()=>{
 const {snapshot,events}=upsetFixture(),rendered=renderStories(snapshot,events),posts=rendered.stories;
 assert.equal(posts.length,5);assert.deepEqual(new Set(posts.map(p=>p.templateFamily)),new Set(['world.upset','reply.fan.upset','reply.fan.upset-loss','context.upset-expectation','context.upset-rival']));
 const map=new Map(posts.map(p=>[p.id,p]));for(const p of posts){assert.ok([...p.body].length<=280&&Buffer.byteLength(p.body)<=512);assert.ok(!/interview|injury|source says|overtime|last.second/i.test(p.body));if(p.replyTo){const parent=map.get(p.replyTo);assert.ok(parent);assert.ok(['W. Michigan','Michigan','31–26'].some(f=>p.body.includes(f)&&parent.body.includes(f)));}}
 assert.equal(posts.find(p=>p.templateFamily==='reply.fan.upset-loss').teamIds.includes(49),true);
 assert.ok(new Set(posts.map(p=>p.body)).size===5);
});
test('upgrading copy preserves every published card and stable refresh identities',()=>{
 const {snapshot:s,events}=upsetFixture(),a=renderStories(s,events),old=structuredClone(a);old.stories=old.stories.map(p=>({...p,body:'Previously published wording '+p.id,contentRevision:'old-proposal'}));
 const sameWeek={events,stories:old.stories};const b=renderStories(s,events,{authors:old.authors,sameWeek});assert.deepEqual(b.stories,old.stories);assert.deepEqual(b.authors,old.authors);
 assert.deepEqual(renderStories(s,events,{authors:a.authors,sameWeek:{events,stories:a.stories}}).stories,a.stories);
 assert.deepEqual(renderStories(s,events),a);
 const publication={leagueId:s.leagueId,userTeamIndex:s.userTeamIndex,season:2026,phase:'RegularSeason',week:3};
 assert.equal(templateHistory([{...publication,leagueId:s.leagueId+1,week:2,stories:a.stories}],publication).byHandle.size,0);
});
test('supplemental facts stay bounded, orphan-free and marked when recruit games are mod-generated',()=>{
 for(const row of supplementalEditions()){
  const {snapshot:s,events}=row,unchanged=JSON.stringify(s),rendered=renderStories(s,events),feed=publishWeek({key:'test',season:2026,phase:row.phase,week:row.week,userTeamIndex:s.userTeamIndex,stories:rendered.stories,events});assert.equal(JSON.stringify(s),unchanged);
  assert.ok(feed.stories.length);assert.ok(nativeViewPosts(feed,'national').length<=250);
  for(const p of feed.stories){assert.ok([...p.body].length<=280&&Buffer.byteLength(p.body)<=512);if(p.category==='recruit-film'){assert.equal(p.stamp,'COT HS Game');assert.equal(p.recordSource,'mod-modeled-hs');}}
 }
});
test('visibility memory tracks the entire selected National window past retired cap 100',()=>{
 const stories=Array.from({length:300},(_,i)=>({id:String(i),category:'results',voice:'beat',priority:i,handle:'@'+i,templateId:'test'+i,variety:{structure:'unique '+i,opening:'open '+i,ending:'end '+i,angle:null}}));
 const history=[{leagueId:1,userTeamIndex:1,season:2026,phase:'RegularSeason',week:1,stories,views:{national:stories.map(p=>p.id),team:[],conference:[],all:[]}}];
 const usage=templateHistory(history,{leagueId:1,userTeamIndex:1,season:2026,phase:'RegularSeason',week:2});assert.equal(usage.memory.forms.size,250);assert.ok(usage.memory.forms.has('unique 299'));assert.ok(!usage.memory.forms.has('unique 0'));
});
test('cached near-shape comparisons still read increasing weights and newly added shapes',()=>{
 const m=varietyMemory(),a={body:'The whole team is ready to face {opponent} this week.'},b={body:'The whole team is ready to face {school} this week again.'};
 rememberVariety(m,templateFeatures(a),.1);assert.equal(globalNearUse(m,b),0);
 rememberVariety(m,templateFeatures(a),.15);assert.equal(globalNearUse(m,b),.25);
 rememberVariety(m,templateFeatures(b),1);assert.equal(globalNearUse(m,b),1.25);
 a.body='A completely different sentence about the program.';assert.equal(globalNearUse(m,a),0);
});
