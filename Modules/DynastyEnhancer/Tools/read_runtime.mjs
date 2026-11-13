import {createHash} from 'node:crypto';
import {normalizeDynasty} from './normalize_dynasty.mjs';
import {sameLeague} from './league_identity.mjs';
import {runtimeRecords} from './runtime_wire.mjs';

export async function readRuntime(raw){
  if(![1,2].includes(raw.format)||raw.sourceKind!=='runtime')throw Error('Invalid runtime capture');
  const rows=runtimeRecords(raw),index=new Map(),all=new Map(rows.map(row=>[row.key,row]));
  if(all.size!==rows.length)throw Error('Duplicate runtime record');
  for(const row of all.values()){if(!row||row.key==null||!row.v)throw Error('Invalid live record');const list=index.get(row.type)??[];list.push(row);index.set(row.type,list);}
  const records=async name=>index.get(name)??[];
  const resolve=async ref=>{if(ref==null)return null;const row=all.get(ref);if(!row)throw Error('Unresolved live record');return row;};
  const arrays=new Map(); // Invocation-local immutable derivation; discarded on every capture/context.
  const arraySlots=async ref=>{if(ref==null)return [];if(arrays.has(ref))return arrays.get(ref);const task=(async()=>{const row=await resolve(ref);if(!row)return [];if(!row.type.endsWith('[]'))throw Error('Expected runtime array');return Object.freeze(await Promise.all(Object.entries(row.v).sort(([a],[b])=>Number(a.slice(6))-Number(b.slice(6))).map(([,v])=>v).map(resolve)));})();arrays.set(ref,task);return task;};
  const array=async ref=>(await arraySlots(ref)).filter(Boolean);
  const flow=(await records('FranchiseServer_CareerFlow'))[0]?.v;
  if(!flow?.IsStageActive||flow.IsStageAdvancing)throw Error('Dynasty stage is inactive or advancing');
  const snapshot=await normalizeDynasty({records,resolve,array,arraySlots},{source:'runtime',sha256:'0'.repeat(64),sourceKind:'runtime'});
  const analytics=raw.playerAnalytics;
  if(analytics?.format===1&&sameLeague(analytics.leagueId,snapshot.leagueId)&&analytics.year===snapshot.season.CurrentYear&&analytics.season===snapshot.season.CurrentSeasonYear&&typeof analytics.modelVersion==='string'&&Array.isArray(analytics.weekly)&&Array.isArray(analytics.seasonRows))snapshot.playerAnalytics=analytics;
  snapshot.source=`runtime:${snapshot.leagueId}:${snapshot.userTeamIndex}`;
  // Content hash excludes pointers, schema internals, and unconsumed game fields.
  snapshot.sha256=createHash('sha256').update(JSON.stringify({...snapshot,sha256:undefined})).digest('hex');
  return snapshot;
}
