import {observeOffseason} from './offseason_ledger.mjs';
import {offseasonLedgerFile,readOffseasonLedger} from './offseason_ledger_store.mjs';
import fs from 'node:fs/promises';
import path from 'node:path';
import {performance} from 'node:perf_hooks';
import {readRuntime} from './read_runtime.mjs';
import {updateRuntime,scopeKey} from './runtime_lifecycle.mjs';
import {readHistory,loadArchives,resumeEdition,atomicJson,writeHistory} from './runtime_archive.mjs';
import {awaitPublicationApproval} from './publication_gate.mjs';
import {CONTENT_REVISION} from './story_renderer.mjs';
const [input,output,gate]=process.argv.slice(2);
if(!input||!output)throw Error('Usage: node export_runtime.mjs runtime-raw.json output.json [native-approval-gate]');
const started=performance.now(),cpuStarted=process.cpuUsage(),timings={};let marked=started;
const mark=key=>{const now=performance.now();timings[key]=Math.round(now-marked);marked=now;};
const inputText=await fs.readFile(input,'utf8');mark('inputReadMs');
const raw=JSON.parse(inputText);mark('inputParseMs');
const s=await readRuntime(raw);mark('normalizeMs');
const directory=path.join(path.dirname(input),'RuntimeHistory',scopeKey(s)),file=path.join(directory,'history.json');
const ledgerFile=offseasonLedgerFile(path.dirname(input),s);
const [state,ledger]=await Promise.all([readHistory(file,{activeWeek:s.season.CurrentWeek}),readOffseasonLedger(ledgerFile)]);
const gap=s.games.some(g=>g.phase===s.season.CurrentWeekType&&g.year===s.season.CurrentYear&&g.week+1<s.season.CurrentWeek&&g.published!==false&&['HomeWon','AwayWon','Tie'].includes(g.status)&&!state?.editions?.some(e=>e.week===g.week+1));
const archives=await loadArchives(path.dirname(input),s,{crossSchool:!state?.editions?.length||gap,sharedSchools:state?.sharedSchools??[]});mark('historyReadMs');
// Exact normalized-content reuse only. Spring/offseason ledgers reconcile.
const same=state?.editions?.find(e=>e.scope===scopeKey(s)&&e.week===s.season.CurrentWeek);
// The old resume helper intentionally accepts old wording. This additional
// fast path must still run existing migration/enrichment for an older pack
// (including the GPI rename) before it may skip generation.
const resumed=!['PreSeason','OffSeason'].includes(s.season.CurrentWeekType)&&same?.feed?.contentRevision===CONTENT_REVISION?resumeEdition(s,state,archives):null;
let catalog,result=null,carry=null;
if(resumed){catalog=resumed;mark('reuseMs');}
else{
 carry=observeOffseason(s,ledger,{history:[...archives,...(state?.editions??[]).map(e=>e.feed)]});s.offseasonCarry=carry.facts;
 result=updateRuntime(s,state??undefined,{archives});catalog=result.catalog;mark('generationMs');
}
await atomicJson(output,catalog);mark('catalogWriteMs');
await awaitPublicationApproval(gate,output,{league:s.leagueId,team:s.userTeamIndex,season:s.season.CurrentSeasonYear,phase:s.season.CurrentWeekType,week:s.season.CurrentWeek,sha256:s.sha256});mark('approvalWaitMs');
if(result){await fs.mkdir(directory,{recursive:true});await atomicJson(ledgerFile,carry.ledger);await writeHistory(file,result.state);}
mark('historyWriteMs');
const cpu=process.cpuUsage(cpuStarted),usage=process.resourceUsage();
console.log(JSON.stringify({event:'runtime_feed_ready',league:s.leagueId,team:s.userTeamIndex,signingDetection:s.signingDetection,week:s.season.CurrentWeek,stories:catalog.stories.length,posts:catalog.posts.length,carryoverFacts:carry?.facts.length??0,newOffers:s.newOffers?.length??0,newCommitments:s.newCommitments?.length??0,sha256:s.sha256,reusedEdition:Boolean(resumed),inputBytes:Buffer.byteLength(inputText),cpuUserMs:cpu.user/1000,cpuSystemMs:cpu.system/1000,peakRssKiB:usage.maxRSS,timings:{...timings,startupAndImportsMs:Math.round(process.uptime()*1000-(performance.now()-started)),totalMs:Math.round(performance.now()-started)}}));
