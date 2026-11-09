// LeagueID is a native 32-bit integer. Accept its signed/unsigned JSON forms,
// never coerce strings, fractions, missing IDs or wider integers into a match.
export function canonicalLeagueId(id){
 return Number.isInteger(id)&&id>=-2147483648&&id<=4294967295?id>>>0:null;
}
export function sameLeague(a,b){const id=canonicalLeagueId(a);return id!==null&&id===canonicalLeagueId(b);}
