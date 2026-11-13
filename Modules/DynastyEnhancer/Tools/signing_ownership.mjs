// Native ownership for confirmed signings only. Interest order is not ownership.
export async function signingOwnership(rawTeams,{resolve,array}){
 const owners=new Map(),diagnostics={unavailableTeamLists:0,repeatedMemberships:0};
 for(const t of rawTeams){
  if(!Number.isInteger(t.TeamIndex)||t.TeamIndex<0||t.TeamIndex>=255)continue;
  if(t.CommittedPlayers==null){++diagnostics.unavailableTeamLists;continue;}
  const list=await resolve(t.CommittedPlayers);if(!list){++diagnostics.unavailableTeamLists;continue;}
  if(list.type!=='Player[]')throw Error('Unsupported Team.CommittedPlayers reference type: '+list.type);
  for(const player of await array(t.CommittedPlayers)){
   if(player.type!=='Player')throw Error('CommittedPlayers member is not a Player reference');
   const schools=owners.get(player.key)??new Set();if(schools.has(t.TeamIndex))++diagnostics.repeatedMemberships;
   schools.add(t.TeamIndex);owners.set(player.key,schools);
  }
 }return {owners,diagnostics};
}
// Existing signing publications retain their original facts and words. Correct
// ownership applies to reports not yet published, not retroactive replacements.
export function preservePublishedSignings(events,sameWeek){
 if(!sameWeek)return events;
 const published=new Set((sameWeek.stories??[]).map(p=>p.eventId));
 const isSigning=e=>e.category==='signings'||e.facts?.signed===true;
 const player=e=>e.playerId??e.options?.player;
 const prior=(sameWeek.events??[]).filter(e=>isSigning(e)&&published.has(e.id));
 const players=new Set(prior.map(player).filter(Boolean));
 return [...events.filter(e=>!isSigning(e)||!players.has(player(e))),...prior];
}
