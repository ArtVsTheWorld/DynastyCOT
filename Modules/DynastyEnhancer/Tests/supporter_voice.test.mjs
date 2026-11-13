import test from 'node:test';import assert from 'node:assert/strict';
import {supporterContext,capturedSupporterContext} from '../Tools/supporter_voice.mjs';
import {contentPack} from '../Tools/story_templates.mjs';
const home={id:1,name:'Liberty',nickname:'Flames'},away={id:2,name:'James Madison',nickname:'Dukes'};
test('captured final fan facts remain usable without an exported full game, with exact account ownership',()=>{
 const teams=new Map([[1,home],[2,away]]),e={id:'barstool:g:1',thread:'game:g',category:'results',teamIds:[1,2],facts:{family:'fanclub.win',school:'Liberty',opponent:'James Madison',score:'31–26',negative:false}};
 assert.equal(capturedSupporterContext(e,home,teams).family,'supporter.relief');
 assert.equal(capturedSupporterContext(e,away,teams),null);assert.equal(capturedSupporterContext({...e,facts:{...e.facts,negative:true}},home,teams),null);
 assert.equal(capturedSupporterContext({...e,facts:{...e.facts,opponent:'Iowa'}},home,teams),null);
});
test('allegiance comes from the actual account team, never the event winner default',()=>{
 const g={home:1,away:2,homeScore:31,awayScore:26};
 assert.equal(supporterContext(g,home,away).family,'supporter.relief');assert.equal(supporterContext(g,away,home).family,'supporter.loss');
 assert.equal(supporterContext(g,{id:3,name:'Neutral'},home),null);assert.equal(supporterContext(g,home,home),null);
 assert.equal(supporterContext({...g,homeScore:26},home,away),null);
 assert.equal(supporterContext({...g,homeScore:NaN},home,away),null);
});
test('only verified margin, rank disparity and rivalry choose emotional contexts',()=>{
 const g={home:1,away:2,homeScore:31,awayScore:26};
 assert.equal(supporterContext(g,home,away,{awayRank:14,homeRank:0}).family,'supporter.upset');
 assert.equal(supporterContext(g,home,away,{awayRank:14,homeRank:4}).family,'supporter.relief');
 assert.equal(supporterContext(g,home,away,{rivalry:true}).family,'supporter.rivalry');
 assert.equal(supporterContext({...g,homeScore:48},away,home).family,'supporter.heavy-loss');
});
test('48 finite original supporter variants fit native limits and do not require invented incidents',()=>{
 let count=0;for(const [family,variants] of Object.entries(contentPack.families).filter(([k])=>k.startsWith('supporter.'))){
  for(const v of variants){++count;const values={school:'Northern Illinois',nickname:'Huskies',opponent:'Iowa',score:'17-20'};const text=v.body.replace(/\{([A-Za-z]+)\}/g,(_,key)=>values[key]??'{'+key+'}');
   assert(!text.includes('{'),family);assert(text.length<=280&&Buffer.byteLength(text)<=512);
   assert(!/referee|injury|interview|fumble|quarterback|penalt|last.second/i.test(text));
  }
 }assert.equal(count,48);
});
