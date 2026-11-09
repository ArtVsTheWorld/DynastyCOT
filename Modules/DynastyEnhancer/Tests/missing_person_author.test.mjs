import test from 'node:test';
import assert from 'node:assert/strict';
import {authorRegistry} from '../Tools/story_authors.mjs';
import {renderStories} from '../Tools/story_renderer.mjs';

const fixture=()=>({format:2,gameYear:27,sourceKind:'runtime',sourceUnchanged:true,sha256:'a'.repeat(64),leagueId:7,userTeamIndex:1,source:'runtime:7:1',season:{CurrentYear:0,CurrentSeasonYear:2026,CurrentWeek:15,CurrentWeekType:'RegularSeason',CurrentStage:'NFLSeason'},teams:[{id:1,name:'Harbor',logo:1,nickname:'Captains'}],players:[{id:'r',name:'Evan Jones',firstName:'Evan',position:'QB',team:255,starRating:5,personality:'Neutral'}],games:[],performances:[],offers:[],commitments:[],heisman:[],recruitStatuses:[{player:'r',stage:'SoftCommitted'}]});
const event=()=>({id:'commit:1:r',category:'recruiting',teamIds:[1],playerId:'r',scopes:['team','national'],priority:90,thread:'commit:1:r',facts:{newlyObserved:true,commitment:'SoftCommitted',stars:5,school:'Harbor',playerName:'Evan Jones',position:'QB',recruitKind:'high school',prospectType:'',originDetail:'',nationalRecruitRank:10}});

test('missing personal subjects cannot crash the registry or create fictional account IDs',()=>{
 const s=fixture(),r=authorRegistry();r.get(s,'recruit',s.teams[0],s.players[0]);const before=structuredClone(r.entries);
 for(const role of ['player','recruit'])for(const p of [null,undefined,{}, {id:null,name:'Missing'}, {id:'',name:'Missing'}, {id:'x',name:null}, {id:'x',name:'  '}])assert.equal(r.get(s,role,null,p),null);
 assert.deepEqual(r.entries,before);
 assert.ok(r.get(s,'player',null,{id:0,name:'Zero Record',team:1}).handle);
});

test('teamless national desks still work without a player',()=>{
 const s=fixture(),r=authorRegistry();for(const role of ['analyst','model','cfp','nflDraft'])assert.ok(r.get(s,role,null,null).handle);
 assert.ok(Object.keys(r.entries).every(k=>!k.includes(':person:')));
});

test('a saved recruit voice revisited through the report branch resolves the actual event player',()=>{
 const s=fixture(),e=event(),initial=renderStories(s,[e]),main=initial.stories.find(p=>p.id===e.id+':main');assert.ok(main);
 const registry=authorRegistry(initial.authors),personal=registry.get(s,'recruit',s.teams[0],s.players[0]);
 const archived={...main,voice:'recruit',author:personal.author,handle:personal.handle,playerId:'r'};
 const sameWeek={leagueId:s.leagueId,season:s.season,events:[e],stories:[archived]},before=structuredClone(sameWeek),authors=structuredClone(registry.entries);
 const result=renderStories(s,[e],{authors,sameWeek});
 assert.deepEqual(result.stories.find(p=>p.id===archived.id),archived);
 assert.deepEqual(sameWeek,before);assert.deepEqual(registry.entries,authors);
 assert.deepEqual(renderStories(s,[e],{authors,sameWeek}),result);
 assert.ok(Object.keys(result.authors).every(k=>!k.endsWith(':person:undefined')&&!k.endsWith(':person:null')));
});

test('a missing recruit record leaves the remaining feed and saved identities usable',()=>{
 const s=fixture(),e=event(),registry=authorRegistry();registry.get(s,'recruit',s.teams[0],s.players[0]);const authors=structuredClone(registry.entries);s.players=[];
 const poll={id:'poll:1',category:'polls',teamIds:[1],scopes:['team'],priority:80,thread:'poll:1',facts:{rank:9,previousRank:11,movement:'up',school:'Harbor',pollName:'AP poll'}};
 const result=renderStories(s,[e,poll],{authors});assert.ok(result.stories.some(p=>p.category==='polls'));
 assert.deepEqual(result.authors['7:person:r'],authors['7:person:r']);
});
