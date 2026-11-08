import fs from 'node:fs';
import {nativeViewIds} from '../../Tools/feed_visibility.mjs';
const cases=[];
for(const mode of ['sparse','busy','draft','orphan','matching-child','fractional']){
 const stories=[],ids=[];
 const add=(id,fields={})=>{stories.push({id,category:'results',voice:'beat',priority:1,...fields});ids.push(id);};
 const categories=['results','polls','signings','injuries','recruit-film','offseason','awards','transfer','nil','schedule','analytics','upsets'];
 const voices=['beat','analyst','fan','official','player','recruit','model','cfp'];
 const count=mode==='sparse'?1:mode==='matching-child'?15:310;
 for(let i=0;i<count;i++){
  add('root'+i,{category:categories[i%categories.length],voice:voices[i%voices.length],priority:mode==='fractional'?(i%19)/7:i%10,essentialNews:i%9===0,coverageExempt:i%13===0,budgetProtected:i%17===0});
  if(i%3===0)add('child'+i,{replyTo:'root'+i,category:'injuries',voice:'fan'});
  if(i%9===0)add('nested'+i,{replyTo:'child'+i,category:'offers',voice:'recruit'});
 }
 if(mode==='draft')for(let i=224;i>0;i--)add('draft'+i,{category:'draft',voice:'nflDraft',transaction:true,draftPick:i,priority:250.4});
 if(mode==='orphan'){add('lost',{replyTo:'absent'});add('cycle1',{replyTo:'cycle2'});add('cycle2',{replyTo:'cycle1'});}
 const feed={stories,views:{all:ids,'all:default':ids.slice(0,3),'all:expanded':ids,national:ids,conference:ids.slice(0,257),team:ids.slice(0,37)}};
 cases.push({name:mode,feed,expected:Object.fromEntries(['all','national','conference','team'].map(scope=>[scope,Array.from({length:12},(_,type)=>nativeViewIds(feed,scope,type))]))});
}
fs.writeFileSync(process.argv[2],JSON.stringify({cases}));
