// Matched archived publications; isolated from menu/FPS claims.
import fs from 'node:fs/promises';
import path from 'node:path';
import {pathToFileURL} from 'node:url';
import {performance} from 'node:perf_hooks';
import assert from 'node:assert/strict';
const [folder,output]=process.argv.slice(2),root=path.resolve('.');
const sources={before:path.join(root,'Build/RC016/Baseline'),after:root};
const modules={};for(const [mode,repo] of Object.entries(sources))modules[mode]=await import(pathToFileURL(path.join(repo,'Modules/DynastyEnhancer/Tools/feed_selection.mjs')));
const feeds=[];for(const name of await fs.readdir(folder))if(name.endsWith('.json'))feeds.push(JSON.parse(await fs.readFile(path.join(folder,name),'utf8')));
const active=feeds.find(f=>f.phase==='NationalChampionship'),history=feeds.filter(f=>f.season===active.season&&f.key!==active.key);
const report={mode:'Offline catalog assembly, identical archived inputs; parsing excluded, not live render/FPS',runs:[],editions:history.length+1};
for(const mode of ['before','after','after','before','before','after']){
 const began=performance.now(),catalog=modules[mode].buildCatalog(active,history);const ms=performance.now()-began;
 if(mode==='after')for(const edition of catalog.weeks){const original=feeds.find(f=>f.key===edition.key);assert.deepEqual(edition.views,original.views,'Published selection changed during catalog assembly');}
 report.runs.push({mode,ms,weeks:catalog.weeks.length});
}
await fs.writeFile(output,JSON.stringify(report,null,2));console.log(JSON.stringify(report));
