import test from 'node:test';
import assert from 'node:assert/strict';
import {applyWeeklyCoverage} from '../Tools/weekly_coverage.mjs';
import {requiredCoverageEventIds,optionalAwardAcknowledgment,assertPrimaryAwardConservation} from './SocialVariety/conservation.mjs';
const award=(id,name,voice='analyst',playerId='prospect-a')=>({id,category:'awards',playerId,teamIds:[1],publication:{season:2026},facts:{family:voice==='player'?'postseason.award-player':'postseason.award',voice,award:name,count:1,school:'Harbor State',playerName:'Evan Moss',finalAward:true,nationalAward:true}});
test('an optional award thank-you is not a second required report of an award already covered',()=>{
 const events=[award('qb-report','top quarterback award'),award('qb-thanks','top quarterback award','player'),award('academic-report','national academic award'),award('academic-thanks','national academic award','player')];
 assert.deepEqual(requiredCoverageEventIds(events.map(e=>e.id),events),['academic-report','qb-report']);
 assert.deepEqual(assertPrimaryAwardConservation(events,events.map(e=>e.id),['qb-report','academic-report']),{expected:2,generated:2,published:2,missingGenerated:[],missingPublished:[]});
 assert.throws(()=>assertPrimaryAwardConservation(events,events.map(e=>e.id),['qb-report']),/academic-report/,'Distinct awards for one player must remain required');
});
test('player thanks stay required without an exact same-player, same-award primary counterpart',()=>{
 const thanks=award('thanks','top quarterback award','player');
 for(const other of [award('other','national academic award'),award('other','top quarterback award','analyst','different-person'),{...award('other','top quarterback award'),publication:{season:2025}},{...award('other','top quarterback award'),facts:{...award('other','top quarterback award').facts,count:2}}]){
  assert.equal(optionalAwardAcknowledgment(thanks,[thanks,other]),false);assert.ok(requiredCoverageEventIds(['thanks'],[thanks,other]).includes('thanks'));
 }
});
test('weekly flavor limits may trim a player acknowledgment while preserving every primary award',()=>{
 const events=[award('qb-report','top quarterback award'),award('qb-thanks','top quarterback award','player'),award('academic-report','national academic award')];
 const primary=events.filter(e=>e.facts.voice==='analyst').map(e=>({id:e.id+':main',eventId:e.id,category:'awards',voice:'analyst',teamIds:[1],scopes:['national'],priority:105,essentialNews:true}));
 const routine=Array.from({length:30},(_,i)=>({id:'routine'+i,eventId:'routine'+i,category:'analytics',voice:'beat',author:'Reporter'+i,teamIds:[1],scopes:['team'],priority:120,thread:'routine'+i}));
 const thanks={id:'qb-thanks:main',eventId:'qb-thanks',category:'awards',voice:'player',teamIds:[1],scopes:[],priority:1};
 const generated=[...routine,...primary,thanks],published=applyWeeklyCoverage(generated,{teams:[{id:1,name:'Harbor State',brandExposure:'F',mediaRank:99}],season:{CurrentWeekType:'BowlSeason1',CurrentWeek:17}});
 assert.ok(!published.some(p=>p.id===thanks.id));assert.ok(primary.every(p=>published.some(q=>q.id===p.id)));
 assert.equal(assertPrimaryAwardConservation(events,generated.map(p=>p.eventId),published.map(p=>p.eventId)).published,2);
});
