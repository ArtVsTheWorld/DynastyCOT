import test from 'node:test';
import assert from 'node:assert/strict';
import {injuryName} from '../Tools/injury_names.mjs';
import {injuryTimetable} from '../Tools/injury_timetables.mjs';
test('confirmed diagnosis uses native-facing abbreviation and unknown identifiers stay omitted',()=>{
 for(const value of ['KneeMCLPartialTear','Knee_MCLPartialTear','Partial MCL Tear'])assert.equal(injuryName(value),'Partial MCL Tear');
 for(const value of [null,'Invalid_','None','Unknown','UnknownInternalDiagnosis','KneeInventedTear'])assert.equal(injuryName(value),null);
 assert.equal(injuryName('Ankle Sprain'),'Ankle Sprain');
});
test('remaining injury bounds are not reduced twice or replaced with total duration',()=>{
 const player={injury:'Injured',injuryType:'KneeMCLPartialTear',injurySeverity:'SeveralGames',injuryYear:0,injuryWeek:13,minInjury:5,maxInjury:6,totalInjuryDuration:8};
 const old=structuredClone(player);assert.equal(injuryTimetable(player,{CurrentYear:0,CurrentWeek:15}).text,'Estimated return: 5–6 weeks.');
 const later={...player,minInjury:0,maxInjury:1};assert.equal(injuryTimetable(later,{CurrentYear:0,CurrentWeek:20}).maxWeeks,1);assert.deepEqual(player,old);
 assert.equal(injuryTimetable({...player,injury:'Uninjured'}),null);
});
