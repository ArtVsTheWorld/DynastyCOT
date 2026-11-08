// Mirrors FeedData.h's native selection contract. No generation budgets or
// stored posts are changed. Native parity is checked by NativeFeedViewChecks.
export const NATIONAL_CARD_LIMIT=250;
export function matchesNativeType(p,type=0){
 switch(type){
 case 0:return true;
 case 1:return ['recruiting','offers','signings'].includes(p.category)||p.voice==='recruit';
 case 2:return ['results','team-stats','player-stats','upsets','streaks','matchup-recap','season-end'].includes(p.category);
 case 3:return ['schedule','watch','lines','preseason','matchup-preview','offseason'].includes(p.category);
 case 4:return ['polls','heisman','cfp','awards','records','history','championship','bowls'].includes(p.category);
 case 5:return p.voice==='analyst'||p.voice==='model';
 case 6:return p.voice==='beat';
 case 7:return ['official','cfp','nflDraft'].includes(p.voice);
 case 8:return p.voice==='player';
 case 9:return p.category==='injuries';
 case 10:return p.voice==='fan';
 case 11:return !!p.transaction||['coaching','nil','transactions','transfer','signings'].includes(p.category)&&['analyst','beat'].includes(p.voice);
 default:return false;
 }
}
export function nativeCandidates(stories,ids,type=0){
 const map=new Map(stories.map(p=>[p.id,p])),members=new Set(ids),included=new Set(),roots=[],children=new Map();
 for(const id of ids){const p=map.get(id);if(!p)throw Error('Unknown native feed reference');if(!p.replyTo)roots.push(id);else if(members.has(p.replyTo)){const list=children.get(p.replyTo)??[];list.push(id);children.set(p.replyTo,list);}}
 for(const id of ids)if(matchesNativeType(map.get(id),type)){let key=id;const seen=new Set();while(members.has(key)&&!seen.has(key)){seen.add(key);included.add(key);key=map.get(key).replyTo;}}
 if(type===11)roots.sort((a,b)=>{const x=map.get(a).draftPick??-1,y=map.get(b).draftPick??-1;return x>0&&y>0?x-y:x>0&&y<=0?-1:y>0&&x<=0?1:0;});
 const result=[],visited=new Set();const append=(id,whole)=>{if(visited.has(id))return;visited.add(id);whole=whole||matchesNativeType(map.get(id),type);if(!whole&&!included.has(id))return;result.push(id);for(const child of children.get(id)??[])append(child,whole);};
 for(const id of roots)append(id,false);return result;
}
export function selectNational(stories,candidates,type=0){
 if(candidates.length<=NATIONAL_CARD_LIMIT)return candidates;
 const map=new Map(stories.map(p=>[p.id,p])),members=new Set(candidates),threads=[],positions=new Map();
 for(const id of candidates){let root=id;const seen=new Set();while(members.has(map.get(root).replyTo)&&!seen.has(root)){seen.add(root);root=map.get(root).replyTo;}
  if(!positions.has(root)){positions.set(root,threads.length);threads.push({root,ids:[],minimum:[],order:threads.length});}threads[positions.get(root)].ids.push(id);
 }
 for(const t of threads){const p=map.get(t.root);t.draft=p.voice==='nflDraft'&&p.draftPick>0&&matchesNativeType(p,type);t.important=!!(p.essentialNews||p.coverageExempt||p.budgetProtected);
  t.score=(t.important?10000:0)+(['results','upsets','polls','cfp','awards','championship','draft'].includes(p.category)?1000:0)+(p.priority??0);
  let id=t.ids.find(id=>matchesNativeType(map.get(id),type));while(members.has(id)){t.minimum.push(id);id=map.get(id).replyTo;}t.minimum.reverse();
 }
 threads.sort((a,b)=>Number(b.draft)-Number(a.draft)||b.score-a.score||a.order-b.order);
 const selected=new Set(),roots=new Set(),protectedCount=threads.filter(t=>t.draft||t.important).length,rootTarget=Math.min(NATIONAL_CARD_LIMIT,Math.max(200,protectedCount));
 const take=t=>{if(!t.minimum.length||roots.has(t.root)||selected.size+t.minimum.length>NATIONAL_CARD_LIMIT)return;for(const id of t.minimum)selected.add(id);roots.add(t.root);};
 for(const t of threads)if(t.draft)take(t);for(const t of threads)if(roots.size<rootTarget)take(t);
 for(const id of candidates)if(selected.size<NATIONAL_CARD_LIMIT&&map.get(id).replyTo&&selected.has(map.get(id).replyTo))selected.add(id);
 for(const t of threads)if(selected.size<NATIONAL_CARD_LIMIT)take(t);
 return candidates.filter(id=>selected.has(id));
}
export function nativeViewIds(feed,scope='all',type=0){
 const views=feed.views??{},key=scope==='all'?(Object.hasOwn(views,'all:expanded')?'all:expanded':Object.hasOwn(views,'all:default')?'all:default':'all'):scope;
 const candidates=nativeCandidates(feed.stories??[],views[key]??[],type);
 return scope==='national'?selectNational(feed.stories??[],candidates,type):candidates;
}
export function nativeViewPosts(feed,scope='all',type=0){const byId=new Map((feed.stories??[]).map(p=>[p.id,p]));return nativeViewIds(feed,scope,type).map(id=>byId.get(id));}
