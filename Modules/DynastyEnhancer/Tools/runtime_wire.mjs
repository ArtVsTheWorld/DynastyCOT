// Strict, lossless transport decoder; no native data is inferred or dropped.
export function runtimeRecords(raw){
 if(raw.format===1){if(!raw.records||Array.isArray(raw.records)||typeof raw.records!=='object')throw Error('Invalid runtime records');return Object.values(raw.records);}
 if(raw.format!==2||!Array.isArray(raw.types)||!Array.isArray(raw.rows)||raw.types.length>350000||raw.rows.length>350000)throw Error('Invalid compact runtime capture');
 const types=raw.types.map(type=>{
  if(!Array.isArray(type)||type.length!==2||typeof type[0]!=='string'||!Array.isArray(type[1])||type[1].length>16384||type[1].some(k=>typeof k!=='string'||['__proto__','prototype','constructor'].includes(k))||new Set(type[1]).size!==type[1].length)throw Error('Invalid compact runtime schema');
  return type;
 });
 const keys=new Set();return raw.rows.map(row=>{
  if(!Array.isArray(row)||row.length!==4||typeof row[0]!=='string'||keys.has(row[0])||!Number.isInteger(row[1])||!types[row[1]]||!(row[2]===null||Number.isInteger(row[2])&&row[2]>=0)||!Array.isArray(row[3]))throw Error('Invalid compact runtime row');
  keys.add(row[0]);const [type,fields]=types[row[1]];
  if(fields.length!==row[3].length)throw Error('Compact runtime field count differs');
  return {key:row[0],type,...(row[2]===null?{}:{capacity:row[2]}),v:Object.fromEntries(fields.map((key,i)=>[key,row[3][i]]))};
 });
}
