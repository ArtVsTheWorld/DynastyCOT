import test from 'node:test';import assert from 'node:assert/strict';
import {recruitAccountName} from '../Tools/recruit_account_name.mjs';
import {authorRegistry} from '../Tools/story_authors.mjs';
import {refreshAccountPresentation} from '../Tools/account_presentation.mjs';
test('recruit stars change display names without changing persistent handles',()=>{
 const p={id:'r',name:'Drew Coleman',position:'DT',starRating:5,team:255};
 const s={leagueId:1,players:[p],teams:[],recruitStatuses:[]};const registry=authorRegistry();const handle=registry.get(s,'recruit',null,p).handle;
 for(let stars=1;stars<=5;stars++){p.starRating=stars;assert.equal(registry.get(s,'recruit',null,p).author,`Drew Coleman | ${stars}-Star DT`);assert.equal(registry.get(s,'recruit',null,p).handle,handle);}
 assert.equal(recruitAccountName({...p,starRating:null,stars:null}),'Drew Coleman | DT');
 assert.equal(recruitAccountName({...p,starRating:0}),'Drew Coleman | DT');
 assert.equal(recruitAccountName(p,'Transfer '),'Drew Coleman | 5-Star Transfer DT');
});
test('presentation v3 archives gain stars and retain prose, dates, handles and likes',()=>{
 const s={players:[{id:'r',name:'Drew Coleman',position:'DT',starRating:5}],teams:[],recruitStatuses:[]};
 const story={id:'one',voice:'recruit',playerId:'r',author:'Drew Coleman | DT',handle:'@DrewC',body:'Locked in.',likes:51,publication:{week:2}};
 const feed={presentationVersion:3,stories:[story],views:{all:['one']}};const out=refreshAccountPresentation(feed,s);
 assert.equal(out.stories[0].author,'Drew Coleman | 5-Star DT');assert.deepEqual({...out.stories[0],author:story.author},story);
 assert.equal(out.presentationVersion,4);assert.deepEqual(refreshAccountPresentation(out,s),out);
});
