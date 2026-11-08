// Frozen event-fact replay. Raw historical inputs absent from the source corpus
// remain absent. Outputs/captures are always private, never installed archives.
import fs from 'node:fs';
import path from 'node:path';
import {pathToFileURL} from 'node:url';
import {createHash} from 'node:crypto';
import {performance} from 'node:perf_hooks';
import {nativeViewPosts} from '../../Tools/feed_visibility.mjs';
import {assertPrimaryAwardConservation,requiredCoverageEventIds} from '../SocialVariety/conservation.mjs';
const [engine,input,output]=process.argv.slice(2);if(!output)throw Error('Usage: replay.mjs engine corpus.json output.json');
const load=name=>import(pathToFileURL(path.resolve(engine,'Tools',name+'.mjs')));
const [{renderStories},{publishWeek},{applyWeeklyCoverage},{deduplicateConversations},{freshEditorialEvents},{contentPack}]=await Promise.all(['story_renderer','feed_selection','weekly_coverage','conversation_topics','editorial_policy','story_templates'].map(load));
const corpus=JSON.parse(fs.readFileSync(input,'utf8')),history=new Map(),editions=[],eligibility={},counts=new Map(),views=['all','team','conference','national'];
const shape=p=>p.variety?.structure??p.body.toLowerCase().replace(/\d+/g,'#');
const phraseCache=new Map();const phrases=form=>{if(phraseCache.has(form))return phraseCache.get(form);const words=form.split(' '),set=new Set(words.slice(0,-3).map((_,i)=>words.slice(i,i+4).join(' ')));phraseCache.set(form,set);return set;};
function metrics(posts,recent){
 const seenForms=new Set(),phraseForms=new Map();
 const remember=form=>{if(seenForms.has(form))return;seenForms.add(form);for(const phrase of phrases(form)){const set=phraseForms.get(phrase)??new Set();set.add(form);phraseForms.set(phrase,set);}};
 const similar=(a,b)=>{if(a===b)return true;const x=phrases(a),y=phrases(b);if(!x.size||!y.size)return false;let n=0;for(const p of x)if(y.has(p))n++;return n/Math.max(x.size,y.size)>=.7;};
 const used=form=>{if(seenForms.has(form))return true;const candidates=new Set();for(const phrase of phrases(form))for(const other of phraseForms.get(phrase)??[])candidates.add(other);for(const other of candidates)if(similar(form,other))return true;return false;};
 recent.forEach(remember);let nearDuplicates=0,adjacent=0;for(let i=0;i<posts.length;i++){const form=shape(posts[i]);if(used(form))nearDuplicates++;if(i&&similar(form,shape(posts[i-1])))adjacent++;remember(form);}
 return {cards:posts.length,nearDuplicates,adjacent};
}
for(const row of corpus.editions){
 const s=row.snapshot,key=s.leagueId+'/'+s.userTeamIndex,prior=history.get(key)??[],publication={leagueId:s.leagueId,userTeamIndex:s.userTeamIndex,season:s.season.CurrentSeasonYear,phase:row.phase,week:row.week};
 const events=freshEditorialEvents(row.events,publication,prior),measuredFamilies=new Set();let auditedCalls=0;
 const auditAt=performance.now();const rendered=renderStories(s,events,{authors:row.authors,history:prior,onTemplates:({family,values,style,candidates})=>{
  auditedCalls++;const e=eligibility[family]??={calls:0,min:Infinity,max:0,total:0,unusedTotal:0,exhaustedCalls:0,shapes:new Set()};e.calls++;e.min=Math.min(e.min,candidates.length);e.max=Math.max(e.max,candidates.length);e.total+=candidates.length;e.unusedTotal+=candidates.filter(c=>!c.recentlyUsed).length;if(candidates.length&&candidates.every(c=>c.recentlyUsed))e.exhaustedCalls++;candidates.forEach(c=>e.shapes.add(c.structure));measuredFamilies.add(family);
 }});const auditedMs=performance.now()-auditAt;
 const lazyAt=performance.now(),lazy=renderStories(s,events,{authors:row.authors,history:prior}),lazyMs=performance.now()-lazyAt;
 if(JSON.stringify(lazy.stories)!==JSON.stringify(rendered.stories))throw Error('Lazy/exhaustive selection mismatch '+key+'/'+row.phase+'/'+row.week);
 const feed=publishWeek({format:1,key:s.leagueId+'/'+publication.season+'/'+row.phase+'/'+row.week,...publication,events,stories:applyWeeklyCoverage(deduplicateConversations(rendered.stories,events),s)});
 const map=new Map(feed.stories.map(p=>[p.id,p])),eventMap=new Map(events.map(e=>[e.id,e]));
 for(const p of feed.stories){if([...p.body].length>280||Buffer.byteLength(p.body)>512||/\{[A-Za-z][A-Za-z0-9]*\}/.test(p.body))throw Error('Invalid rendered body '+p.id);if(p.replyTo&&!map.has(p.replyTo))throw Error('Orphan reply '+p.id);}
 const essential=pool=>requiredCoverageEventIds(pool.filter(p=>{const f=eventMap.get(p.eventId)?.facts??{};return !p.replyTo&&p.voice!=='fan'&&(['results','upsets','awards','records','injuries'].includes(p.category)||p.category==='draft'&&p.voice==='nflDraft'||['recruiting','signings'].includes(p.category)&&(f.signed||f.flipped||f.decommitted||f.newlyObserved&&['HardCommitted','Signed'].includes(f.commitment)));}).map(p=>p.eventId),events);
 const primaryAwardConservation=assertPrimaryAwardConservation(events,essential(rendered.stories),essential(feed.stories)),selected=Object.fromEntries(views.map(v=>[v,nativeViewPosts(feed,v)])),viewMetrics={};
 for(const view of views)viewMetrics[view]=metrics(selected[view],prior.slice(-4).flatMap(f=>nativeViewPosts(f,view).map(shape)));
 for(const p of selected.all)counts.set(p.templateFamily,(counts.get(p.templateFamily)??0)+1);
 editions.push({key:feed.key,phase:row.phase,week:row.week,generated:rendered.stories.length,kept:feed.stories.length,roots:feed.stories.filter(p=>!p.replyTo).length,lazyMs,auditedMs,auditedCalls,metrics:viewMetrics,essentialGenerated:essential(rendered.stories),essentialPublished:essential(feed.stories),primaryAwardConservation,views:Object.fromEntries(views.map(v=>[v,selected[v].map(p=>({id:p.id,eventId:p.eventId,family:p.templateFamily,templateId:p.templateId,voice:p.voice,author:p.author,handle:p.handle,body:p.body,replyTo:p.replyTo,angle:p.variety?.angle,structure:shape(p)}))]))});
 prior.push(feed);history.set(key,prior);
 console.log(JSON.stringify({phase:row.phase,week:row.week,stories:feed.stories.length,displayed:selected.all.length,lazyMs:Math.round(lazyMs)}));
}
const summary={};for(const v of views){summary[v]={cards:0,nearDuplicates:0,adjacent:0};for(const e of editions)for(const key of Object.keys(summary[v]))summary[v][key]+=e.metrics[v][key];summary[v].rate=summary[v].nearDuplicates/Math.max(1,summary[v].cards);}
for(const e of Object.values(eligibility)){e.usableStructures=e.shapes.size;delete e.shapes;e.average=e.total/e.calls;}
const outputReport={format:1,revision:contentPack.revision,inputSha256:createHash('sha256').update(fs.readFileSync(input)).digest('hex'),method:'Retained event-fact replay; native current default scopes/250-card National selection. Near duplicate: normalized template form equal or >=70% intersection/larger-set word four-grams against four prior editions and earlier same-view cards. Names/numbers cannot create diversity. Not live menu/FPS timing.',summary,topFamilies:[...counts].sort((a,b)=>b[1]-a[1]),eligibility,editions,performance:{lazyTotalMs:editions.reduce((n,e)=>n+e.lazyMs,0),maxLazyMs:Math.max(...editions.map(e=>e.lazyMs))}};
fs.writeFileSync(output,JSON.stringify(outputReport));console.log(JSON.stringify({editions:editions.length,summary,performance:outputReport.performance}));
