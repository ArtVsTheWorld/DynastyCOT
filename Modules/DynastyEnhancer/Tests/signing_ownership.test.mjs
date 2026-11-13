import test from 'node:test';
import assert from 'node:assert/strict';
import {normalizeDynasty} from '../Tools/normalize_dynasty.mjs';
import {signingOwnership,preservePublishedSignings} from '../Tools/signing_ownership.mjs';

function fixture({stage='Signed',owners=[2],duplicate=false,repeated=false,recruitClass='HighSchool'}={}){
 const row=(key,type,v,capacity=200)=>({key,type,v,capacity});
 const rows=[row('season','SeasonInfo',{CurrentYear:0,CurrentSeasonYear:2026,CurrentWeekType:'BowlSeason1',CurrentWeek:17}),row('league','Franchise',{LeagueID:7}),row('user','FranchiseUser',{Team:'one'}),row('one','Team',{TeamIndex:1,DisplayName:'Old Interest Leader',CommittedPlayers:'list1'}),row('two','Team',{TeamIndex:2,DisplayName:'Signing School',CommittedPlayers:'list2'}),row('list1','Player[]',{}),row('list2','Player[]',{}),row('player','Player',{FirstName:'Augustine',LastName:'Goss',TeamIndex:255,Position:'QB'}),row('recruit','Recruit',{Player:'player',RecruitStage:stage,Class:recruitClass,NationalRank:20,TopSchoolsList:'interest'}),row('interest','ProspectTargetSchool[]',{}),row('interest1','ProspectTargetSchool',{TeamId:1,TeamInfluence:900}),row('interest2','ProspectTargetSchool',{TeamId:2,TeamInfluence:800})];
 if(duplicate)rows.push(row('duplicate','Recruit',{...rows.find(r=>r.key==='recruit').v}));
 const map=new Map(rows.map(r=>[r.key,r]));
 const arrays={list1:owners.includes(1)?[map.get('player')]:[],list2:owners.includes(2)?[map.get('player')]:[],interest:[map.get('interest1'),map.get('interest2')]};
 if(repeated)arrays.list2.push(map.get('player'));
 const adapter={records:async name=>rows.filter(r=>r.type===name),resolve:async key=>map.get(key)??null,array:async key=>arrays[key]??[],arraySlots:async()=>[]};
 return {rows,arrays,map,adapter};
}
test('Signed ownership wins over stale interest without requiring a board entry; source unchanged',async()=>{
 const f=fixture(),before=JSON.stringify(f.rows);const s=await normalizeDynasty(f.adapter,{});
 assert.equal(s.signings.length,1);assert.equal(s.signings[0].team,2);assert.equal(s.signings[0].player,'player');assert.equal(s.commitments.length,0);assert.equal(JSON.stringify(f.rows),before);
});
test('missing and conflicting memberships are explicit, without interest fallback',async()=>{
 for(const owners of [[],[1,2]]){const s=await normalizeDynasty(fixture({owners}).adapter,{});assert.equal(s.signings.length,0);assert.equal(s.signingDetection[owners.length?'conflictingMembership':'missingMembership'],1);}
});
test('duplicate list members and duplicate Recruit rows never multiply signing posts',async()=>{
 const s=await normalizeDynasty(fixture({duplicate:true,repeated:true}).adapter,{});assert.equal(s.signings.length,1);assert.equal(s.signingDetection.repeatedMemberships,1);assert.equal(s.signingDetection.duplicateSignedRows,1);
});
test('unsigned/verbal recruits do not become signed from list membership',async()=>{
 for(const stage of ['Open','HardCommitted','SoftCommitted']){const s=await normalizeDynasty(fixture({stage}).adapter,{});assert.equal(s.signings.length,0);assert.equal(s.signingDetection.signedRows,0);}
});
test('transfers use the same verified relationship; cleared rollover lists do not invent ownership',async()=>{
 const s=await normalizeDynasty(fixture({recruitClass:'Transfer'}).adapter,{});assert.equal(s.signings[0].team,2);assert.equal(s.signings[0].recruitClass,'Transfer');
 const rolled=fixture({owners:[]});rolled.rows.find(r=>r.key==='player').v.TeamIndex=2;rolled.rows.find(r=>r.key==='season').v.CurrentYear=1;
 assert.equal((await normalizeDynasty(rolled.adapter,{})).signings.length,0);
});
test('committed-list and member types remain validated; unavailable is distinct',async()=>{
 for(const type of ['Recruit[]','Team']){const f=fixture();f.map.get('list2').type=type;await assert.rejects(signingOwnership(f.rows.filter(r=>r.type==='Team').map(r=>r.v),f.adapter),/reference type/);}
 const f=fixture();f.arrays.list2=[{key:'bad',type:'Recruit',v:{}}];await assert.rejects(signingOwnership(f.rows.filter(r=>r.type==='Team').map(r=>r.v),f.adapter),/not a Player/);
 const missing=fixture();delete missing.map.get('two').v.CommittedPlayers;const s=await normalizeDynasty(missing.adapter,{});assert.equal(s.signingDetection.unavailableTeamLists,1);assert.equal(s.signings.length,0);
});
test('already-published signing facts and words remain frozen, unpublished reports can be corrected',()=>{
 const old={id:'signed:old',category:'signings',playerId:'p',facts:{team:'Old School'}},fresh={id:'signed:new',category:'signings',playerId:'p',facts:{team:'Correct School'}},other={id:'result',category:'results',playerId:'p'},unpublished={id:'signed:q',category:'signings',playerId:'q'};
 const catalog={events:[old,unpublished],stories:[{eventId:old.id,body:'Original published wording'}]},before=structuredClone(catalog);
 assert.deepEqual(preservePublishedSignings([fresh,other,unpublished],catalog),[other,unpublished,old]);assert.deepEqual(catalog,before);assert.deepEqual(preservePublishedSignings([fresh],undefined),[fresh]);
});

test('recruit-authored signing publications use the same preservation guard',()=>{const old={id:'recruit-old',category:'recruiting',facts:{signed:true},options:{player:'p'}},fresh={...old,id:'recruit-new',options:{player:'p',team:2}};assert.deepEqual(preservePublishedSignings([fresh],{events:[old],stories:[{eventId:old.id}]}),[old]);});
