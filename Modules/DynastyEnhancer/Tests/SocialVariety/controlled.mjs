// Fully fictional normalized inputs, including reliable grade/record inputs that
// were not retained in the older private event archives. No game/save access.
import fs from 'node:fs';import path from 'node:path';import {pathToFileURL} from 'node:url';
const [beforeRoot,afterRoot,output]=process.argv.slice(2);
const engines=await Promise.all([beforeRoot,afterRoot].map(root=>import(pathToFileURL(path.resolve(root,'Tools/feed_engine.mjs')))));
const phases=[['PreSeason',0],...Array.from({length:17},(_,w)=>['RegularSeason',w]),['BowlSeason1',17],['BowlSeason2',18],['BowlSeason3',19],['NationalChampionship',20],...Array.from({length:9},(_,w)=>['OffSeason',w])];
function fixture(brand,phase,week){const played=phase==='PreSeason'?0:phase==='RegularSeason'?Math.min(week,12):12;
 return {format:2,gameYear:27,sourceKind:'runtime',source:'fictional-variety-fixture',sourceUnchanged:true,sha256:'a'.repeat(64),leagueId:{F:101,B:102,'A+':103}[brand],userTeamIndex:1,
 season:{CurrentYear:0,CurrentSeasonYear:2027,CurrentWeekType:phase,CurrentWeek:week,CurrentStage:phase==='OffSeason'?'OffSeason':phase==='PreSeason'?'PreSeason':'NFLSeason',CurrentOffseasonStage:phase==='OffSeason'?week+1:0},
 teams:[{id:1,name:'Harbor State',nickname:'Captains',brandExposure:brand,mediaRank:brand==='F'?null:8,mediaLastRank:brand==='F'?null:10,logo:1,rivals:[2],wins:played,losses:0},{id:2,name:'Mesa',nickname:'Hawks',brandExposure:'B',logo:2,rivals:[1],wins:0,losses:played}],
 players:[{id:'p',name:'Evan Moss',firstName:'Evan',team:1,position:'QB',personality:'Leader',injury:'Uninjured'}],
 games:Array.from({length:played},(_,w)=>({id:'g'+w,year:0,week:w,phase:'RegularSeason',home:1,away:2,homeScore:31,awayScore:28,published:true,status:'HomeWon'})),
 performances:Array.from({length:played},(_,w)=>({player:'p',team:1,game:'g'+w,type:'GameOffensiveStats',stats:{PASSYARDS:250,PASSTDS:2,PASSINTS:0,RUSHYARDS:55,RUSHTDS:1}})),offers:[],commitments:[],heisman:[]};
}
const result={method:'Fictional fixtures, identical normalized facts for both engines. 31 consecutive publication stages for each of three brand grades; includes minimal preseason/offseason context. Does not represent historical program brand grades.',programs:[]};
for(const brand of ['F','B','A+']){const histories=[[],[]],editions=[];for(const [phase,week] of phases){const input=fixture(brand,phase,week),feeds=engines.map((engine,i)=>engine.buildFeed(structuredClone(input),null,{history:histories[i]}));
 for(let i=0;i<2;i++){if(feeds[i].posts.length>30)throw Error('Post-count inflation');for(const p of feeds[i].stories){if([...p.body].length>280||Buffer.byteLength(p.body)>512||/undefined|NaN/.test(p.body))throw Error('Invalid text');if(p.replyTo&&!feeds[i].stories.some(x=>x.id===p.replyTo))throw Error('Orphan');}histories[i].push(feeds[i]);}
 editions.push({phase,week,before:feeds[0].posts.map(p=>({id:p.id,author:p.author,body:p.body})),after:feeds[1].posts.map(p=>({id:p.id,author:p.author,body:p.body})),beforeGenerated:feeds[0].stories.length,afterGenerated:feeds[1].stories.length});
 }result.programs.push({brand,editions});}
fs.writeFileSync(output,JSON.stringify(result));console.log(JSON.stringify(result.programs.map(p=>({brand:p.brand,editions:p.editions.length,beforeCards:p.editions.reduce((n,e)=>n+e.before.length,0),afterCards:p.editions.reduce((n,e)=>n+e.after.length,0)}))));
