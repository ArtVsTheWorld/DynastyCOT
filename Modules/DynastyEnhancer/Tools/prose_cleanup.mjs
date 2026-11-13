// Final surface cleanup preserves handles, abbreviations and statistic values.
// The native News font shows U+2192 as tofu. Normalize only that confirmed
// unsupported symbol; Unicode names and native punctuation keep their bytes.
export function socialDisplayText(body){return body.replace(/[ \t]*\u2192[ \t]*/g,' to ');}
// Render-time agreement: templates can be reused with zero, one or many.
// Run only while choosing NEW wording, never over a published archive.
export function proofreadNewText(body){
 const singular={points:'point',yards:'yard',touchdowns:'touchdown',interceptions:'interception',sacks:'sack',wins:'win',losses:'loss',games:'game',attempts:'attempt',catches:'catch',receptions:'reception',tackles:'tackle',commits:'commit',players:'player',spots:'spot',votes:'vote',weeks:'week',seasons:'season',punts:'punt',snaps:'snap',completions:'completion',passes:'pass',fumbles:'fumble',offers:'offer',commitments:'commitment',awards:'award',titles:'title',seconds:'second',minutes:'minute',hours:'hour',days:'day'};
 return body.replace(/(?<![\d.,–-])\b1 (points|yards|touchdowns|interceptions|sacks|wins|losses|games|attempts|catches|receptions|tackles|commits|players|spots|votes|weeks|seasons|punts|snaps|completions|passes|fumbles|offers|commitments|awards|titles|seconds|minutes|hours|days)\b/gi,(_,word)=>'1 '+singular[word.toLowerCase()])
  .replace(/\b([Aa]) (\$?(?:8\d*|11|18)(?:[.,]\d+)?[KM]?)(?=[\s–-])/g,(_,article,value)=>(article==='A'?'An':'an')+' '+value)
  .replace(/\b(rankings) has\b/gi,'$1 have')
  .replace(/\b1 point separate them\b/g,'1 point separates them')
  .replace(/\b1 (point|yard|touchdown|interception|sack|win|loss|game|attempt|catch|reception|tackle|commit|player|spot|vote|week|season|punt|snap|completion|pass|fumble|offer|commitment|award|title|second|minute|hour|day) (are|were)\b/g,(_,unit,verb)=>'1 '+unit+(verb==='were'?' was':' is'));
}
export function capitalizeSentences(body){
 return body.replace(/^((?:@[A-Za-z0-9_.]+\s+)+)([a-z])/,(_,mentions,letter)=>mentions+letter.toUpperCase()).replace(/(^|\n|[!?]\s+|\.\s+)(["'“‘(]*)([a-z])/g,(all,before,quote,letter,offset,source)=>{
  if(before.startsWith('.')&&/(?:\b(?:vs|No|Mr|Mrs|Dr|Jr|Sr|St|i\.e|e\.g)|\b[A-Z])\.$/i.test(source.slice(0,offset+1)))return all;
  return before+quote+letter.toUpperCase();
 }).replace(/\bi(?=(?:['’](?:m|ve|ll|d))?\b)/g,(letter,offset,source)=>source[offset-1]==='@'||/^i\.e\./.test(source.slice(offset))?letter:'I');
}
