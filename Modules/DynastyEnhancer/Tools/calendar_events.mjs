// Calendar inference is anchored in captured schedule dates, never assumed
// postseason week numbers. Native display Week 13 is the requested rivalry week.
const DAY=86400000;
const weekdays=['Sunday','Monday','Tuesday','Wednesday','Thursday','Friday','Saturday'];
export function gameDate(s,g){
 const season=s.season.CurrentSeasonYear;
 if(!Number.isInteger(season)||!Number.isInteger(g.month)||g.month<1||g.month>12||!Number.isInteger(g.day)||g.day<1||g.day>31)return null;
 const year=season+(g.month<=3?1:0),date=Date.UTC(year,g.month-1,g.day),d=new Date(date);
 if(d.getUTCMonth()!==g.month-1||d.getUTCDate()!==g.day)return null;
 return date;
}
export function editionCalendar(s){
 if(s.season.CurrentStage!=='NFLSeason')return null;
 const games=s.games.filter(g=>g.year===s.season.CurrentYear&&g.phase===s.season.CurrentWeekType&&g.week===s.season.CurrentWeek),rows=games.map(g=>gameDate(s,g)).filter(Number.isFinite).sort((a,b)=>a-b);
 if(!rows.length)return null;
 // A native bowl edition can span several calendar weeks. Its actual date
 // range is authoritative; extending December28 to a full week would publish
 // New Year too early and steal it from the following native bowl edition.
 const anchor=rows[Math.floor(rows.length/2)],weekdayMismatches=games.filter(g=>{const date=gameDate(s,g);return date!==null&&g.weekday&&weekdays[new Date(date).getUTCDay()]!==g.weekday;}).length;
 let start=rows[0],source='captured-schedule-range';
 if(s.season.CurrentWeekType==='RegularSeason'){
  const previous=s.games.filter(g=>g.year===s.season.CurrentYear&&g.phase==='RegularSeason'&&g.week===s.season.CurrentWeek-1).map(g=>gameDate(s,g)).filter(Number.isFinite);
  if(previous.length){const last=Math.max(...previous);if(last<start&&rows.at(-1)-last<=9*DAY){start=last+DAY;source='captured-neighbor-and-current-schedule';}}
 }
 return {start,end:rows.at(-1)+DAY-1,anchor,source,weekdayMismatches};
}
export function calendarEvents(s){
 const events=[],year=s.season.CurrentSeasonYear,window=editionCalendar(s),own=s.teams.find(t=>t.id===s.userTeamIndex);
 const emit=(key,holiday,date=null)=>events.push({id:`calendar:${date?new Date(date).getUTCFullYear():year}:${key}`,category:'holiday',facts:{calendarKey:key,holiday,calendarDate:date?new Date(date).toISOString().slice(0,10):null},options:{team:own?.id??null,national:true,priority:80,thread:'calendar:'+key}});
 if(s.season.CurrentStage==='NFLSeason'&&s.season.CurrentWeekType==='RegularSeason'&&s.season.CurrentWeek===13)emit('rivalry','Rivalry Week');
 if(!window)return events;
 const firstThursday=1+(4-new Date(Date.UTC(year,10,1)).getUTCDay()+7)%7;
 const dates=[['halloween','Halloween',Date.UTC(year,9,31)],['thanksgiving','Thanksgiving',Date.UTC(year,10,firstThursday+21)],['christmas','Christmas',Date.UTC(year,11,25)],['newyear','New Year',Date.UTC(year,11,31)],['newyear','New Year',Date.UTC(year+1,0,1)]];
 for(const [key,label,date] of dates)if(date>=window.start&&date<=window.end&&!events.some(e=>e.facts.calendarKey===key)){
  // Dec31/Jan1 share the approaching year's identity even if native editions
  // put them in separate windows. Editorial history prevents a second burst.
  emit(key,label,key==='newyear'?Date.UTC(year+1,0,1):date);
 }
 return events;
}
