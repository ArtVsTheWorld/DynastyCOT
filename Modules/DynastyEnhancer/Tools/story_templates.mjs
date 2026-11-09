import fs from 'node:fs';
import {socialDisplayText} from './prose_cleanup.mjs';
import {comparePublication} from './publication_timeline.mjs';
import {nativeViewIds} from './feed_visibility.mjs';
import {templateFeatures,varietyMemory,rememberVariety,repetitionCost,structuralNeighbors,nearUse,globalNearUse} from './story_variety.mjs';
export const contentPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/first-pack.json',import.meta.url),'utf8'));
const pffPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/pff-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,pffPack.families);
const expansion=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/analysis-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,expansion.families);contentPack.revision=expansion.revision;
const preseason=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/preseason-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,preseason.families);contentPack.revision=preseason.revision;
const launch=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/season-launch-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,launch.families);contentPack.revision=launch.revision;
const accounts=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/account-refresh-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,accounts.families);contentPack.revision=accounts.revision;
const playoff=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/playoff-desk-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,playoff.families);contentPack.revision=playoff.revision;
const historyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/history-analysis-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,historyPack.families);contentPack.revision=historyPack.revision;
const matchupPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/matchup-season-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,matchupPack.families);contentPack.revision=matchupPack.revision;
const postseasonPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/postseason-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,postseasonPack.families);contentPack.revision=postseasonPack.revision;
const contextPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/context-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,contextPack.families);contentPack.revision=contextPack.revision;
const transactionsPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/transactions-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,transactionsPack.families);contentPack.revision=transactionsPack.revision;
const cyclePack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/cycle-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,cyclePack.families);contentPack.revision=cyclePack.revision;
const reviewPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/season-review-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,reviewPack.families);contentPack.revision=reviewPack.revision;
const portalPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/portal-draft-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,portalPack.families);contentPack.revision=portalPack.revision;
const discoursePack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/offseason-discourse-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,discoursePack.families);contentPack.revision=discoursePack.revision;
const carryPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/offseason-carryover-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,carryPack.families);contentPack.revision=carryPack.revision;
const editorialPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/editorial-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,editorialPack.families);contentPack.revision=editorialPack.revision;
const correctionsPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/review-corrections-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,correctionsPack.families);contentPack.revision=correctionsPack.revision;
const analyticsVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/analytics-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,analyticsVarietyPack.families);contentPack.revision=analyticsVarietyPack.revision;
const voiceVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/voice-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,voiceVarietyPack.families);contentPack.revision=voiceVarietyPack.revision;
const eventVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/event-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,eventVarietyPack.families);contentPack.revision=eventVarietyPack.revision;
const reactionVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/reaction-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,reactionVarietyPack.families);contentPack.revision=reactionVarietyPack.revision;
const cycleVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/cycle-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,cycleVarietyPack.families);contentPack.revision=cycleVarietyPack.revision;
const buzzVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/buzz-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,buzzVarietyPack.families);contentPack.revision=buzzVarietyPack.revision;
const retrospectiveVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/retrospective-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,retrospectiveVarietyPack.families);contentPack.revision=retrospectiveVarietyPack.revision;
const openingVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/opening-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,openingVarietyPack.families);contentPack.revision=openingVarietyPack.revision;
const raceVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/race-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,raceVarietyPack.families);contentPack.revision=raceVarietyPack.revision;
const recruitingVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/recruiting-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,recruitingVarietyPack.families);contentPack.revision=recruitingVarietyPack.revision;
const reporterVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/reporter-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,reporterVarietyPack.families);contentPack.revision=reporterVarietyPack.revision;
const matchupVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/matchup-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,matchupVarietyPack.families);contentPack.revision=matchupVarietyPack.revision;
const fanVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/fan-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,fanVarietyPack.families);contentPack.revision=fanVarietyPack.revision;
const finalCycleVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/final-cycle-variety-pack.json',import.meta.url),'utf8'));
Object.assign(contentPack.families,finalCycleVarietyPack.families);contentPack.revision=finalCycleVarietyPack.revision;
const compactVarietyPack=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/compact-variety-pack.json',import.meta.url),'utf8'));
for(const [family,lines] of Object.entries(compactVarietyPack.families))contentPack.families[family].push(...lines);
contentPack.revision=compactVarietyPack.revision;
const perspectives=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/perspectives-pack.json',import.meta.url),'utf8'));
for(const [family,lines] of Object.entries(perspectives.families)){
 if(!contentPack.families[family])contentPack.families[family]=[];
 contentPack.families[family].push(...lines);
}
contentPack.revision=perspectives.revision;
Object.assign(contentPack.publicationPolicy,perspectives.publicationPolicy??{});
const beyond=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/beyond-field-pack.json',import.meta.url),'utf8'));
for(const [family,lines] of Object.entries(beyond.families))(contentPack.families[family]??=[]).push(...lines);
contentPack.revision=beyond.revision;

// These stock endings add no information to an otherwise factual analysis.
const emptyEndings=['The whole stat line belongs in the conversation.','Worth keeping alongside the headline numbers.','A profile worth following as the evaluation develops.','A candidate, not a coronation.','A useful unit comparison before kickoff.','That changes the conversation.','It changes the conversation.','That changes the conversation around {school}.'];
for(const variants of Object.values(contentPack.families))for(const t of variants)for(const ending of emptyEndings)t.body=t.body.replace(ending,'').trim();
// Reviewed ID patches leave guards, personalities and publication identities in
// place. Reject a stale proposal rather than silently patching different prose.
const refresh=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/editorial011-pack.json',import.meta.url),'utf8'));
const originalTemplates=new Map(Object.entries(contentPack.families).flatMap(([family,ts])=>ts.map(t=>[t.id,{family,t}])));
for(const patch of refresh.overrides){const old=originalTemplates.get(patch.id);if(!old||old.family!==patch.family||old.t.body!==patch.originalBody)throw Error('Stale editorial patch '+patch.id);old.t.body=patch.body;}
for(const [family,lines] of Object.entries(refresh.families)){if(!contentPack.families[family])throw Error('Unknown editorial family '+family);contentPack.families[family].push(...lines);}
contentPack.revision=refresh.revision+'+'+pffPack.revision;
const seasonReview=JSON.parse(fs.readFileSync(new URL('../Content/Runtime/season-fixes-pack.json',import.meta.url),'utf8'));
const reviewedTemplates=new Map(Object.entries(contentPack.families).flatMap(([family,ts])=>ts.map(t=>[t.id,{family,t}])));
for(const patch of seasonReview.overrides){const old=reviewedTemplates.get(patch.id);if(!old||old.family!==patch.family||old.t.body!==patch.originalBody)throw Error('Stale season-review patch '+patch.id);old.t.body=patch.body;}
contentPack.revision+='+'+seasonReview.revision;
// Apply after the exact originalBody checks above. Editing the source packs
// or normalizing first would weaken those reviewed editorial patch guards.
for(const variants of Object.values(contentPack.families))for(const template of variants)template.body=socialDisplayText(template.body);
const slots=/\{([A-Za-z][A-Za-z0-9]*)\}/g,ids=new Set(),slotCache=new WeakMap();
for(const variants of Object.values(contentPack.families))for(const t of variants){
 if(!t.id||ids.has(t.id)||typeof t.body!=='string'||/[\uFFFD\u{1F000}-\u{1FAFF}\u2600-\u27BF]/u.test(t.body))throw Error('Invalid runtime template '+t.id);ids.add(t.id);
}
const historyKey=(publication,week)=>publication?`${publication.leagueId}|${publication.userTeamIndex}|${publication.season}|${publication.phase}|${publication.week}`:`week:${week}`;
const templateCatalog=new Map(Object.values(contentPack.families).flat().map(t=>[t.id,t]));
const visibleHistory=new WeakMap();
function historyWindow(f){
 // Published edition objects are immutable. Both generation passes reuse this
 // projection; the weak key releases it when the archive leaves memory.
 const cached=visibleHistory.get(f);if(cached&&cached.stories===f.stories&&cached.views===f.views&&cached.count===(f.stories?.length??0))return cached.ids;
 const ids=new Set();for(const view of ['all','team','conference','national'])for(const id of nativeViewIds(f,view))ids.add(id);
 visibleHistory.set(f,{stories:f.stories,views:f.views,count:f.stories?.length??0,ids});return ids;
}
export function templateHistory(history=[],publication=null,week=null){
 const scoped=publication?history.filter(f=>f.season<=publication.season&&f.season>=publication.season-1&&(f.leagueId==null||f.leagueId===publication.leagueId)&&(f.userTeamIndex==null||f.userTeamIndex===publication.userTeamIndex)&&(f.season<publication.season||comparePublication(f,publication)<0)).sort((a,b)=>a.season-b.season||comparePublication(a,b)):history.filter(f=>f.week<week).sort((a,b)=>a.week-b.week);
 const recent=publication?scoped.slice(-contentPack.publicationPolicy.templateCooldownWeeks):scoped.filter(f=>week-f.week<=contentPack.publicationPolicy.templateCooldownWeeks);
 const memoryEditions=contentPack.publicationPolicy.structuralMemoryEditions??4,wording=publication?scoped.slice(-memoryEditions):scoped.filter(f=>week-f.week<=memoryEditions);
 const byHandle=new Map();for(let i=0;i<recent.length;i++)for(const p of recent[i].stories??[]){const used=byHandle.get(p.handle)??new Map();used.set(p.templateId,i);if(p.voiceTemplateId)used.set(p.voiceTemplateId,i);byHandle.set(p.handle,used);}
 const memory=varietyMemory();for(let i=0;i<wording.length;i++){
  const f=wording[i],visible=historyWindow(f);
  // The native UI now shows complete local scopes and a selected 250-card
  // National view. Track those actual windows, not the retired 30/100 caps.
  for(const p of f.stories??[]){if(visible.size&&!visible.has(p.id))continue;const prior=p.variety??(templateCatalog.has(p.templateId)?templateFeatures(templateCatalog.get(p.templateId)):null);rememberVariety(memory,prior,(i+1)/wording.length,p.varietyTopic??'');}
 }
 memory.last=null;return {history,key:historyKey(publication,week),byHandle,memory};
}
function requiredSlots(t){let cached=slotCache.get(t);if(!cached||cached.body!==t.body){cached={body:t.body,keys:[...new Set([...t.body.matchAll(slots)].map(m=>m[1]))]};slotCache.set(t,cached);}return cached.keys;}
export function chooseTemplate(family,values,seed,{handle,week,history=[],sameStory,renderKey,transformBody=body=>body,style=null,voice=null,topic='',avoidBodies=new Set(),publication=null,onCandidates=null,usage=null}={}){
 const eligible=(contentPack.families[family]??[]).filter(t=>requiredSlots(t).every(k=>values[k]!==null&&values[k]!==undefined)&&(!t.when||t.when.every(k=>values[k]!==null&&values[k]!==undefined&&values[k]!==false&&values[k]!==''))&&(!t.unless||t.unless.every(k=>!values[k])));
 // Publication wording survives content upgrades. Changed verified facts still
 // get a new renderKey and are reconciled normally.
 if(sameStory&&renderKey&&sameStory.renderKey===renderKey&&typeof sameStory.body==='string')return {body:sameStory.body,templateId:sameStory.templateId,cooldownFallback:sameStory.cooldownFallback??false,...(sameStory.variety?{variety:sameStory.variety}:{})};
 const context=usage?.history===history&&usage.key===historyKey(publication,week)?usage:templateHistory(history,publication,week),lastUsed=context.byHandle.get(handle)??new Map();
 // Prefer a different phrase for this account across weeks. If all valid phrases
 // are cooling down, use the least recent one rather than inventing news/facts.
 const seedPrefix=score('',seed+'|',true),scores=new Map(eligible.map(t=>[t,score(seedPrefix,t.id,true)])),neighbors=structuralNeighbors(contentPack.families[family]??[]),near=new Map(eligible.map(t=>[t,Math.max(nearUse(context.memory,neighbors.get(t)),globalNearUse(context.memory,t))]));
 const costs=new Map(eligible.map(t=>[t,repetitionCost(context.memory,t,topic)+near.get(t)*48+(voice&&t.voices&&!t.voices.includes(voice)?18:0)]));
 const compare=(a,b)=>Number(near.get(a)>=.2)-Number(near.get(b)>=.2)||costs.get(a)-costs.get(b)||Number(lastUsed.has(a.id))-Number(lastUsed.has(b.id))||Number(!!a.fallback)-Number(!!b.fallback)||(style?Number(a.style!==style)-Number(b.style!==style):0)||(lastUsed.get(a.id)??-1)-(lastUsed.get(b.id)??-1)||scores.get(a)-scores.get(b)||a.id.localeCompare(b.id);
 eligible.sort(compare);
 const render=t=>{const body=transformBody(t.body.replace(slots,(_,k)=>values[k])).replace(/[ \t]+\n/g,'\n').replace(/[ \t]{2,}/g,' ').trim();return body&&!/[\uFFFD\x00-\x08\x0b\x0c\x0e-\x1f]/u.test(body)&&[...body].length<=280&&Buffer.byteLength(body,'utf8')<=512?body:null;};
 // Persist the compact signature. Phrase indexes are rebuilt from its shape,
 // avoiding a repeated phrase array on every archived card.
 const choice=(t,body)=>{const {phrases,...variety}=templateFeatures(t);return {body,templateId:t.id,cooldownFallback:lastUsed.has(t.id),variety};};
 // Audits deliberately render every candidate. The normal path transforms only
 // as many as needed to find the same best valid, non-repeated wording.
 if(onCandidates){const ranked=eligible.map(t=>({t,body:render(t)})).filter(x=>x.body).sort((a,b)=>Number(avoidBodies.has(a.body))-Number(avoidBodies.has(b.body))||compare(a.t,b.t));onCandidates({family,values,style,candidates:ranked.map(({t,body})=>({id:t.id,body,style:t.style??null,angle:t.angle??null,structure:templateFeatures(t).structure,recentlyUsed:context.memory.forms.has(templateFeatures(t).structure)}))});return ranked.length?choice(ranked[0].t,ranked[0].body):null;}
 let repeated=null;for(const t of eligible){const body=render(t);if(!body)continue;const result=choice(t,body);if(!avoidBodies.has(body))return result;repeated??=result;}
 return repeated;
}
function score(seed,id,continuation=false){let n=typeof seed==='number'?seed:2166136261;for(const c of continuation?id:seed+'|'+id){n^=c.codePointAt(0);n=Math.imul(n,16777619);}return n>>>0;}
