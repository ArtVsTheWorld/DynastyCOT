// Conference labels remain native in the scope selector. Only authored prose
// uses the customary article and C-USA abbreviation.
export function conferenceGrammar(body){
 const name='(?:C-USA|CUSA|Conference USA|SEC|ACC|Big Ten|Big 10|Big 12|Sun Belt|Mountain West|American|Pac-12|MAC)';
 return body.replace(new RegExp('\\b(joins?|joined|joining|leaves?|left|leaving|in|into|from|to|of|across|within|throughout|leads?)\\s+(?:the\\s+)*('+name+')\\b','gi'),(all,verb,conference,offset,source)=>{
  // "in ACC play" and "of ACC teams" use a conference as a modifier.
  const tail=source.slice(offset+all.length);
  if(/^['’]|^\s+(?:teams?|programs?|players?|coaches?|play|games?|football|territory|country|membership|recruiting)\b/i.test(tail))return all;
  return verb+' the '+(/^(?:CUSA|Conference USA)$/i.test(conference)?'C-USA':conference);
 }).replace(/\bthe\s+the\s+(C-USA|SEC|ACC|Big Ten|Big 12|Sun Belt|Mountain West|American|Pac-12|MAC)\b/gi,'the $1');
}
