const shorten=(text,max=48)=>{text=String(text??'').replace(/\s+/g,' ').trim();return text.length>max?text.slice(0,max-1).trimEnd()+'…':text;};
// These are summaries of captured stories/results, not invented trends or timestamps.
export function editionSidebar(feed){
 const stories=feed.stories??[],events=feed.events??[],lookup=new Map(events.map(e=>[e.id,e])),own=feed.userTeamIndex;
 const primary=stories.filter(p=>!p.replyTo&&!['fan','player','recruit'].includes(p.voice));
 const seen=new Set(),trends=[];
 for(const p of [...primary].sort((a,b)=>(b.priority??0)-(a.priority??0)||a.id.localeCompare(b.id))){
  const topic=p.eventId??p.id;if(seen.has(topic))continue;seen.add(topic);trends.push(shorten(p.body));if(trends.length===3)break;
 }
 const result=events.find(e=>e.category==='results'&&e.facts?.home&&e.facts?.away&&Number.isFinite(e.facts.homeScore)&&Number.isFinite(e.facts.awayScore));
 const game=result?.facts;
 const nation=game?[shorten(game.away,20)+' '+game.awayScore,shorten(game.home,20)+' '+game.homeScore]:primary.filter(p=>p.scopes?.includes('national')).slice(0,2).map(p=>shorten(p.body));
 const local=primary.filter(p=>p.teamIds?.includes(own)),localEvent=local.map(p=>lookup.get(p.eventId)).find(e=>e?.category==='results');
 const f=localEvent?.facts;
 const program=f&&Number.isFinite(f.homeScore)&&Number.isFinite(f.awayScore)?[shorten(f.away,20)+' '+f.awayScore,shorten(f.home,20)+' '+f.homeScore]:local.slice(0,2).map(p=>shorten(p.body));
 return [
  {title:'TRENDING',lines:trends.length?trends:['No major stories yet.']},
  {title:'AROUND THE NATION',lines:nation.length?nation:['No results available.']},
  {title:'YOUR PROGRAM',lines:program.length?program:['No new program news.']}
 ];
}
