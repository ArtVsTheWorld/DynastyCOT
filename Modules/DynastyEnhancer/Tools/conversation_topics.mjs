export const bodyKey=text=>typeof text==='string'?text.normalize('NFKC').toLowerCase().replace(/[^\p{L}\p{N}@]+/gu,' ').trim():null;
export function canonicalTopic(e){
 if(!e)return null;const f=e.facts??{},player=e.playerId??f.playerName;
 if(player&&(f.recruitKind==='transfer'||e.id?.startsWith('portal-opening:')&&!e.id.includes('unsigned'))){const state=f.signed?'signed':f.newlyObserved?'commitment':f.flipped||f.decommitted?'changed':'open';return 'portal:'+player+':'+state+(state==='open'?'':':'+(f.school??f.destination??''));}
 return null;
}
export function deduplicateConversations(stories,events=[],publishedIds=new Set()){
 const eventMap=new Map(events.map(e=>[e.id,e])),kept=new Set(),seen=new Set(),predictionReplies=new Set(),matchupFlavor=new Map();
 const ordered=[...stories].sort((a,b)=>Number(publishedIds.has(b.id))-Number(publishedIds.has(a.id)));
 for(const p of ordered){const e=eventMap.get(p.eventId),topic=p.canonicalTopic??canonicalTopic(e),body=bodyKey(p.body),protectedPost=p.essentialNews||p.coverageExempt||p.budgetProtected||p.voice==='nflDraft',key=p.replyTo&&topic&&['fan','player','recruit'].includes(p.voice)?'reaction|'+p.handle+'|'+topic:body&&!protectedPost?'text|'+body:null;
  // One account need not repeat the same matchup reaction beneath each desk.
  // Keep its first real parent; never move a reply to another conversation.
  const prediction=p.replyTo&&/^buzz\.pick-reply/.test(p.templateFamily??'')?p.handle+'|'+p.thread:null;
  const thread=e?.thread??p.thread,game=e?.facts?.gameId??(thread?.startsWith('game:')?thread.slice(5):null),flavor=game&&!protectedPost&&!['player-stats','team-stats','analytics'].includes(p.category);
  const published=publishedIds.has(p.id);
  if(!published&&(key&&seen.has(key)||prediction&&predictionReplies.has(prediction)||flavor&&(matchupFlavor.get(game)??0)>=3))continue;
  if(key)seen.add(key);if(prediction)predictionReplies.add(prediction);if(flavor)matchupFlavor.set(game,(matchupFlavor.get(game)??0)+1);kept.add(p.id);
 }
 // Remove descendants when an equivalent parent was rejected; never reparent a reply.
 let changed=true;while(changed){changed=false;for(const p of stories)if(kept.has(p.id)&&p.replyTo&&!kept.has(p.replyTo)){kept.delete(p.id);changed=true;}}
 return stories.filter(p=>kept.has(p.id));
}
