import test from 'node:test';
import assert from 'node:assert/strict';
import {publishWeek,buildCatalog} from '../Tools/feed_selection.mjs';

test('native catalog retains fractional priorities and news protection without altering publications',()=>{
 const stories=[{id:'offer',author:'Recruit',handle:'@Recruit',body:'Verified offer',stamp:'Recruiting',category:'offers',voice:'recruit',scopes:['team'],teamIds:[1],priority:72+1/37,curated:true,imageLibrary:79,imageValue:1,budgetProtected:true},
 {id:'result',author:'Reporter',handle:'@Reporter',body:'Verified result',stamp:'Recap',category:'results',voice:'beat',scopes:['national'],teamIds:[1],priority:91.375,curated:true,imageLibrary:79,imageValue:1,essentialNews:true,coverageExempt:false}];
 const edition=week=>publishWeek({format:1,engineVersion:2,leagueId:1,userTeamIndex:1,season:2027,phase:'RegularSeason',week,key:`1/2027/RegularSeason/${week}`,stories:structuredClone(stories)});
 const active=edition(2),old=edition(1),before=structuredClone({active,old});const catalog=buildCatalog(active,[old]);
 assert.deepEqual({active,old},before);
 for(const week of catalog.weeks){
  assert.equal(week.stories.find(p=>p.id==='offer').priority,72+1/37);
  assert.equal(week.stories.find(p=>p.id==='offer').budgetProtected,true);
  assert.equal(week.stories.find(p=>p.id==='result').essentialNews,true);
  assert.equal(week.stories.find(p=>p.id==='result').coverageExempt,false);
 }
 assert.deepEqual(JSON.parse(JSON.stringify(catalog)).weeks.map(w=>w.stories),catalog.weeks.map(w=>w.stories));
});
