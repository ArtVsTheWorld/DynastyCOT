import {createHash} from 'node:crypto';
import {hash} from './story_authors.mjs';
import {editionCalendar} from './calendar_events.mjs';
const traits={Normal:0,College_Impact:1,College_Star:2,College_Elite:3};
const group=position=>position==='QB'?'qb':['HB','RB','FB'].includes(position)?'runner':['WR','TE'].includes(position)?'receiver':['LT','LG','C','RG','RT'].includes(position)?'line':['K','P'].includes(position)?'specialist':['LE','RE','LEDG','REDG','DT','LOLB','ROLB','MLB','SAM','MIKE','WILL','CB','FS','SS'].includes(position)?'defense':null;
const identity=p=>[p.id,p.presentationId??'',p.name,p.position].join('|');
export function observeRecruitGames(s,prior=null){
 const scope=`${s.leagueId}|${s.userTeamIndex}|${s.season.CurrentSeasonYear}`,ledger=prior?.format===1&&prior.scope===scope?structuredClone(prior):{format:1,scope,records:{}};
 const window=editionCalendar(s),date=window&&new Date(window.anchor);
 if(s.reconstructed||s.season.CurrentWeekType!=='RegularSeason'||!date||date.getUTCMonth()<7||date.getUTCMonth()>10)return {ledger,records:[]};
 const people=new Map(s.players.map(p=>[p.id,p])),interests=new Set(s.recruitInterests??[]),candidates=(s.recruitStatuses??[]).filter(r=>r.recruitClass==='HighSchool'&&r.stage!=='Signed'&&people.get(r.player)?.team===255).sort((a,b)=>Number(interests.has(b.player))-Number(interests.has(a.player))||(a.nationalRank??99999)-(b.nationalRank??99999)||a.player.localeCompare(b.player));
 const records=[];
 for(const r of candidates){const p=people.get(r.player),kind=group(p.position);if(!kind)continue;
  const person=identity(p),key=createHash('sha256').update(person+'|'+s.season.CurrentWeek).digest('hex'),seed=[s.leagueId,s.season.CurrentSeasonYear,person,s.season.CurrentWeek].join('|');
  if(ledger.records[key]){records.push(ledger.records[key]);continue;}
  // At most a three-percentage-point posting nudge. Unknown traits use neutral
  // probability. Independent noise dominates; public star counts remain public.
  const trait=traits[p.developmentTrait]??0,probability=.105+trait*.01;
  if(hash(seed+'|post')/4294967296>=probability)continue;
  const u=name=>hash(seed+'|'+name)/4294967296,noise=.35+u('game')*1.25,quality=(.78+(Math.max(1,Math.min(5,p.starRating??3))-1)*.08)*(1+trait*.006),round=x=>Math.max(0,Math.round(x));
  const values={passYards:round(210*noise*quality),passTD:round(u('passTD')*4*quality),rushYards:round(100*noise*quality),rushTD:round(u('rushTD')*3),catches:1+round(u('catches')*7),receiveYards:0,receiveTD:round(u('receiveTD')*2),blocks:round(2+u('blocks')*8),tackles:round(2+u('tackles')*10),sacks:u('sacks')<.7?0:1+round(u('sackcount')*2),kickAttempts:1+round(u('attempts')*3),kickMade:0,punts:2+round(u('punts')*5),puntAverage:round(29+u('puntAverage')*19),isKicker:p.position==='K',isPunter:p.position==='P'};
  values.receiveYards=values.catches*round(4+u('catchLength')*19);values.kickMade=Math.min(values.kickAttempts,round(values.kickAttempts*(.4+u('kicks')*.6)));values.tackles=Math.max(values.tackles,values.sacks);
  // No native high-school game-stat table is captured by the verified adapter.
  // Keep the source explicit; never alter native player/rating/stat records.
  const record={key,player:p.id,identity:person,season:s.season.CurrentSeasonYear,week:s.season.CurrentWeek,date:date.toISOString().slice(0,10),source:'mod-modeled-hs',group:kind,values};
  ledger.records[key]=record;records.push(record);
 }
 return {ledger,records};
}
export function recruitGameEvents(s){
 const relevant=new Set(s.recruitInterests??[]),statuses=new Map((s.recruitStatuses??[]).map(r=>[r.player,r])),players=new Map(s.players.map(p=>[p.id,p]));
 // Occasional scouting clues share the ordinary feed budget. Do not promote
 // an entire simulated class into national news.
 return [...(s.recruitGameRecords??[])].sort((a,b)=>Number(relevant.has(b.player))-Number(relevant.has(a.player))||hash(a.key)-hash(b.key)).slice(0,3).flatMap(record=>{
  const p=players.get(record.player),r=statuses.get(record.player);if(!p||identity(p)!==record.identity||record.week!==s.season.CurrentWeek||record.season!==s.season.CurrentSeasonYear)return [];
  const team=r.topSchools?.find(id=>s.teams.some(t=>t.id===id&&id!==255))??s.userTeamIndex;
  return [{id:'recruit-game:'+record.key,category:'recruit-film',facts:{family:'recruit.game.'+record.group,voice:'recruit',...record.values,recordSource:record.source,playerName:p.name,position:p.position},options:{team,player:p.id,national:false,priority:relevant.has(p.id)?69:43,thread:'recruit-film:'+p.id}}];
 });
}
