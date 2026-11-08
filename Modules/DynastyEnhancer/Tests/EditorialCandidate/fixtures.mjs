// Explicit synthetic acceptance fixtures, never imported into published history.
import {worldEvents} from '../../Tools/world_events.mjs';
export const fixture=()=>({format:2,gameYear:27,sourceKind:'runtime',sourceUnchanged:true,sha256:'a'.repeat(64),leagueId:70111,userTeamIndex:111,season:{CurrentStage:'NFLSeason',CurrentYear:0,CurrentSeasonYear:2026,CurrentWeekType:'RegularSeason',CurrentWeek:2},teams:[{id:111,name:'W. Michigan',nickname:'Broncos',logo:111,prestigeStars:1,mediaRank:99,mediaLastRank:99,rivals:[]},{id:49,name:'Michigan',nickname:'Wolverines',logo:49,prestigeStars:4.5,mediaRank:14,mediaLastRank:14,rivals:[50]},{id:50,name:'Michigan State',nickname:'Spartans',logo:50,prestigeStars:3.5,mediaRank:99,rivals:[49]}],players:[],games:[],performances:[],heisman:[],commitments:[],offers:[],recruitStatuses:[],recruitInterests:[]});
export function upsetFixture(){
 const s=fixture();s.games=[{id:'fixture-wmu-michigan',year:0,phase:'RegularSeason',week:1,status:'AwayWon',published:true,home:49,away:111,homeName:'Michigan',awayName:'W. Michigan',homeScore:26,awayScore:31}];s.pregameGames={'fixture-wmu-michigan':{ranks:{111:99,49:14}}};
 const event=worldEvents(s).find(e=>e.category==='upsets');return {snapshot:s,events:[{...event,...event.options,teamIds:[111,49],scopes:['national','team'],priority:event.options.priority,thread:event.options.thread}],phase:'RegularSeason',week:2,authors:{}};
}
export function supplementalEditions(){
 const editions=[upsetFixture()];
 const add=(name,phase,week,family,values,options={})=>{
  const s=fixture();s.leagueId+=editions.length;Object.assign(s.season,{CurrentWeekType:phase,CurrentWeek:week});
  if(options.long){s.teams[0].name='Northwestern Metropolitan University of the Great Lakes';s.teams[0].nickname='Northwestern Metropolitan Captains';}
  const id='fixture-'+name;s.players=[{id:'p',name:options.long?'Christopher Alexander Montgomery-Washington III':'Evan Moss',position:'QB',team:options.recruit?255:111,personality:'Leader',presentationId:501}];
  const facts={family,voice:options.voice??'beat',school:s.teams[0].name,nickname:s.teams[0].nickname,playerName:s.players[0].name,position:'QB',...values};
  editions.push({snapshot:s,authors:{},phase,week,events:[{id,category:options.category??'analytics',teamIds:[111],playerId:'p',scopes:['team','conference','national'],priority:90,thread:id,facts}]});
 };
 add('sparse','RegularSeason',3,'narrative.game',{winner:'W. Michigan',loser:'Michigan',score:'31–26',turningPoint:null,detail:null},{category:'results'});
 add('long-names','RegularSeason',4,'roster.commit',{starDescription:'5-star ',prospectType:'high school ',originDetail:' from Texas',recruitRankDetail:' No. 12 nationally.'},{category:'recruiting',long:true,recruit:true});
 add('recruit-game','RegularSeason',9,'recruit.game.specialist',{recordSource:'mod-modeled-hs',kickMade:2,kickAttempts:3,punts:5,puntAverage:42,isKicker:true,isPunter:false},{category:'recruit-film',voice:'recruit',recruit:true});
 editions.at(-1).snapshot.players[0].position='K';
 add('postseason','NationalChampionship',20,'team.win',{score:'31–26',opponent:'Michigan',hashtag:'#Broncos'},{category:'championship',voice:'official'});
 add('offseason-draft','OffSeason',2,'draft.pick',{draftPick:7,draftRound:1,nflTeam:'Detroit Lions'},{category:'draft',voice:'nflDraft',long:true});
 return editions;
}
