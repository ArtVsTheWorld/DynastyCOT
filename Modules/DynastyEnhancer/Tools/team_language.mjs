// New-publication prose only. School names/headlines and nickname modifiers
// remain untouched; captured team context distinguishes Liberty from Flames.
const escape=s=>s.replace(/[.*+?^${}()|[\]\\]/g,'\\$&');
const patterns=new Map();
const modifiers=/^(?:fans?|football|players?|teams?|supporters?|alumni|coaches?|staff|pride|territory|country|nation|jerseys?|uniforms?|logo|defense|offense|roster|recruiting|win|loss|game)\b/i;
export function teamNounGrammar(body,teams=[]){
 for(const team of teams){
  const nickname=team?.nickname?.trim(),school=team?.name?.trim();
  if(school&&/^the\s/i.test(school)&&body.includes(school))body=body.replace(new RegExp("\\bthe\\s+"+escape(school)+"\\b","gi"),school);
  if(!nickname||!body.includes(nickname)||!school||nickname.toLowerCase()===school.toLowerCase()||/^the\s/i.test(nickname))continue;
  let pattern=patterns.get(nickname);if(!pattern){pattern=new RegExp('(^|\\b(?:for|against|between|versus|vs\\.?|beat|beats|beating|defeated|over|face|faces|facing|host|hosts|hosting|with|and)\\s+)('+escape(nickname)+')(?=$|[^\\p{L}\\p{N}])','giu');if(patterns.size>=256)patterns.delete(patterns.keys().next().value);patterns.set(nickname,pattern);}
  body=body.replace(pattern,(all,before,noun,offset,source)=>{
   const tail=source.slice(offset+all.length);
   if(!before&&!/^\s+(?:are|were|have|had|will|can|could|should|must)\b/i.test(tail))return all;
   if(/^['’]/.test(tail)||modifiers.test(tail.trimStart())||!before&&/^\s*[:|–-]/.test(tail))return all;
   return before+(before?'the ':'The ')+noun;
  });
 }
 return body;
}
