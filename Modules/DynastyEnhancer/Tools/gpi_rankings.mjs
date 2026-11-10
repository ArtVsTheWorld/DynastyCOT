// These are COT grade-board honors, not native awards or proprietary film grades.
const offense=new Set(['QB','RB','WR','TE','OL']),defense=new Set(['DL','LB','CB','S']);
export const gpiPosition=p=>({LE:'LEDG',RE:'REDG',LOLB:'SAM',MLB:'MIKE',ROLB:'WILL'}[p]??p);
const fit=s=>[...s].length<=248&&Buffer.byteLength(s,'utf8')<=440;
const gradeOrder=(a,b)=>b.grade-a.grade||b.snaps-a.snaps||a.player.localeCompare(b.player);
const uniqueWinner=rows=>{const sorted=[...rows].sort((a,b)=>b.grade-a.grade);return sorted.length&&(!sorted[1]||sorted[0].grade!==sorted[1].grade)?sorted[0]:null;};
const positionGroup=p=>p==='QB'?'QB':['HB','FB'].includes(p)?'RB':['LT','LG','C','RG','RT'].includes(p)?'OL':['LEDG','REDG','DT'].includes(p)?'DL':['SAM','MIKE','WILL'].includes(p)?'LB':['FS','SS'].includes(p)?'S':p;
const boardPosition=p=>({FS:'Free Safety',SS:'Strong Safety',LEDG:'Left Edge',REDG:'Right Edge',SAM:'SAM Linebacker',MIKE:'MIKE Linebacker',WILL:'WILL Linebacker'}[p]??p);
export function gpiRankings(s,a,old=null){
 const c=a.coverage;if(c?.format!==1||!c.scheduleComplete||!Array.isArray(c.fbsTeams)||!Array.isArray(c.weeklyTeams)||!Array.isArray(c.positions)||!Array.isArray(c.trackedPositions))return [];
 const teams=new Map(s.teams.filter(t=>c.fbsTeams.includes(t.id)).map(t=>[t.id,t])),players=new Map(s.players.map(p=>[p.id,p]));
 if(!teams.size||!c.weeklyTeams.length||c.fbsTeams.some(id=>!teams.has(id))||c.weeklyTeams.some(id=>!teams.has(id)))return [];
 const valid=r=>{const p=players.get(r.player);return teams.has(r.team)&&p?.team===r.team&&p.rosterEligible!==false&&gpiPosition(p.position)===gpiPosition(r.position)&&positionGroup(gpiPosition(r.position))===r.group&&Number.isFinite(r.grade)&&r.grade>=0&&r.grade<=100&&Number.isFinite(r.snaps)&&r.snaps>0;};
 const weekly=a.weekly.filter(valid),season=a.seasonRows.filter(valid);
 // Never silently remove a malformed/duplicated candidate from a national field.
 if(weekly.length!==a.weekly.length||season.length!==a.seasonRows.length||weekly.some(r=>!c.weeklyTeams.includes(r.team))||new Set(weekly.map(r=>r.player)).size!==weekly.length||new Set(season.map(r=>r.player)).size!==season.length)return [];
 const weekLabel=a.weekType==='RegularSeason'?`Week ${a.week}`:`${a.weekType} Week ${a.week}`;
 const base={voice:'analyst',persona:6,analystPersona:6,modelVersion:a.modelVersion,sourceWeek:a.week,sourceWeekType:a.weekType,weekLabel,statBased:true};
 const event=(kind,identity,facts,teamIds,player=null)=>({id:`pff:${kind}:${identity}`,category:'analytics',facts:{...base,...facts,family:'pff.'+kind},options:{team:teamIds[0],relatedTeams:teamIds.slice(1),player,national:true,priority:89,eventWeek:s.season.CurrentWeek}});
 const result=[];
 const positions=['QB','HB','FB','WR','TE','LT','LG','C','RG','RT','LEDG','REDG','DT','SAM','MIKE','WILL','CB','FS','SS','K','P'];
 for(let i=0;i<positions.length;i++){
  const position=positions[(s.season.CurrentWeek+i)%positions.length];if(!c.positions.includes(position))continue;
  const rows=season.filter(r=>gpiPosition(r.position)===position&&r.gradedGames>=3&&(['K','P'].includes(r.group)?r.opportunities>=8:r.snaps>=200)).sort(gradeOrder);
  if(rows.length<3||rows[3]?.grade===rows[2].grade)continue;
  const leaders=rows.slice(0,3);
  const makeList=compact=>leaders.map((r,i)=>{const p=players.get(r.player),t=teams.get(r.team),rank=1+leaders.filter(other=>other.grade>r.grade).length,tied=leaders.filter(other=>other.grade===r.grade).length>1;
   const name=compact&&p.firstName&&p.lastName?p.firstName[0]+'. '+p.lastName:p.name,school=compact?t.shortName||t.name:t.name;
   return `${tied?'T':''}${rank}. ${name} (${school}) ${r.grade.toFixed(1)}`;}).join('\n');
  const label=boardPosition(position);let topThree=makeList(false);if(!fit(`Three names leading the ${label} grade board through ${weekLabel}:\n${topThree}`))topThree=makeList(true);
  if(!fit(`Three names leading the ${label} grade board through ${weekLabel}:\n${topThree}`)||new Set(topThree.split('\n').map(x=>x.replace(/^(T?\d+)\. /,'').replace(/ [\d.]+$/,''))).size!==3)continue;
  result.push(event('position-top-three',position,{position:label,positionKey:position,topThree,leaderboard:leaders.map(r=>({player:r.player,team:r.team,grade:r.grade,snaps:r.snaps,gradedGames:r.gradedGames})),qualifiedPlayers:rows.length},[...new Set(leaders.map(r=>r.team))]));break;
 }
 const newWeek=!old||old.week!==a.week||old.weekType!==a.weekType;
 if(!newWeek)return result;
 const complete=groups=>{const tracked=c.trackedPositions.filter(p=>groups.has(positionGroup(p)));return tracked.length>0&&tracked.every(p=>c.positions.includes(p));};
 const playerHonor=(kind,groups)=>{
  if(!complete(groups))return null;
  const winner=uniqueWinner(weekly.filter(r=>groups.has(r.group)&&r.snaps>=20));if(!winner)return null;
  const p=players.get(winner.player);return event(kind,winner.player,{playerName:p.name,school:teams.get(winner.team).name,position:p.position,grade:winner.grade.toFixed(1),snaps:winner.snaps,sampleLabel:`${winner.snaps} recorded snaps`,rankingBasis:'highest qualified weekly individual grade'},[winner.team],winner.player);
 };
 const teamHonor=(kind,groups)=>{
  if(!complete(groups)||!c.weeklyTeams.length)return null;const grades=[];
  for(const team of c.weeklyTeams){const rows=weekly.filter(r=>r.team===team&&groups.has(r.group)),qualified=rows.filter(r=>r.snaps>=20),n=g=>qualified.filter(r=>g.includes(r.group)).length;
   if(qualified.length<8)return null;
   // Roster positions need not equal the position played on every snap. Five
   // qualified linemen can include an RT filling in at LT; nickel fronts vary.
   if(groups===offense&&(n(['QB'])<1||n(['RB','WR','TE'])<2||n(['OL'])<5))return null;
   if(groups===defense&&(n(['DL'])<2||n(['LB'])<1||n(['CB','S'])<3))return null;
   const snaps=rows.reduce((sum,r)=>sum+r.snaps,0);grades.push({team,grade:Math.round(rows.reduce((sum,r)=>sum+r.grade*r.snaps,0)/snaps*10)/10,snaps,players:rows.length});
  }
  const winner=uniqueWinner(grades);return winner?event(kind,winner.team,{school:teams.get(winner.team).name,teamGrade:winner.grade.toFixed(1),unitPlayers:winner.players,unitSnaps:winner.snaps,rankingBasis:'snap-weighted qualified unit; all played FBS teams covered'},[winner.team]):null;
 };
 // Two weekly slots alternate players/units; a changed Top 3 and the existing
 // program spotlight share the other slots. New families do not add post slots.
 const playersFirst=a.week%2===0;
 for(const [groups,playerKind,teamKind] of [[offense,'offensive-player-of-week','offense-of-week'],[defense,'defensive-player-of-week','defense-of-week']]){
  const p=()=>playerHonor(playerKind,groups),t=()=>teamHonor(teamKind,groups),honor=playersFirst?p()??t():t()??p();if(honor)result.push(honor);
 }
 return result;
}
