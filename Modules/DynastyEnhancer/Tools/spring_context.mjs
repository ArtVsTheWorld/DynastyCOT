// Extensions 0.3 creates real PreSeason SeasonGame rows, including a non-played
// marker (250). Keep only context here; these rows never enter competitive facts.
export function exhibitionContext(rows,season,userTeam){
 const current=rows.filter(g=>g.year===season.CurrentYear);
 const fixtures=current.filter(g=>g.number!==250&&g.home!==null&&g.away!==null&&g.home!==g.away);
 const user=fixtures.filter(g=>g.home===userTeam||g.away===userTeam);
 const supported=fixtures.length>0||current.some(g=>g.number===250);
 return {supported,active:season.CurrentWeekType==='PreSeason'&&supported,
  fixtures:user.map(g=>({id:g.id,week:g.week,status:g.status})),marker:current.some(g=>g.number===250)};
}
export function preseasonPublicationContext(s){
 if(s.exhibitions?.active)return 'spring';
 return 'fall';
}
export function fallOpeningAfterSpring(s){
 return s.exhibitions?.supported===true&&s.season.CurrentWeekType==='RegularSeason'&&
  s.season.CurrentStage==='NFLSeason'&&s.season.CurrentWeek===0&&
  s.games.some(g=>g.phase==='RegularSeason'&&g.year===s.season.CurrentYear&&g.status==='Unplayed');
}
