import {gpiRankings} from './gpi_rankings.mjs';
import {sameLeague} from './league_identity.mjs';
// Consume the exact native menu model, never a second approximation of it.
const finite=x=>typeof x==='number'&&Number.isFinite(x);
const sample=(r,season)=>finite(r.grade)&&r.grade>=0&&r.grade<=100&&finite(r.snaps)&&(['K','P'].includes(r.group)?r.opportunities>=(season?8:2):r.snaps>=(season?200:20))&&(!season||r.gradedGames>=3);
const stage=t=>({PreSeason:0,Preseason:0,RegularSeason:1,PostSeason:2,Postseason:2,BowlSeason:2,Playoffs:2,BowlSeason1:3,BowlSeason2:4,BowlSeason3:5,NationalChampionship:6,OffSeason:7,Offseason:7}[t]??-1);
const released=(s,a)=>Number.isInteger(a.week)&&a.week>=0&&stage(a.weekType)>=0&&(stage(a.weekType)<stage(s.season.CurrentWeekType)||stage(a.weekType)===stage(s.season.CurrentWeekType)&&a.week<s.season.CurrentWeek);
// Store only comparable qualified observations, rather than another full
// roster/capture in every publication. This is history, never a native write.
export function gpiObservation(s){
 const a=s.playerAnalytics;if(!a||!Array.isArray(a.seasonRows))return null;
 return {modelVersion:a.modelVersion,leagueId:a.leagueId,year:a.year,season:a.season,week:a.week,weekType:a.weekType,
  seasonRows:a.seasonRows.filter(r=>sample(r,true)).map(({player,team,group,grade,snaps,gradedGames})=>({player,team,group,grade,snaps,gradedGames}))};
}
export function freezePublishedGpi(events,s,feed){
 if(!feed||feed.leagueId!==s.leagueId||feed.userTeamIndex!==s.userTeamIndex||feed.season!==s.season.CurrentSeasonYear||feed.phase!==s.season.CurrentWeekType||feed.week!==s.season.CurrentWeek)return events;
 const published=new Set((feed.stories??[]).map(p=>p.eventId)),old=(feed.events??[]).filter(e=>e.facts?.family?.startsWith('pff.')&&published.has(e.id));
 return old.length?[...events.filter(e=>!e.facts?.family?.startsWith('pff.')),...old]:events;
}
export function gpiEvents(s,previous=null){
 const a=s.playerAnalytics,candidate=previous?.playerAnalytics;
 const old=sameLeague(candidate?.leagueId,s.leagueId)&&candidate.year===s.season.CurrentYear&&candidate.season===s.season.CurrentSeasonYear&&candidate.modelVersion===a?.modelVersion?candidate:null;
 if(!a||a.format!==1||typeof a.modelVersion!=='string'||!Array.isArray(a.weekly)||!Array.isArray(a.seasonRows)||!sameLeague(a.leagueId,s.leagueId)||a.year!==s.season.CurrentYear||a.season!==s.season.CurrentSeasonYear||s.reconstructed||s.season.CurrentStage!=='NFLSeason'||/preseason|offseason/i.test(s.season.CurrentWeekType)||!released(s,a))return [];
 const teams=new Map(s.teams.filter(t=>t.id!==255).map(t=>[t.id,t])),players=new Map(s.players.map(p=>[p.id,p]));
 const valid=r=>teams.has(r.team)&&players.get(r.player)?.team===r.team;
 const oldRows=old?.modelVersion===a.modelVersion&&sameLeague(old.leagueId,a.leagueId)&&old.year===a.year&&Array.isArray(old.seasonRows)?new Map(old.seasonRows.map(r=>[r.player,r])):new Map();
 const season=a.seasonRows.filter(r=>valid(r)&&sample(r,true)),weekly=a.weekly.filter(r=>valid(r)&&sample(r,false));
 const comparable=(p,r)=>p&&p.team===r.team&&p.group===r.group;
 const changed=r=>{const p=oldRows.get(r.player);return !comparable(p,r)||p.grade!==r.grade||p.snaps!==r.snaps||p.gradedGames!==r.gradedGames;};
 const selected=[],usedTeams=new Set(),usedPlayers=new Set();
 const choose=(row,kind,national=false)=>{
  if(!row||selected.length>=4||usedTeams.has(row.team)||usedPlayers.has(row.player))return;
  const p=players.get(row.player),team=teams.get(row.team),before=oldRows.get(row.player);
  const values={family:'pff.'+kind,voice:'analyst',persona:6,analystPersona:6,playerName:p.name,position:p.position,school:team.name,grade:row.grade.toFixed(1),snaps:row.snaps,gradedGames:row.gradedGames??1,modelVersion:a.modelVersion,
   sampleLabel:row.group==='K'?`${row.opportunities} FG/PAT attempts`:row.group==='P'?`${row.opportunities} punts`:`${row.snaps} recorded snaps`,
   weekLabel:a.weekType==='RegularSeason'?`Week ${a.week}`:`${a.weekType} Week ${a.week}`,sourceWeek:a.week,sourceWeekType:a.weekType,statBased:true};
  if(kind==='movement'){if(!comparable(before,row)||row.gradedGames<=before.gradedGames||Math.abs(row.grade-before.grade)<3)return;values.previousGrade=before.grade.toFixed(1);values.direction=row.grade>before.grade?'up':'down';}
  selected.push({id:`pff:${kind}:${row.player}`,category:'analytics',facts:values,options:{team:row.team,player:row.player,national,priority:row.team===s.userTeamIndex?88:81,eventWeek:s.season.CurrentWeek}});usedTeams.add(row.team);usedPlayers.add(row.player);
 };
 const sort=(a,b)=>b.grade-a.grade||b.snaps-a.snaps||a.player.localeCompare(b.player);
 for(const event of gpiRankings(s,a,old)){
  const board=event.facts.family==='pff.position-top-three',related=[event.options.team,...event.options.relatedTeams];if(selected.length>=4||!board&&related.some(t=>usedTeams.has(t)))continue;
  selected.push(event);if(!board)related.forEach(t=>usedTeams.add(t));if(event.options.player)usedPlayers.add(event.options.player);
 }
 const own=season.filter(r=>r.team===s.userTeamIndex&&changed(r)).sort(sort),movement=own.find(r=>{const p=oldRows.get(r.player);return comparable(p,r)&&r.gradedGames>p.gradedGames&&Math.abs(r.grade-p.grade)>=3;});
 const ownWeekly=weekly.filter(r=>r.team===s.userTeamIndex).sort(sort)[0];
 if(movement)choose(movement,'movement');
 else if(own.length){const low=[...own].reverse().find(r=>r.grade<50);choose(low&&s.season.CurrentWeek%3===0?low:own[0],low&&s.season.CurrentWeek%3===0?'season-watch':'season-spotlight');}
 else if(ownWeekly?.grade>=80&&(!old||old.week!==a.week||old.weekType!==a.weekType))choose(ownWeekly,'weekly');
 const groups=['QB','RB','WR','TE','OL','DL','LB','CB','S','K','P'],focus=groups[Math.abs(s.season.CurrentWeek)%groups.length];
 for(const r of season.filter(r=>r.group===focus&&r.grade>=80&&changed(r)).sort(sort)){const n=selected.length;choose(r,'season-spotlight',true);if(selected.length>n)break;}
 if(!old||old.week!==a.week||old.weekType!==a.weekType)for(const r of weekly.filter(r=>r.grade>=85).sort(sort))choose(r,'weekly',true);
 return selected;
}
