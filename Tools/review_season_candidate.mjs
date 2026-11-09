// Private, read-only replay of frozen runtime inputs. Never imports live paths.
import fs from 'node:fs/promises';
import path from 'node:path';
import {pathToFileURL} from 'node:url';
import {performance} from 'node:perf_hooks';
import assert from 'node:assert/strict';
const [repo,manifestFile,output]=process.argv.slice(2);
if(!output)throw Error('Usage: review_season_candidate.mjs source-root input-manifest output');
const tool=name=>pathToFileURL(path.resolve(repo,'Modules/DynastyEnhancer/Tools',name)).href;
const {readRuntime}=await import(tool('read_runtime.mjs'));
const {buildFeed}=await import(tool('feed_engine.mjs'));
const {nativeViewIds}=await import(tool('feed_visibility.mjs'));
const manifest=JSON.parse(await fs.readFile(manifestFile,'utf8'));
const history=[],editions=[],eligibility={};let previous=null,authors={};
for(const input of manifest.inputs){
 const raw=JSON.parse(await fs.readFile(input.path,'utf8')),s=await readRuntime(raw);
 const began=performance.now();let nextAuthors;
 const feed=buildFeed(s,previous,{history,authors,onAuthors:x=>nextAuthors=x});
 const generationMs=performance.now()-began;
 const visible={};for(const view of ['all','team','conference','national']){
  const ids=new Set(nativeViewIds(feed,view));visible[view]=feed.stories.filter(p=>ids.has(p.id));
 }
 // Diagnostics are deliberately excluded from generation timing.
 if(process.env.COT_FAST_REPLAY!=='1'){
 const audited=buildFeed(s,previous,{history,authors,onTemplates:r=>{
  const v=eligibility[r.family]??={calls:0,total:0,min:Infinity,max:0};
  v.calls++;v.total+=r.candidates.length;v.min=Math.min(v.min,r.candidates.length);v.max=Math.max(v.max,r.candidates.length);
 }});assert.deepEqual(audited,feed,'Diagnostic selection changed output');
 }
 const refresh=buildFeed(s,previous,{history,authors:nextAuthors,sameWeek:feed});
 assert.deepEqual(refresh.stories,feed.stories,'Same-week refresh changed publication');
 const ids=new Set(feed.stories.map(p=>p.id));
 for(const p of feed.stories){assert.ok(!p.replyTo||ids.has(p.replyTo));assert.ok([...p.body].length<=280&&Buffer.byteLength(p.body)<=512);assert.ok(!/[\uFFFD\x00-\x08\x0b\x0c\x0e-\x1f]/u.test(p.body));}
 const rows=Object.fromEntries(Object.entries(visible).map(([k,v])=>[k,v.map(p=>({id:p.id,body:p.body,author:p.author,handle:p.handle,voice:p.voice,family:p.templateFamily,templateId:p.templateId,variety:p.variety,replyTo:p.replyTo,teamIds:p.teamIds,category:p.category,draftPick:p.draftPick}))]));
 editions.push({key:feed.key,inputSha256:input.sha256,generationMs,stories:feed.stories.length,protectedRootIds:feed.stories.filter(p=>!p.replyTo&&(p.essentialNews||p.coverageExempt||p.budgetProtected)).map(p=>p.id),allRootIds:feed.stories.filter(p=>!p.replyTo).map(p=>p.id),pff:feed.stories.filter(p=>p.handle==='@PFF_College').length,calendar:feed.stories.filter(p=>p.category==='holiday').map(p=>p.body),calendarEvents:feed.events.filter(e=>e.category==='holiday').map(e=>e.facts),views:rows});
 console.log(JSON.stringify({key:feed.key,stories:feed.stories.length,pff:editions.at(-1).pff,calendar:editions.at(-1).calendar.length,ms:Math.round(generationMs)}));
 history.push(feed);authors=nextAuthors;previous=s;
}
for(const x of Object.values(eligibility))x.average=x.total/x.calls;
await fs.writeFile(output,JSON.stringify({baseline:manifest.baseline,mode:'Fresh counterfactual publications from identical frozen captures; source history untouched',editions,eligibility},null,2));
