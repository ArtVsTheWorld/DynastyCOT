import {hash} from './story_authors.mjs';
const topics=['uniforms','traditions','tailgating','campus','conference','rivalries','arguments','optimism'];
const archive=['boise','texas','ohio','nebraska','miami','bc','lsu','georgia','tennessee','appstate','cal','ringo'];
export function casualEvents(s){
 if(s.reconstructed)return [];
 const own=s.teams.find(t=>t.id===s.userTeamIndex);if(!own)return [];
 const key=[s.leagueId,s.season.CurrentSeasonYear,s.season.CurrentWeekType,s.season.CurrentWeek].join('|'),seed=hash(key+'|chatter'),events=[];
 const athletes=s.players.filter(p=>p.team===own.id&&p.injury==='Uninjured').sort((a,b)=>a.id.localeCompare(b.id));
 // Two low-priority candidates; the unchanged 30-card default editorial budget
 // decides whether either has room. These are opinions, not invented events.
 for(let slot=0;slot<2;slot++){
  const topic=topics[(seed+slot*3)%topics.length],personal=slot===0&&athletes.length&&seed%3===0,role=personal?'player':slot===0?'fan':seed%2?'analyst':'beat',player=personal?athletes[seed%athletes.length]:null;
  const suffix=personal?'personal':role==='beat'?'local':role;
  events.push({id:`chatter:${key}:${slot}`,category:'chatter',facts:{family:`casual.${topic}`,voice:role,persona:slot===1?1+seed%5:seed%2,topic,opinion:true},options:{team:own.id,player:player?.id??null,national:slot===1,priority:44-slot,thread:`chatter:${key}:${slot}`}});
 }
 if(seed%4===0){const moment=archive[hash(s.leagueId+'|'+s.season.CurrentSeasonYear+'|'+s.season.CurrentWeekType+'|'+Math.floor(s.season.CurrentWeek/4))%archive.length],role=['fan','analyst','beat'][seed%3];events.push({id:`archive:${s.season.CurrentSeasonYear}:${moment}`,category:'chatter',facts:{family:`archive.reference`,['archive_'+moment]:true,voice:role,opinion:true,archiveReference:moment},options:{team:own.id,national:true,priority:41,thread:'archive:'+moment}});}
 return events;
}
