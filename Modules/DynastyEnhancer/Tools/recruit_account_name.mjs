// Presentation only: never change persistent handles or archived story content.
export function recruitAccountName(person,classLabel=''){
 const stars=Number.isInteger(person.starRating)?person.starRating:
  ({ONE_STAR:1,TWO_STAR:2,THREE_STAR:3,FOUR_STAR:4,FIVE_STAR:5})[person.stars];
 const rating=stars>=1&&stars<=5?`${stars}-Star `:'';
 const detail=rating+classLabel+(person.position??'');
 return person.name+(detail.trim()?' | '+detail.trim():'');
}
