// Audit-only classification. It does not suppress, edit or reprioritize posts.
const reactionVoices=new Set(['player','recruit','fan']);
export function primaryAwardEvents(events){return events.filter(e=>e.category==='awards'&&e.facts?.family!=='postseason.award-player'&&!reactionVoices.has(e.facts?.voice));}
export function optionalAwardAcknowledgment(event,events){
 if(event?.category!=='awards'||event.facts?.family!=='postseason.award-player'||event.facts.voice!=='player'||!event.playerId||!event.facts.award)return false;
 const f=event.facts;
 return primaryAwardEvents(events).some(primary=>{
  const p=primary.facts??{};
  return primary.playerId===event.playerId&&p.award===f.award&&(p.count??1)===(f.count??1)&&p.school===f.school&&p.playerName===f.playerName&&
   primary.teamIds?.[0]===event.teamIds?.[0]&&(primary.publication?.season??null)===(event.publication?.season??null);
 });
}
export function requiredCoverageEventIds(ids,events){
 const byId=new Map(events.map(e=>[e.id,e]));
 return [...new Set(ids)].filter(id=>!optionalAwardAcknowledgment(byId.get(id),events)).sort();
}
export function assertPrimaryAwardConservation(events,generatedIds,publishedIds){
 const expected=[...new Set(primaryAwardEvents(events).map(e=>e.id))].sort(),generated=new Set(generatedIds),published=new Set(publishedIds);
 const missingGenerated=expected.filter(id=>!generated.has(id)),missingPublished=expected.filter(id=>!published.has(id));
 if(missingGenerated.length||missingPublished.length)throw Error('Primary award coverage lost: '+JSON.stringify({missingGenerated,missingPublished}));
 return {expected:expected.length,generated:expected.length,published:expected.length,missingGenerated,missingPublished};
}
