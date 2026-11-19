import fs from 'node:fs';
const bank=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/portrait-ranges.json',import.meta.url),'utf8'));
const logoBank=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/team-logo-variants.json',import.meta.url),'utf8')),logos=logoBank.logos;
const national=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/national-account-avatars.json',import.meta.url),'utf8')).portraits;
const validPortrait=id=>Number.isInteger(id)&&id>0&&bank.ranges.some(([lo,hi])=>id>=lo&&id<=hi);
const validLogo=logo=>Number.isInteger(logo)&&logo>=0;

export function recruitJerseys(snapshot,teams){
 const destinations=new Map();
 for(const [rows,stage] of [[snapshot.commitments,'HardCommitted'],[snapshot.signings,'Signed']]){
  for(const row of rows??[]){
   if(row.stage!==stage||!teams.has(row.team)||row.team===255)continue;
   if(!destinations.has(row.player))destinations.set(row.player,row.team);
   else if(destinations.get(row.player)!==row.team)destinations.set(row.player,null);
  }
 }
 return new Map([...destinations].map(([player,team])=>[player,teams.get(team)?.logo]));
}

export function recruitPortraitForDisplay(post,eventFor){
 if(post.voice!=='recruit'||typeof post.defaultRecruitJersey==='boolean'||post.uniformTeamLogo>0||
    post.imageLibrary!==bank.libraryId||!validPortrait(post.imageValue))return post;
 // Version 1.0.37 stored zero both for the default and for the actual Air
 // Force logo. Keep a school jersey if the historical story verifies it.
 const event=eventFor(post.eventId);
 const facts=event?.facts;
 if(['HardCommitted','Signed'].includes(facts?.commitment)||facts?.signed===true)return post;
 const {uniformTeamLogo,...portrait}=post;
 return {...portrait,defaultRecruitJersey:true};
}

export function avatarFor(role,team,player,persona=0,recruitLogo){
 const personal=['player','recruit'].includes(role),id=player?.portrait;
 if(personal&&!player?.portraitForcedSilhouette&&validPortrait(id)){
  const portrait={imageLibrary:bank.libraryId,imageValue:id};
  if(role==='recruit')return validLogo(recruitLogo)
   ?{...portrait,uniformTeamLogo:recruitLogo,defaultRecruitJersey:false}
   :{...portrait,defaultRecruitJersey:true};
  if(Number.isInteger(player.team)&&player.team!==255&&player.team===team?.id&&validLogo(team.logo))return {...portrait,uniformTeamLogo:team.logo};
  return portrait;
 }
 if(role==='nflDraft')return {imageLibrary:34,imageValue:21};
 if(role==='cfp')return {imageLibrary:34,imageValue:41};
 if(role==='analyst'&&national[persona]){const {imageLibrary,imageValue}=national[persona];return {imageLibrary,imageValue};}
 if(team?.isTeamBuilder&&['beat','fan'].includes(role)&&Number.isInteger(team.logo))return {imageLibrary:0,imageValue:team.logo};
 if(Number.isInteger(team?.logo)&&team.logo>=0){
  if(role==='official')return !team.isTeamBuilder&&logos[team.logo]?.onwhite||{imageLibrary:0,imageValue:team.logo};
  if(role==='beat')return !team.isTeamBuilder&&logos[team.logo]?.helmet||{imageLibrary:0,imageValue:team.logo};
  if(role==='fan'&&persona===0)return !team.isTeamBuilder&&logos[team.logo]?.mascot||{imageLibrary:0,imageValue:team.logo};
  if(role==='fan'&&persona===1)return !team.isTeamBuilder&&logos[team.logo]?.secondary||{imageLibrary:0,imageValue:team.logo};
 }
 return {imageLibrary:79,imageValue:({beat:939,analyst:598,model:1053,fan:203,player:974,recruit:974})[role]??598};
}
