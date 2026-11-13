// Native final score + the resolved account's school establish allegiance.
// No prose substitutions, inferred incidents or personal-account changes.
export function supporterContext(game,school,opponent,facts={}){
 if(!game||!school||!opponent||![game.home,game.away].includes(school.id)||opponent.id==school.id||![game.home,game.away].includes(opponent.id))return null;
 const home=school.id===game.home,own=home?game.homeScore:game.awayScore,other=home?game.awayScore:game.homeScore;
 if(!Number.isFinite(own)||!Number.isFinite(other)||own<0||other<0||own===other)return null;
 const won=own>other,margin=Math.abs(own-other),rank=home?facts.awayRank:facts.homeRank,ownRank=home?facts.homeRank:facts.awayRank;
 const ranked=n=>Number.isInteger(n)&&n>=1&&n<=25;
 const kind=won?facts.rivalry?'rivalry':!ranked(ownRank)&&ranked(rank)?'upset':margin<=7?'relief':'win':margin>=21?'heavy-loss':'loss';
 return {family:'supporter.'+kind,values:{school:school.name,nickname:school.nickname||school.name,opponent:opponent.name,ownScore:own,opponentScore:other,score:own+'-'+other}};
}
export function capturedSupporterContext(event,school,teams){
 const f=event.facts??{};
 // fanAccountEvents captures score in the account school's order from a
 // verified final native game. Keep that provenance when the full game graph
 // is no longer in the export; never infer allegiance from winner-first text.
 if(event.category!=='results'||!event.id?.startsWith('barstool:')||!event.thread?.startsWith('game:')||
    !['fanclub.win','fanclub.loss','fanclub.bad-loss','fanclub.rivalry-win'].includes(f.family)||
    !school||school.id!==event.teamIds?.[0]||f.school!==school.name)return null;
 const other=teams.get(event.teamIds[1]);if(!other||f.opponent!==other.name||other.id===school.id)return null;
 const score=/^(\d{1,3})[-\u2013](\d{1,3})$/.exec(f.score??'');if(!score)return null;
 const own=Number(score[1]),against=Number(score[2]);if(own===against||typeof f.negative!=='boolean'||f.negative!==(own<against))return null;
 return supporterContext({home:school.id,away:other.id,homeScore:own,awayScore:against},school,other,{rivalry:f.family==='fanclub.rivalry-win'});
}
