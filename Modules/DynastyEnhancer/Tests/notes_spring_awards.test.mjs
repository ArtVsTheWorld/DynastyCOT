import test from 'node:test';
import assert from 'node:assert/strict';
import {contentPack} from '../Tools/story_templates.mjs';
import {renderStories} from '../Tools/story_renderer.mjs';
const snapshot=()=>({format:2,gameYear:27,leagueId:7,userTeamIndex:1,season:{CurrentSeasonYear:2026,CurrentYear:0,CurrentWeek:2,CurrentWeekType:'RegularSeason'},teams:[{id:1,name:'Ohio State',nickname:'Buckeyes',logo:1}],players:[{id:'p',name:'Jeremiah Smith',position:'WR',team:1,injury:'Uninjured'}],games:[],performances:[],heisman:[{player:'p',team:1,rank:1}],conferences:[],commitments:[]});
test('award field-dump variants become prose without changing rank meaning',()=>{
 const variant=contentPack.families['beat.heisman'].find(v=>v.id==='beat.heisman.cot104.af40af2dd878ca57');
 assert.equal(variant.body,'{playerName} of {school} {rankChange} on the Heisman watch.');
 for(const family of ['beat.heisman','preseason.honor.beat','world.award'])assert(!contentPack.families[family].some(v=>/^Heisman watch\n|^Preseason list\n|^Week .*award recipient\n/.test(v.body)));
});
test('Heisman sparse context validates watch rank, refresh and publication history',()=>{
 const s=snapshot(),event={id:'heisman:p',category:'heisman',playerId:'p',teamIds:[1],scopes:['national','team'],priority:88,thread:'h',facts:{school:'Ohio State',playerName:'Jeremiah Smith',rank:1}};
 const feed=renderStories(s,[event]);assert(feed.stories.length);assert(feed.stories.some(p=>p.body.includes('Heisman')));
 const stable=renderStories(s,[event],{authors:feed.authors,sameWeek:feed});assert.deepEqual(stable.stories,feed.stories);
 for(const rank of [undefined,0,6,1.5])assert.equal(renderStories(s,[{...event,facts:{...event.facts,rank}}]).stories.length,0);
 s.players[0].name='Christopher Alexander Montgomery-Washington III';const long=renderStories(s,[{...event,facts:{...event.facts,playerName:s.players[0].name}}]);assert(long.stories.every(p=>p.body.length<=280&&Buffer.byteLength(p.body)<=512));
});
