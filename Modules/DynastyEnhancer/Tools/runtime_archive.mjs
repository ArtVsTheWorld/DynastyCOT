import path from 'node:path';
import fs from 'node:fs/promises';
import {sharedEdition} from './shared_editions.mjs';
import {PUBLICATION_PHASES,phaseIndex} from './publication_timeline.mjs';
import {scopeKey} from './runtime_lifecycle.mjs';
import {buildCatalog} from './feed_selection.mjs';
import {CONTENT_REVISION} from './story_renderer.mjs';
import {refreshAccountPresentation} from './account_presentation.mjs';
import {EDITORIAL_POLICY_VERSION} from './editorial_policy.mjs';
import {currentPublicationPolicy} from './publication_policy.mjs';
import {readEditionHistory,writeEditionHistory,atomicJsonFile} from './history_store.mjs';
export const readHistory=readEditionHistory,writeHistory=writeEditionHistory;
export async function loadArchives(folder,s,{crossSchool=false,sharedSchools=[]}={}){
 const seasonFolder=path.join(folder,'RuntimeHistory',String(s.leagueId),String(s.userTeamIndex),String(s.season.CurrentSeasonYear)),archives=[];
 for(const phase of PUBLICATION_PHASES){if(phaseIndex(phase)>=phaseIndex(s.season.CurrentWeekType))continue;
  const state=await readHistory(path.join(seasonFolder,phase,'history.json'),{archiveOnly:true});if(state)archives.push(...state.editions.map(e=>({...e.feed,milestoneObservation:e.snapshot?.milestoneTotals,seasonEndPublications:e.snapshot?.seasonEndPublications,postseasonPublications:e.snapshot?.postseasonPublications})));
 }
 if(crossSchool||sharedSchools.length){
  const leagueFolder=path.join(folder,'RuntimeHistory',String(s.leagueId));
  let dirs=[];if(crossSchool){try{dirs=await fs.readdir(leagueFolder,{withFileTypes:true});}catch(e){if(e.code!=='ENOENT')throw e;}}else dirs=sharedSchools.map(id=>({name:String(id),isDirectory:()=>true}));
  // Bounded by native team IDs, current season only, requested only when a
  // school's current-phase archive is absent. No permanent cache or migration.
  for(const dir of dirs.filter(d=>d.isDirectory()&&/^\d{1,3}$/.test(d.name)&&Number(d.name)<255&&Number(d.name)!==s.userTeamIndex).sort((a,b)=>Number(a.name)-Number(b.name))){
   for(const phase of PUBLICATION_PHASES){if(phaseIndex(phase)>phaseIndex(s.season.CurrentWeekType))continue;
    const state=await readHistory(path.join(leagueFolder,dir.name,String(s.season.CurrentSeasonYear),phase,'history.json'),{archiveOnly:true});
    for(const e of state?.editions??[]){const recovered=sharedEdition(e.feed,s);if(recovered&&!archives.some(f=>f.key===recovered.key))archives.push(recovered);}
   }
  }
 }
 return archives;
}
export function resumeEdition(s,state,archives=[]){
 if(!state)return null;const scope=scopeKey(s),week=s.season.CurrentWeek;
 const entries=state.editions.filter(e=>e.scope===scope),same=entries.find(e=>e.week===week);
 // Rewinds follow normal reconciliation so future observations are pruned.
 if(!same||entries.some(e=>e.week>week)||same.snapshot?.sha256!==s.sha256)return null;
 const f=same.feed;if(f?.editorialPolicyVersion!==EDITORIAL_POLICY_VERSION)return null;if(f?.sourceKind!=='runtime'||f.key!==`${s.leagueId}/${s.season.CurrentSeasonYear}/${s.season.CurrentWeekType}/${week}`||f.leagueId!==s.leagueId||f.userTeamIndex!==s.userTeamIndex||f.season!==s.season.CurrentSeasonYear||f.phase!==s.season.CurrentWeekType||f.week!==week||f.sha256!==s.sha256||!currentPublicationPolicy(f))return null;
 return buildCatalog(refreshAccountPresentation(f,s),[...archives,...entries.map(e=>e.feed)].map(old=>refreshAccountPresentation(old,s)));
}
export const atomicJson=atomicJsonFile;
