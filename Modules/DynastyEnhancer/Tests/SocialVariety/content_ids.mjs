import {createHash} from 'node:crypto';
// An added/reordered entry cannot renumber existing wording. New wording gets
// a new identity; published cards keep their original IDs and stored text.
export function perspectiveId(family,template){
 const identity={body:template.body,style:template.style??null,when:[...(template.when??[])].sort(),unless:[...(template.unless??[])].sort()};
 return family+'.cot104.'+createHash('sha256').update(JSON.stringify(identity)).digest('hex').slice(0,16);
}
