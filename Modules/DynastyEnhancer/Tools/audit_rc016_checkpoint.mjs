// Private read-only audit: output belongs under ignored Build/RC016 only.
import fs from 'node:fs/promises';
import {normalizeDynasty} from './normalize_dynasty.mjs';
import {pathToFileURL} from 'node:url';
import {gunzipSync} from 'node:zlib';
import {createHash} from 'node:crypto';
const [save,out]=process.argv.slice(2);
if(!save||!out)throw Error('Usage: node audit_rc016_checkpoint.mjs checkpoint-copy output.json');
const {default:Franchise}=await import(pathToFileURL('C:/Apps/CFB 27/MMC/MMC_Editor_v1.1.0.6/FtcCompatibilityPoc/node_modules/madden-franchise/dist/index.mjs'));
const schemaPath='C:/Users/Art/AppData/Local/Programs/MyDynasty/resources/schemas/C27_486_6.gz',schema=JSON.parse(gunzipSync(await fs.readFile(schemaPath)));
const hash=async()=>createHash('sha256').update(await fs.readFile(save)).digest('hex'),before=await hash();
const f=await Franchise.create(save,{autoParse:true,saveOnChange:false,gameYearOverride:27,gameTypeOverride:'college',schemaOverride:{major:schema.meta.major,minor:schema.meta.minor,gameYear:27,path:schemaPath}});
const vals=r=>Object.fromEntries(Object.entries(r.fields).map(([k,v])=>[k,v.value])),tables=new Map(f.tables.map(t=>[t.header.tableId,t])),loaded=new Set(),omissions=[];
async function load(t){if(!loaded.has(t)){await t.readRecords();loaded.add(t);}return t;}
async function records(name){const result=[];for(const t of f.tables.filter(t=>t.name===name&&!t.header.isArray)){await load(t);for(const r of t.records)if(!r.isEmpty)result.push({key:`${t.header.tableId}:${r.index}`,v:vals(r),capacity:t.header.recordCapacity});}return result;}
async function resolve(ref){if(ref==null||ref==='00000000000000000000000000000000')return null;if(typeof ref!=='string'||!/^[01]{32}$/.test(ref))throw Error('Malformed reference');const t=tables.get(parseInt(ref.slice(0,15),2)),row=parseInt(ref.slice(15),2);if(!t){omissions.push({kind:'missing-table',ref});return null;}await load(t);const r=t.records[row];if(!r||r.isEmpty){omissions.push({kind:'empty-row',ref});return null;}return {key:`${t.header.tableId}:${row}`,type:t.name,v:vals(r)};}
async function arraySlots(ref){const a=await resolve(ref);if(!a)return [];if(!a.type.endsWith('[]'))throw Error('Expected array');const result=[];for(const v of Object.values(a.v))if(typeof v==='string'&&/^[01]{32}$/.test(v))result.push(await resolve(v));return result;}
const s=await normalizeDynasty({records,resolve,array:async r=>(await arraySlots(r)).filter(Boolean),arraySlots},{source:save,sha256:before});
s.audit={readOnly:true,optionalReferenceOmissions:omissions,traits:Object.entries(Object.groupBy((await records('Player')).filter(r=>r.capacity>1&&r.v.TeamIndex===255),r=>r.v.TraitDevelopment)).map(([trait,rows])=>({trait,count:rows.length})),seasonInfo:(await records('SeasonInfo'))[0]?.v};
if(await hash()!==before)throw Error('Checkpoint changed');
await fs.writeFile(out,JSON.stringify(s));
console.log(JSON.stringify({save,league:s.leagueId,team:s.userTeamIndex,season:s.season,players:s.players.length,games:s.games.length,draft:s.draftResults?.length,dates:s.games.filter(g=>g.week===s.season.CurrentWeek&&g.phase===s.season.CurrentWeekType).slice(0,4).map(g=>[g.month,g.day,g.weekday]),sourceUnchanged:s.sourceUnchanged}));
