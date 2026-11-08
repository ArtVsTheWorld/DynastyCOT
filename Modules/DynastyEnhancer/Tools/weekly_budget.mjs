export const WEEKLY_TEAM_POST_LIMIT=15;
const reporters=new Set(['beat','analyst','official','cfp','nflDraft','model']);
export function protectedNews(event,role){
 const f=event.facts??{};
 if((event.id?.startsWith("commit:")||event.category==="signings")&&["beat","analyst"].includes(role))return true;
 if(reporters.has(role)&&f.family==='review.retention-watch')return true;
 if(['recruiting','signings'].includes(event.category)&&role!=='fan'&&(f.signed||f.flipped||f.decommitted||f.newlyObserved&&['Signed','HardCommitted'].includes(f.commitment)))return true;
 if(reporters.has(role)&&(['coaching'].includes(event.category)||['departure','realignment','alignment-history','rename'].includes(f.carryoverKind)||f.family==='transactions.position-change'))return true;
 return reporters.has(role)&&(['results','upsets','awards','records','injuries','polls','cfp','championship','bowls','draft'].includes(event.category)||f.importance==='major');
}
export function weeklyBudget(){
 const counts=new Map();
 return {
  allows(event,role,{reply=false,published=false}={}){
   return reply||published||event.category==='recruit-film'||protectedNews(event,role)||(event.teamIds??[]).filter(id=>id!==255).every(id=>(counts.get(id)??0)<WEEKLY_TEAM_POST_LIMIT);
  },
  record(event,{reply=false}={}){if(reply||event.category==='recruit-film')return;for(const id of new Set(event.teamIds??[]))if(id!==255)counts.set(id,(counts.get(id)??0)+1);},
  counts
 };
}
export function budgetEventOrder(events){
 const key=e=>protectedNews(e,'beat')?1:0;
 return [...events].sort((a,b)=>key(b)-key(a)||(b.priority??0)-(a.priority??0)||a.id.localeCompare(b.id));
}
export function enforceWeeklyBudget(stories,events,publishedIds=new Set()){
 const byEvent=new Map(events.map(e=>[e.id,e])),budget=weeklyBudget(),kept=new Set();
 const event=p=>byEvent.get(p.eventId)??{category:p.category,teamIds:p.teamIds,facts:{}};
 const protectedPost=p=>p.budgetProtected||p.essentialNews||p.coverageExempt||protectedNews(event(p),p.voice);
 const roots=stories.filter(p=>!p.replyTo).sort((a,b)=>Number(publishedIds.has(b.id))-Number(publishedIds.has(a.id))||Number(!!protectedPost(b))-Number(!!protectedPost(a))||(b.priority??0)-(a.priority??0)||a.id.localeCompare(b.id));
 for(const p of roots){const e=event(p);if(budget.allows(e,p.voice,{published:publishedIds.has(p.id)||!!protectedPost(p)})){kept.add(p.id);budget.record(e);}}
 // Preserve actual ancestry; exempt replies do not keep a discarded root alive.
 let changed=true;while(changed){changed=false;for(const p of stories)if(p.replyTo&&kept.has(p.replyTo)&&!kept.has(p.id)){kept.add(p.id);changed=true;}}
 return stories.filter(p=>kept.has(p.id));
}
