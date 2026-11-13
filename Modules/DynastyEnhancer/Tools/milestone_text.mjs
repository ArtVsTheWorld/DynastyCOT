const number=v=>typeof v==='number'?v:typeof v==='string'&&/^[0-9][0-9,]*(?:\.[0-9]+)?$/.test(v)?Number(v.replaceAll(',','')):NaN;
export function milestoneTotalDetail(family,values){
 if(!['history.coach-wins','history.team-wins','history.player-career'].includes(family))return '';
 const threshold=number(values.threshold),total=number(values.total),unit=family==='history.player-career'?values.metric:'wins';
 const minimum=typeof unit==='string'&&unit.includes('yards')?10:unit==='sacks'?.5:1;
 return Number.isFinite(threshold)&&Number.isFinite(total)&&total-threshold>=minimum&&unit?` Current total: ${values.total} ${unit}.`:'';
}
