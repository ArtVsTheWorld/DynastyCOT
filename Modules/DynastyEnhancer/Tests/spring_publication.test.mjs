import test from 'node:test';
import assert from 'node:assert/strict';
import {exhibitionContext,fallOpeningAfterSpring} from '../Tools/spring_context.mjs';
import {preseasonEvents} from '../Tools/preseason_events.mjs';
import {seasonLaunchEvents} from '../Tools/season_launch_events.mjs';
import {freshEditorialEvents} from '../Tools/editorial_policy.mjs';
const pub=week=>({leagueId:1,userTeamIndex:1,season:2026,phase:'PreSeason',week});
const e=(family='preseason.poll.beat',facts={})=>({id:'poll:1',teamIds:[1],category:'polls',facts:{family,school:'JMU',rank:14,...facts}});
const old=(event,week=0,extra={})=>({...pub(week),events:[event],stories:[{id:'root',eventId:event.id}],...extra});
test('week or native practice/empty rows alone cannot identify Extensions',()=>{
 const season={CurrentYear:0,CurrentWeekType:'PreSeason'};
 assert.equal(exhibitionContext([],season,1).active,false);
 assert.equal(exhibitionContext([{year:0,number:0,home:null,away:null}],season,1).active,false);
 assert.equal(exhibitionContext([{year:0,number:250,home:null,away:null}],season,1).active,true);
 const s=exhibitionContext([{id:'spring',year:0,week:2,number:3,home:255,away:1,status:'Unplayed'}],season,1);
 assert.equal(s.active,true);assert.equal(s.fixtures.length,1);
 season.CurrentWeekType='RegularSeason';assert.equal(exhibitionContext([{year:0,number:3,home:255,away:1}],season,1).active,false);
});
test('actually published unchanged preseason facts stop across weeks; facts can change back',()=>{
 const a=e(),b=e('preseason.poll.beat',{rank:9});
 assert.equal(freshEditorialEvents([a],pub(2),[old(a)]).length,0);
 assert.equal(freshEditorialEvents([b],pub(2),[old(a)]).length,1);
 assert.equal(freshEditorialEvents([a],pub(3),[old(a),old(b,2)]).length,1);
 assert.equal(freshEditorialEvents([a],pub(2),[old(a,0,{stories:[]})]).length,1);
 for(const h of [{leagueId:2},{userTeamIndex:2},{season:2027},{week:3},{week:2},{reconstructed:true}])assert.equal(freshEditorialEvents([a],pub(2),[old(a,0,h)]).length,1);
});
test('rewrites, authors and fan voices cannot create a new announcement',()=>{
 for(const family of ['preseason.team.hype','preseason.honor.beat','preseason.opener.beat','launch.new_coach']){
  const a=e(family,{award:'All-American',opponent:'EMU',coach:'Coach One'});
  const b={...a,id:'new-id',author:'other',facts:{...a.facts,voice:'fan'}};
  assert.equal(freshEditorialEvents([b],pub(2),[old(a)]).length,0);
 }
 const a=e('preseason.fan.nervous'),b=e('preseason.fan.excited');assert.equal(freshEditorialEvents([b],pub(2),[old(a)]).length,0);
});
test('old spring publications do not consume the actual fall launch',()=>{
 const a=e(),fall={...pub(0),phase:'RegularSeason',springExhibitions:true};
 assert.equal(freshEditorialEvents([a],fall,[old(a,2)]).length,1);
 const explicit=e('preseason.poll.beat',{publicationContext:'spring'});assert.equal(freshEditorialEvents([a],fall,[old(explicit)]).length,1);
});
test('spring retains genuine recruit watches and defers fall announcement families',()=>{
 const s={leagueId:1,userTeamIndex:1,exhibitions:{active:true,supported:true},season:{CurrentYear:0,CurrentWeek:2,CurrentWeekType:'PreSeason',CurrentStage:'PreSeason'},teams:[{id:1,name:'JMU',mediaRank:14}],players:[{id:'r',name:'Alex Bell',team:255,position:'TE',starRating:4}],games:[],preseasonHonors:[],commitments:[],recruitStatuses:[{player:'r',stage:'Top5',nationalRank:25,topSchools:[1,2]}]};
 const rows=preseasonEvents(s);assert.ok(rows.length);assert.ok(rows.every(e=>e.category==='recruiting'));assert.deepEqual(seasonLaunchEvents(s),[]);
 const a={...rows[0],teamIds:[]},h=old(a);h.stories[0].eventId=a.id;
 assert.equal(freshEditorialEvents([a],pub(2),[h]).length,0);
 const changed={...a,facts:{...a.facts,recruitStage:'Top3'}};assert.equal(freshEditorialEvents([changed],pub(2),[h]).length,1);
 s.recruitStatuses=[];assert.deepEqual(preseasonEvents(s),[]);
});

test('deferred fall previews stop before competitive week one',()=>{
 const s={exhibitions:{supported:true},season:{CurrentYear:0,CurrentStage:'NFLSeason',CurrentWeekType:'RegularSeason',CurrentWeek:0},games:[{year:0,phase:'RegularSeason',status:'Unplayed'}]};
 assert.equal(fallOpeningAfterSpring(s),true);s.season.CurrentWeek=1;assert.equal(fallOpeningAfterSpring(s),false);
});
