// Internal editorial metadata. Never sent to native widgets or used to remove news.
const cache=new WeakMap();
const neighborhoods=new WeakMap(),shingleCache=new Map();
const phraseCache=new Map();
const globalNeighbors=new WeakMap();
function phrasesFor(form){
 if(phraseCache.has(form))return phraseCache.get(form);
 const words=form.split(' '),phrases=[];
 for(let i=0;i+4<=words.length;i++){const w=words.slice(i,i+4);if(w.filter(x=>x!=='@'&&x!=='#').length>=3)phrases.push(w.join(' '));}
 const result=[...new Set(phrases)];if(phraseCache.size>=16384)phraseCache.delete(phraseCache.keys().next().value);phraseCache.set(form,result);return result;
}
export function structure(text){return text.toLowerCase().replace(/\{[a-z][a-z0-9]*\}/gi,' @ ').replace(/\b\d+(?:[.,–-]\d+)*\b/g,' # ').replace(/[^a-z@#]+/g,' ').trim().replace(/\s+/g,' ');}
export function templateFeatures(t){
 const old=cache.get(t);if(old?.body===t.body)return old.features;
 const form=structure(t.body),words=form.split(' ');
 const features={structure:form,opening:words.slice(0,5).join(' '),ending:words.slice(-5).join(' '),phrases:phrasesFor(form),angle:t.angle??null};
 cache.set(t,{body:t.body,features});return features;
}
export function varietyMemory(){return {forms:new Map(),openings:new Map(),endings:new Map(),phrases:new Map(),phraseForms:new Map(),angles:new Map(),last:null};}
// Cross-family near shapes must use the same 70% four-gram rule as review.
// An inverted phrase index avoids comparing every candidate with every card.
export function globalNearUse(memory,t){
 const f=templateFeatures(t);let cache=globalNeighbors.get(memory);if(!cache){cache=new WeakMap();globalNeighbors.set(memory,cache);}let neighbors=cache.get(f);
 // A pass only adds shapes/increases weights. Reuse structural matches while
 // its shape set is unchanged, but always read current weights and threshold.
 if(!neighbors||neighbors.count!==memory.forms.size){
  const hits=new Map();for(const phrase of f.phrases)for(const form of memory.phraseForms.get(phrase)??[])hits.set(form,(hits.get(form)??0)+1);
  neighbors={count:memory.forms.size,forms:[...hits].filter(([form,n])=>form!==f.structure&&n/Math.max(f.phrases.length,phrasesFor(form).length)>=.7).map(([form])=>form)};cache.set(f,neighbors);
 }
 let weight=memory.forms.get(f.structure)??0;for(const form of neighbors.forms){const value=memory.forms.get(form)??0;if(value>=.2)weight+=value;}return weight;
}
function shingles(form){let result=shingleCache.get(form);if(!result){const w=form.split(' ');result=new Set(w.slice(0,-3).map((_,i)=>w.slice(i,i+4).join(' ')));shingleCache.set(form,result);}return result;}
// Compile structural neighbors once per immutable family. This also catches
// superficially different IDs/wording that share >=70% of their 4-gram shape.
export function structuralNeighbors(variants){
 const old=neighborhoods.get(variants);if(old&&variants.length===old.lines.length&&variants.every((t,i)=>t===old.lines[i]&&t.body===old.bodies[i]))return old.map;
 const forms=[...new Set(variants.map(t=>templateFeatures(t).structure))],near=new Map(forms.map(f=>[f,new Set([f])]));
 for(let i=0;i<forms.length;i++)for(let j=0;j<i;j++){
  const a=shingles(forms[i]),b=shingles(forms[j]);if(!a.size||!b.size||Math.min(a.size,b.size)/Math.max(a.size,b.size)<.70)continue;
  let n=0;for(const x of a)if(b.has(x))n++;
  if(n/Math.max(a.size,b.size)>=.70){near.get(forms[i]).add(forms[j]);near.get(forms[j]).add(forms[i]);}
 }
 const map=new Map(variants.map(t=>[t,[...near.get(templateFeatures(t).structure)]]));neighborhoods.set(variants,{lines:[...variants],bodies:variants.map(t=>t.body),map});return map;
}
export function nearUse(memory,forms){let weight=0;for(const f of forms)weight+=memory.forms.get(f)??0;return weight;}
const increment=(map,key,weight)=>{if(key)map.set(key,(map.get(key)??0)+weight);};
export function rememberVariety(memory,features,weight=1,topic=''){
 if(!features||weight<=0)return;increment(memory.forms,features.structure,weight);increment(memory.openings,features.opening,weight);increment(memory.endings,features.ending,weight);
 for(const p of features.phrases??phrasesFor(features.structure)){increment(memory.phrases,p,weight);if(!memory.phraseForms.has(p))memory.phraseForms.set(p,new Set());memory.phraseForms.get(p).add(features.structure);}
 if(features.angle&&topic)increment(memory.angles,topic+'|'+features.angle,weight);
 memory.last=features;
}
export function repetitionCost(memory,t,topic=''){
 const f=templateFeatures(t),last=memory.last;
 const phrase=f.phrases.length?f.phrases.reduce((n,p)=>n+Math.min(4,memory.phrases.get(p)??0),0)/f.phrases.length:0;
 return (memory.forms.get(f.structure)??0)*24+(memory.openings.get(f.opening)??0)*6+(memory.endings.get(f.ending)??0)*3+phrase*4+
  (last?.structure===f.structure?80:0)+(last?.opening===f.opening?30:0)+(f.angle&&topic?(memory.angles.get(topic+'|'+f.angle)??0)*8:0);
}
