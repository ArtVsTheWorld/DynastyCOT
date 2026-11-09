// Final fact/role/length-guard candidate counts for twelve frozen editions.
import fs from 'node:fs/promises';
import path from 'node:path';
import {readRuntime} from '../Modules/DynastyEnhancer/Tools/read_runtime.mjs';
import {buildFeed} from '../Modules/DynastyEnhancer/Tools/feed_engine.mjs';
const folder=path.resolve('Build/RC016'),manifest=JSON.parse(await fs.readFile(path.join(folder,'inputs.json'),'utf8'));
const names=['2026-PreSeason-0','2026-RegularSeason-1','2026-RegularSeason-4','2026-RegularSeason-9','2026-RegularSeason-13','2026-RegularSeason-15','2026-BowlSeason1-17','2026-NationalChampionship-20','2026-OffSeason-0','2026-OffSeason-2','2026-OffSeason-7','2027-PreSeason-0'];
const eligibility={},editions=[];
for(const name of names){const input=manifest.inputs.find(x=>path.basename(x.path)===name+'.json');const s=await readRuntime(JSON.parse(await fs.readFile(input.path,'utf8')));
 buildFeed(s,null,{onTemplates:r=>{const x=eligibility[r.family]??={calls:0,total:0,min:Infinity,max:0};x.calls++;x.total+=r.candidates.length;x.min=Math.min(x.min,r.candidates.length);x.max=Math.max(x.max,r.candidates.length);}});
 editions.push({key:name,inputSha256:input.sha256});console.log(name);
}
for(const x of Object.values(eligibility))x.average=x.total/x.calls;
await fs.writeFile(path.join(folder,'final-eligibility.json'),JSON.stringify({mode:'Twelve final-candidate fixtures; render attempts including variety passes; no published history modified',editions,eligibility},null,2));
