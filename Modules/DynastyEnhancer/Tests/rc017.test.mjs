import {preserveSameWeekStories} from '../Tools/story_renderer.mjs';
import test from 'node:test';
import assert from 'node:assert/strict';
import {weeklyBudget,enforceWeeklyBudget,protectedNews} from '../Tools/weekly_budget.mjs';
import {scopeViews,buildCatalog,publishWeek} from '../Tools/feed_selection.mjs';
import {editionSidebar} from '../Tools/edition_sidebar.mjs';
const event=(id,extra={})=>({id,category:'analytics',teamIds:[1],facts:{},...extra});
const post=(id,extra={})=>({id,eventId:id,voice:'beat',category:'analytics',teamIds:[1],scopes:['team'],body:'Verified '+id,priority:80,...extra});
test('routine limit counts roots against both teams; replies and recruit statlines are exempt',()=>{
 const budget=weeklyBudget();for(let i=0;i<15;i++){const e=event('e'+i,{teamIds:[1,2]});assert.ok(budget.allows(e,'beat'));budget.record(e);}
 assert.equal(budget.allows(event('new',{teamIds:[2,3]}),'beat'),false);assert.equal(budget.allows(event('other',{teamIds:[3]}),'beat'),true);
 assert.ok(budget.allows(event('reply'),'fan',{reply:true}));budget.record(event('reply'),{reply:true});
 assert.ok(budget.allows(event('film',{category:'recruit-film'}),'recruit'));budget.record(event('film',{category:'recruit-film'}));assert.equal(budget.counts.get(1),15);
});
test('protected news takes ordinary slots first, survives overflow, and retains replies to surviving parents',()=>{
 const events=Array.from({length:30},(_,i)=>event('r'+i));events.push(...Array.from({length:20},(_,i)=>event('award'+i,{category:'awards'})));
 const stories=events.map(e=>post(e.id,{category:e.category}));stories.push(post('reply',{replyTo:'award0',voice:'fan'}));
 const kept=enforceWeeklyBudget(stories,events);assert.equal(kept.filter(p=>!p.replyTo).length,20);assert.equal(kept.filter(p=>p.category==='analytics'&&!p.replyTo).length,0);assert.ok(kept.some(p=>p.id==='reply'));
 assert.equal(enforceWeeklyBudget(stories,events,new Set(['r0'])).filter(p=>p.id==='r0').length,1);
});
test('For You includes every relevant root and reply plus a bounded national mix; National is complete',()=>{
 const local=Array.from({length:15},(_,i)=>post('l'+i,{curated:i===0?false:true})),replies=local.map(p=>post(p.id+'reply',{replyTo:p.id,voice:'fan'}));
 const national=Array.from({length:80},(_,i)=>post('n'+i,{teamIds:[2],scopes:['national'],category:'results',priority:100-i}));
 const v=scopeViews([...local,...replies,...national]);assert.ok([...local,...replies].every(p=>v.all.includes(p.id)));assert.equal(v.national.length,110);assert.ok(v.all.filter(id=>id.startsWith('n')).length<=20);
 assert.deepEqual(scopeViews([...local,...replies,...national].map(p=>({...p,likes:999}))),v);
});
test('sidebar uses retained facts and provides neutral missing-data labels without fake counts or timestamps',()=>{
 const feed={userTeamIndex:1,events:[event('result',{category:'results',facts:{home:'Harbor',away:'Valley',homeScore:27,awayScore:20}})],stories:[post('result',{category:'results',body:'Harbor beat Valley 27-20.',scopes:['team','national']})]};
 const cards=editionSidebar(feed);assert.equal(cards.length,3);assert.deepEqual(cards[1].lines,['Valley 20','Harbor 27']);assert.deepEqual(cards[2].lines,['Valley 20','Harbor 27']);assert.ok(!/posts|\d+h\b/.test(JSON.stringify(cards)));
 assert.ok(editionSidebar({stories:[],events:[]}).every(card=>card.lines.length));
});
test('catalog presentation changes leave published post objects and history views untouched',()=>{
 const original=publishWeek({engineVersion:2,key:'1/2027/RegularSeason/1',season:2027,phase:'RegularSeason',week:1,leagueId:1,userTeamIndex:1,stories:[post('old')]});const before=JSON.stringify(original);
 const catalog=buildCatalog(original);assert.equal(JSON.stringify(original),before);assert.equal(catalog.weeks[0].sidebar.length,3);assert.equal(catalog.weeks[0].stories[0].body,'Verified old');
});

test('primary commitment coverage is protected before wording selection even for retained context reports',()=>{assert.ok(protectedNews(event('commit:1:2',{category:'recruiting',facts:{newlyObserved:false}}),'beat'));assert.equal(protectedNews(event('commit:1:2',{category:'recruiting'}),'fan'),false);});

test('new budgets retain old publications while corrections, withdrawals and orphaned replies remain factual',()=>{
 const events=[event('keep',{facts:{score:'31-26'}}),event('changed',{facts:{score:'20-10'}}),event('withdrawn'),event('child')];
 const original=[post('old',{eventId:'keep',body:'Original published text',author:'Original account'}),post('reply',{eventId:'keep',replyTo:'old'}),post('wrong',{eventId:'changed'}),post('gone',{eventId:'withdrawn'}),post('orphan',{eventId:'child',replyTo:'gone'})];
 const now=[events[0],{...events[1],facts:{score:'21-10'}},events[3]];
 const kept=preserveSameWeekStories([post('new')],now,{events,stories:original});assert.deepEqual(kept.map(p=>p.id),['new','old','reply']);assert.equal(kept[1],original[0]);assert.equal(kept[2],original[1]);
});

test('broad pipeline chatter cannot inflate For You while tracked recruits and every local post remain complete',()=>{
 const team=post('team'),tracked=post('tracked',{teamIds:[2],scopes:['relevant'],userRelevant:true}),pipeline=Array.from({length:80},(_,i)=>post('pipeline'+i,{teamIds:[2],scopes:['relevant']}));
 const v=scopeViews([team,tracked,...pipeline],{userTeamIndex:1});assert.ok(v.all.includes(team.id)&&v.all.includes(tracked.id));assert.ok(v.all.length<=22);assert.equal(v.national.length,82);
});
