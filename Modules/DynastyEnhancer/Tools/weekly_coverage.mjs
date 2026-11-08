import {WEEKLY_TEAM_POST_LIMIT,enforceWeeklyBudget} from './weekly_budget.mjs';
// One routine allowance for every program. Replies/statlines are exempt;
// primary news survives special weeks even when it exceeds the allowance.
export function teamPostBudget(){return WEEKLY_TEAM_POST_LIMIT;}
export function applyWeeklyCoverage(stories,s,{sameWeek=null}={}){
 return enforceWeeklyBudget(stories,[],new Set(sameWeek?.stories?.map(p=>p.id)??[]));
}
