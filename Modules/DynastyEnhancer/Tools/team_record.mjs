// Prefer captured current-season totals. Home/Road omit neutral-site games.
// Older captures may lack total stats: include explicit neutral counters then.
export function teamRecord(team,stats=null){
 const valid=n=>Number.isInteger(n)&&n>=0;
 const total=(field,home,road,neutral)=>valid(stats?.[field])?stats[field]:(valid(team[home])?team[home]:0)+(valid(team[road])?team[road]:0)+(valid(team[neutral])?team[neutral]:0);
 return {wins:total('WINS','HomeWin','RoadWin','NeutralWin'),losses:total('LOSSES','HomeLoss','RoadLoss','NeutralLoss'),ties:total('TIES','HomeTie','RoadTie','NeutralTie')};
}
