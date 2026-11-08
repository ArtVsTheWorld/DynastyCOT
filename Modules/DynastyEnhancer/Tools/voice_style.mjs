import {hash} from './story_authors.mjs';
// Stable editorial interests, not per-post punctuation skins. Personal native
// character rules remain authoritative (including restrained quarterbacks).
export function accountVoice(account,role,player=null,persona=0){
 if(role==='analyst')return ['wire','race','film','evidence','schedule','development'][persona%6];
 const seed=account.styleSeed??hash(account.handle??'');
 if(role==='beat')return ['wire','local','cautious'][seed%3];
 if(role==='fan')return ['excited','skeptical','wry','optimistic','rival'][seed%5];
 if(role==='official')return ['announcement','pride'][seed%2];
 if(['player','recruit'].includes(role))return personalVoice(account,player);
 return 'wire';
}
export function personalVoice(account,player){
 const seed=account.styleSeed??hash(account.handle??player?.id??'');
 if(player?.position==='QB'&&player.personality!=='Unpredictable')return 'measured';
 if(player?.personality==='Leader')return seed%4===0?'conversational':'measured';
 if(['Entertainer','Intense'].includes(player?.personality))return seed%3===0?'conversational':'expressive';
 return ['measured','conversational','expressive'][seed%3];
}
export function finishPersonalText(body,style){
 if(style==='measured'&&/[\p{L}\p{N}]$/u.test(body))return body+'.';
 if(style==='conversational')return body.replace(/\.$/,'');
 return body;
}
