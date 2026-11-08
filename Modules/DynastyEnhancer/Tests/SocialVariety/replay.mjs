// Private corpus stays in Build. Run identical inputs against frozen and working engines.
import fs from 'node:fs';
import path from 'node:path';
import {pathToFileURL} from 'node:url';
import {performance} from 'node:perf_hooks';
import {createHash} from 'node:crypto';
import {requiredCoverageEventIds,assertPrimaryAwardConservation} from './conservation.mjs';
const [engine,input,output]=process.argv.slice(2);
const sourceFiles=['Tools','Content/Runtime'].flatMap(dir=>fs.readdirSync(path.join(engine,dir)).filter(n=>n.endsWith(dir==='Tools'?'.mjs':'.json')).map(n=>dir+'/'+n)).sort(),sourceHash=createHash('sha256');
for(const name of sourceFiles)sourceHash.update(name+'\0').update(fs.readFileSync(path.join(engine,name)));
const sourceFingerprint=sourceHash.digest('hex');
const load=name=>import(pathToFileURL(path.resolve(engine,'Tools',name+'.mjs')));
const [{renderStories},{applyWeeklyCoverage},{publishWeek},{contentPack},{freshEditorialEvents}]=await Promise.all(['story_renderer','weekly_coverage','feed_selection','story_templates','editorial_policy'].map(load));
const corpus=JSON.parse(fs.readFileSync(input,'utf8')),history=new Map(),editions=[],counts=new Map(),eligibility={},timings=[];
const skeleton=t=>t.toLowerCase().replace(/\{[a-z][a-z0-9]*\}/gi,' @ ').replace(/\b\d+(?:[.,–-]\d+)*\b/g,' # ').replace(/[^a-z@#]+/g,' ').trim().replace(/\s+/g,' ');
const byId=new Map(Object.values(contentPack.families).flat().map(t=>[t.id,skeleton(t.body)]));
const shape=p=>{const base=p.variety?.structure??byId.get(p.templateId)??skeleton(p.body);return p.voiceTemplateId&&byId.has(p.voiceTemplateId)?byId.get(p.voiceTemplateId).replace('@',base):base;};
const shingleCache=new Map();const shingles=s=>{if(shingleCache.has(s))return shingleCache.get(s);const w=s.split(' '),v=new Set(w.slice(0,-3).map((_,i)=>w.slice(i,i+4).join(' ')));shingleCache.set(s,v);return v;};
const similar=(a,b)=>{if(a===b)return true;const x=shingles(a),y=shingles(b);if(!x.size||!y.size)return false;let n=0;for(const v of x)if(y.has(v))n++;return n/Math.max(x.size,y.size)>=.70;};
const views=['all','team','conference','national'];
for(const row of corpus.editions){
 const s=row.snapshot,key=s.leagueId+'/'+s.userTeamIndex,prior=history.get(key)??[];
 const publication={leagueId:s.leagueId,userTeamIndex:s.userTeamIndex,season:s.season.CurrentSeasonYear,phase:row.phase,week:row.week},events=freshEditorialEvents(row.events,publication,prior);
 const start=performance.now(),alternativesByRender=new Map(),renderId=(id,body)=>id+'|'+body.toLowerCase();
 const result=renderStories(s,events,{authors:row.authors,history:prior,onTemplates:({family,candidates})=>{
  const e=eligibility[family]??={calls:0,total:0,min:Infinity,max:0,exhaustedCalls:0,unusedTotal:0,usableStructures:new Set()};e.calls++;e.total+=candidates.length;e.min=Math.min(e.min,candidates.length);e.max=Math.max(e.max,candidates.length);const unused=candidates.filter(c=>c.recentlyUsed===false).length;e.unusedTotal+=unused;if(candidates.length&&!unused)e.exhaustedCalls++;for(const c of candidates)e.usableStructures.add(byId.get(c.id));
  if(candidates.length)alternativesByRender.set(renderId(candidates[0].id,candidates[0].body),candidates.map(c=>byId.get(c.id)));
 }});
 const feed=publishWeek({format:1,key:s.leagueId+'/'+s.season.CurrentSeasonYear+'/'+row.phase+'/'+row.week,leagueId:s.leagueId,userTeamIndex:s.userTeamIndex,season:s.season.CurrentSeasonYear,phase:row.phase,week:row.week,
  events,stories:applyWeeklyCoverage(result.stories,s)});
 timings.push(performance.now()-start);
 // Mirror native FeedData.h default type selection, including parent bundles,
 // per-view caps and required draft roots. Candidate pools are not display counts.
 const nativeView=(f,v)=>{const map=new Map(f.stories.map(p=>[p.id,p])),ids=f.views[v==='all'?'all:default':v]??f.views[v]??[],members=new Set(ids),required=new Set(f.views[v+':required']??[]),children=new Map(),out=[],seen=new Set();
  const limit={all:30,national:100,conference:60,team:25}[v];for(const id of ids){const p=map.get(id);if(p.replyTo&&members.has(p.replyTo)){const list=children.get(p.replyTo)??[];list.push(id);children.set(p.replyTo,list);}}
  const append=(id,force)=>{if(seen.has(id)||out.length>=limit&&!force)return;seen.add(id);out.push(map.get(id));for(const c of children.get(id)??[])append(c,force);};
  for(const id of ids){const p=map.get(id);if(!p.replyTo)append(id,required.has(id)||v==='team'&&p.draftPick>0);}return out;};
 const viewPosts=v=>nativeView(feed,v);
 // Callbacks deliberately render all alternatives. Time the real lazy generation separately.
 const lazyAt=performance.now();const lazy=renderStories(s,events,{authors:row.authors,history:prior});
 const lazyMs=performance.now()-lazyAt;
 if(JSON.stringify(lazy.stories)!==JSON.stringify(result.stories))throw Error('Exhaustive/lazy mismatch '+feed.key);
 const priorMetrics=prior.slice(-4),metrics={};
 for(const view of views){const posts=viewPosts(view),recent=priorMetrics.flatMap(f=>nativeView(f,view).map(shape));let duplicates=0,adjacent=0,avoidable=0,matched=0;
  for(let i=0;i<posts.length;i++){const p=posts[i],form=shape(p);if(recent.some(x=>similar(form,x))){duplicates++;const alternatives=alternativesByRender.get(renderId(p.templateId,p.rawBody??p.body));if(alternatives){matched++;if(alternatives.some(a=>{const f=p.voiceTemplateId&&byId.has(p.voiceTemplateId)?byId.get(p.voiceTemplateId).replace('@',a):a;return !recent.some(x=>similar(f,x));}))avoidable++;}}if(i&&similar(form,shape(posts[i-1])))adjacent++;recent.push(form);}
  metrics[view]={cards:posts.length,nearDuplicates:duplicates,adjacent,duplicatesWithAuditedAlternatives:matched,duplicatesWithUnusedAlternative:avoidable};
 }
 for(const p of feed.posts)counts.set(p.templateFamily,(counts.get(p.templateFamily)??0)+1);
 const eventMap=new Map(events.map(e=>[e.id,e]));
 const essential=pool=>requiredCoverageEventIds(pool.filter(p=>{const f=eventMap.get(p.eventId)?.facts??{};return !p.replyTo&&p.voice!=='fan'&&(['results','upsets','awards','records','injuries'].includes(p.category)||p.category==='draft'&&p.voice==='nflDraft'||['recruiting','signings'].includes(p.category)&&(f.signed===true||f.flipped||f.decommitted||f.newlyObserved&&['HardCommitted','Signed'].includes(f.commitment)));}).map(p=>p.eventId),events);
 const primaryAwardConservation=assertPrimaryAwardConservation(events,essential(result.stories),essential(feed.stories));
 editions.push({key:feed.key,user:s.userTeamIndex,phase:row.phase,week:row.week,metrics,lazyMs,generated:result.stories.length,kept:feed.stories.length,essentialGenerated:essential(result.stories),essentialPublished:essential(feed.stories),primaryAwardConservation,
  views:Object.fromEntries(views.map(v=>[v,viewPosts(v).map(p=>({id:p.id,eventId:p.eventId,body:p.body,author:p.author,handle:p.handle,voice:p.voice,replyTo:p.replyTo,family:p.templateFamily,templateId:p.templateId,structure:shape(p),angle:p.variety?.angle}))]))});
 prior.push(feed);history.set(key,prior);
}
const summary={};for(const v of views){summary[v]={cards:0,nearDuplicates:0,adjacent:0,duplicatesWithAuditedAlternatives:0,duplicatesWithUnusedAlternative:0};for(const e of editions)for(const k of Object.keys(summary[v]))summary[v][k]+=e.metrics[v][k];summary[v].rate=summary[v].nearDuplicates/summary[v].cards;}
for(const e of Object.values(eligibility)){e.usableStructures=e.usableStructures.size;e.average=e.total/e.calls;}
const result={revision:contentPack.revision,sourceFingerprint,method:'Four preceding editions plus earlier cards in the same view; exact normalized template skeleton or >=70% overlap of word 4-grams (intersection / larger set). Slot names, names and numbers cannot create diversity. Callbacks count text-budget-valid alternatives.',summary,
 observedTop20:corpus.observedFamilies.slice(0,20),replayedTop20:[...counts].sort((a,b)=>b[1]-a[1]).slice(0,20),eligibility,editions,
 performance:{exhaustiveMs:timings.reduce((a,b)=>a+b,0),lazyMs:editions.reduce((a,b)=>a+b.lazyMs,0),maxLazyMs:Math.max(...editions.map(e=>e.lazyMs))}};
fs.writeFileSync(output,JSON.stringify(result));console.log(JSON.stringify({revision:result.revision,editions:editions.length,summary,performance:result.performance}));
