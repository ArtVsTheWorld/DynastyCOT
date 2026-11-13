import test from 'node:test';
import assert from 'node:assert/strict';
import {teamNounGrammar} from '../Tools/team_language.mjs';
import {conferenceGrammar} from '../Tools/conference_language.mjs';
import {proofreadNewText} from '../Tools/prose_cleanup.mjs';
const teams=[{name:'James Madison',nickname:'Dukes'},{name:'Liberty',nickname:'Flames'},{name:'Texas',nickname:'Longhorns'},{name:'The Citadel',nickname:'Bulldogs'}];
test('team nouns use articles without changing school names or modifiers',()=>{
 const cases=[['A rivalry date ahead for Dukes and Liberty.','A rivalry date ahead for the Dukes and Liberty.'],['We beat Longhorns.','We beat the Longhorns.'],['Dukes fans love this.','Dukes fans love this.'],["Dukes\' season.","Dukes\' season."],['Dukes | Final','Dukes | Final'],['A win for The Citadel.','A win for The Citadel.'],['The The Citadel game.','The Citadel game.'],['Dukes forever!','Dukes forever!'],['We beat the Dukes.','We beat the Dukes.'],['Liberty hosts James Madison.','Liberty hosts James Madison.']];
 for(const [a,b] of cases)assert.equal(teamNounGrammar(a,teams),b);
});
test('conference articles respect nouns, modifiers, headlines and possessives',()=>{
 for(const conference of ['ACC','SEC','Big Ten','Big 12','MAC','Mountain West','Sun Belt','Pac-12']){
  assert.equal(conferenceGrammar('A win in '+conference+'.'),'A win in the '+conference+'.');
  assert.equal(conferenceGrammar(conference+' teams compete.'),conference+' teams compete.');
  assert.equal(conferenceGrammar('Confidence in '+conference+' teams.'),'Confidence in '+conference+' teams.');
 }
 assert.equal(conferenceGrammar('East Point joins CUSA.'),'East Point joins the C-USA.');
 assert.equal(conferenceGrammar('A win in the the ACC.'),'A win in the ACC.');
});
test('units agree without changing scores, decimals or compound measures',()=>{
 assert.equal(proofreadNewText('1 points. 1 hour. 1 players are ready. 1 wins were enough.'),'1 point. 1 hour. 1 player is ready. 1 win was enough.');
 assert.equal(proofreadNewText('11 points. 21 yards. 1.1 yards. A 1-point win.'),'11 points. 21 yards. 1.1 yards. A 1-point win.');
 assert.equal(proofreadNewText('1 points separate them. 21 points separate them.'),'1 point separates them. 21 points separate them.');
});
