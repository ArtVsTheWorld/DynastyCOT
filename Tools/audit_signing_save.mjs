// Read-only signing relationship audit. All output must stay in ignored Build.
import fs from 'node:fs/promises';
import {pathToFileURL} from 'node:url';
import {gunzipSync} from 'node:zlib';
import {createHash} from 'node:crypto';
import {signingOwnership} from '../Modules/DynastyEnhancer/Tools/signing_ownership.mjs';
const [save,schemaPath,parserPath,out]=process.argv.slice(2);
if(!out)throw Error('Usage: audit_signing_save.mjs save-copy schema parser output');
const hash=b=>createHash('sha256').update(b).digest('hex'),before=hash(await fs.readFile(save));
const schemaBytes=await fs.readFile(schemaPath),schema=JSON.parse(gunzipSync(schemaBytes));
const {default:Franchise}=await import(pathToFileURL(parserPath));
const file=await Franchise.create(save,{saveOnChange:false,gameYearOverride:27,gameTypeOverride:'college',schemaOverride:{...schema.meta,path:schemaPath}});
const tables=new Map(file.tables.map(t=>[t.header.tableId,t])),loaded=new Set(),layouts=[];
async function load(t){
 if(loaded.has(t))return;
 if(!t.header.isArray){
  if(t.header.data1Type!==schema.meta.major||t.schema?.attributes.length!==t.header.numMembers)throw Error('Table/schema layout mismatch: '+t.name);
  layouts.push({name:t.name,id:t.header.tableId,tableSchema:t.header.data1Type,members:t.header.numMembers,schemaMembers:t.schema.attributes.length});
 }
 await t.readRecords();loaded.add(t);
}
const vals=r=>Object.fromEntries(Object.entries(r.fields).map(([k,v])=>[k,v.value]));
async function resolve(ref){
 if(ref==null||ref==='00000000000000000000000000000000')return null;
 if(typeof ref!=='string'||!/^[01]{32}$/.test(ref))throw Error('Malformed reference');
 const t=tables.get(parseInt(ref.slice(0,15),2)),index=parseInt(ref.slice(15),2);if(!t)throw Error('Missing referenced table');await load(t);
 const r=t.records[index];if(!r||r.isEmpty)throw Error('Dangling reference');return {key:t.header.tableId+':'+index,type:t.name,v:vals(r)};
}
async function array(ref){const r=await resolve(ref);if(!r)return [];if(!r.type.endsWith('[]'))throw Error('Not an array');const result=[];for(const value of Object.values(r.v))if(typeof value==='string'&&/^[01]{32}$/.test(value)){const x=await resolve(value);if(x)result.push(x);}return result;}
async function records(name){const out=[];for(const t of file.tables.filter(t=>t.name===name&&!t.header.isArray&&t.header.recordCapacity>1)){await load(t);for(const r of t.records)if(!r.isEmpty)out.push({key:t.header.tableId+':'+r.index,type:name,v:vals(r)});}return out;}
const teamRows=await records('Team'),teams=teamRows.map(r=>r.v).filter(t=>t.TeamIndex>=0&&t.TeamIndex<255),byIndex=new Map(teams.map(t=>[t.TeamIndex,t]));
const playerField=file.schemaList.getSchema('Recruit').attributes.find(a=>a.name==='Player'),committedField=file.schemaList.getSchema('Team').attributes.find(a=>a.name==='CommittedPlayers');
if(playerField?.type!=='Player'||committedField?.type!=='Player[]')throw Error('Unsupported signing relationship types');
const {owners,diagnostics}=await signingOwnership(teams,{resolve,array});
const recruits=await records('Recruit'),signed=recruits.filter(r=>r.v.RecruitStage==='Signed'),old=new Map(),candidate=new Map(),problems=[],details=[];
const leading=new Map();
for(const r of signed){const p=await resolve(r.v.Player),schools=await array(r.v.TopSchoolsList);if(p&&schools.length&&!schools.slice(1).some(s=>s.v.TeamInfluence>=schools[0].v.TeamInfluence))leading.set(r.key,{player:p.key,team:schools[0].v.TeamId});}
for(const team of teams){const board=await resolve(team.RecruitingBoard);if(!board)continue;for(const target of await array(board.v.Recruits)){const r=await resolve(target.v.Recruit),c=r&&leading.get(r.key);if(c?.team===team.TeamIndex)old.set(c.player,c.team);}}
for(const r of signed){const p=await resolve(r.v.Player),schools=p&&owners.get(p.key);if(!p||schools?.size!==1){problems.push({recruit:r.key,player:p?.key,owners:schools?[...schools]:[]});continue;}
 const team=[...schools][0];if(candidate.has(p.key))throw Error('Duplicate signed Recruit.Player');candidate.set(p.key,team);
 const row={player:p.key,name:[p.v.FirstName,p.v.LastName].filter(Boolean).join(' '),recruit:r.key,school:byIndex.get(team)?.DisplayName,team,previousTeam:old.get(p.key)??null,previousSchool:byIndex.get(old.get(p.key))?.DisplayName??null,interestLeader:leading.get(r.key)?.team??null,interestLeaderSchool:byIndex.get(leading.get(r.key)?.team)?.DisplayName??null};details.push(row);
}
const misses=details.filter(r=>r.previousTeam===null),changed=details.filter(r=>r.previousTeam!==null&&r.previousTeam!==r.team),nmsu=details.filter(r=>/new mexico (?:state|st\.?)/i.test(r.school)),goss=details.filter(r=>r.name==='Augustine Goss');
const stages=Object.fromEntries(['Signed','HardCommitted','SoftCommitted'].map(stage=>[stage,recruits.filter(r=>r.v.RecruitStage===stage).length]));
const seasonTable=file.tables.find(t=>t.name==='SeasonInfo');await load(seasonTable);const season=vals(seasonTable.records.find(r=>!r.isEmpty));
if(hash(await fs.readFile(save))!==before)throw Error('Save changed during read-only audit');
const report={saveSha256:before,schemaSha256:hash(schemaBytes),containerVersion:file.expectedSchemaVersion,schemaMeta:schema.meta,layouts,relationship:{recruitPlayer:playerField.type,committedPlayers:committedField.type},season,stages,signed:signed.length,oldDetected:old.size,candidateDetected:candidate.size,missedBefore:misses.length,changedDestinations:changed.length,problems,diagnostics,nmsu:nmsu.map(r=>({...r,missedBefore:r.previousTeam===null})),goss,misses,changed,details,sourceUnchanged:true,readOnly:true};
await fs.writeFile(out,JSON.stringify(report,null,2)+'\n');console.log(JSON.stringify({...report,season:undefined,details:undefined,misses:undefined,nmsu:report.nmsu.filter(r=>r.missedBefore),layouts:layouts.map(l=>l.name)},null,2));
