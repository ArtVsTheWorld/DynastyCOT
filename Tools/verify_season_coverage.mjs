// Explain fact-driven event changes rather than calling them lost coverage.
import fs from 'node:fs/promises';
import path from 'node:path';
import {pathToFileURL} from 'node:url';
import assert from 'node:assert/strict';
const root=path.resolve('.'),folder=path.join(root,'Build/RC016');
const read=async p=>JSON.parse(await fs.readFile(path.join(folder,p),'utf8'));
const before=await read('before-coverage.json'),after=await read('after-coverage.json'),manifest=await read('inputs.json');
const load=(repo,name)=>import(pathToFileURL(path.join(repo,'Modules/DynastyEnhancer/Tools',name)));
const oldReader=await load(path.join(folder,'Baseline'),'read_runtime.mjs'),newReader=await load(root,'read_runtime.mjs'),newEvents=await load(root,'story_events.mjs');
const changes=[];let checked=0;
for(let i=0;i<before.editions.length;i++){
 const b=before.editions[i],a=after.editions[i];assert.equal(b.inputSha256,a.inputSha256);
 const actual=new Set(a.allRootIds),missing=b.protectedRootIds.filter(id=>!actual.has(id));checked+=b.protectedRootIds.length;
 if(!missing.length)continue;
 const raw=JSON.parse(await fs.readFile(manifest.inputs[i].path,'utf8')),old=await oldReader.readRuntime(raw),s=await newReader.readRuntime(raw),eligible=new Set(newEvents.detectEvents(s).map(e=>e.id));
 for(const id of missing){const event=id.replace(/:main$/,''),teamId=Number(event.split(':').at(-1)),t=old.teams.find(t=>t.id===teamId),next=s.teams.find(t=>t.id===teamId);
  assert.match(event,/^late:bowl-(?:watch|position|no-room):/,'Unexpected missing protected news');
  assert.ok(!eligible.has(event),'Still-eligible protected event lost its story');
  assert.ok(t.wins!==next.wins||t.losses!==next.losses||t.ties!==next.ties,'Missing news has no factual record change');
  changes.push({edition:a.key,id,school:t.name,before:[t.wins,t.losses,t.ties],after:[next.wins,next.losses,next.ties],reason:'Native totals change bowl eligibility; obsolete watch/no-room angle is no longer eligible'});
 }
}
const report={captures:before.editions.length,checkedProtectedRoots:checked,lostStillEligibleProtectedRoots:0,factDrivenChanges:changes};
await fs.writeFile(path.join(folder,'coverage-verification.json'),JSON.stringify(report,null,2));console.log(JSON.stringify({captures:report.captures,checked,obsoleteAngles:changes.length,lostStillEligibleProtectedRoots:0}));
