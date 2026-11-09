// Offline matched generation/catalog benchmark. No native menu/FPS inference.
import fs from 'node:fs/promises';
import path from 'node:path';
import {pathToFileURL} from 'node:url';
import {performance} from 'node:perf_hooks';
import assert from 'node:assert/strict';
const root=path.resolve('.'),folder=path.join(root,'Build/RC016'),modules={};
for(const mode of ['before','after']){
 const repo=mode==='before'?path.join(folder,'Baseline'):root;
 const load=name=>import(pathToFileURL(path.join(repo,'Modules/DynastyEnhancer/Tools',name)));
 modules[mode]={...await load('read_runtime.mjs'),...await load('feed_engine.mjs'),...await load('feed_selection.mjs'),...await load('publication_timeline.mjs')};
}
const manifest=JSON.parse(await fs.readFile(path.join(folder,'inputs.json'),'utf8'));
const feeds=[];for(const file of await fs.readdir(path.join(folder,'Feeds')))feeds.push(JSON.parse(await fs.readFile(path.join(folder,'Feeds',file),'utf8')));
const report={mode:'Identical frozen inputs and archived history; JSON reads/normalization excluded; source generation + catalog only, not live FPS',fixtures:[]};
for(const name of ['2026-RegularSeason-12','2026-NationalChampionship-20','2026-OffSeason-2']){
 const input=manifest.inputs.find(x=>path.basename(x.path)===name+'.json'),raw=JSON.parse(await fs.readFile(input.path,'utf8')),snapshots={};
 for(const mode of ['before','after'])snapshots[mode]=await modules[mode].readRuntime(raw);
 const s=snapshots.after,pub={season:s.season.CurrentSeasonYear,phase:s.season.CurrentWeekType,week:s.season.CurrentWeek};
 const history=feeds.filter(f=>f.season===pub.season&&modules.after.comparePublication(f,pub)<0),reference={},runs=[];
 for(const mode of ['before','after','after','before','before','after']){
  const began=performance.now(),feed=modules[mode].buildFeed(snapshots[mode],null,{history}),generated=performance.now(),catalog=modules[mode].buildCatalog(feed,history),ended=performance.now();
  if(reference[mode])assert.deepEqual(feed,reference[mode],'Benchmark refresh changed generated feed');else reference[mode]=feed;
  runs.push({mode,generationMs:generated-began,catalogMs:ended-generated,totalMs:ended-began,stories:feed.stories.length,weeks:catalog.weeks.length});
 }
 report.fixtures.push({name,inputSha256:input.sha256,runs});console.log(JSON.stringify(report.fixtures.at(-1)));
}
await fs.writeFile(path.join(folder,'worker-performance.json'),JSON.stringify(report,null,2));
