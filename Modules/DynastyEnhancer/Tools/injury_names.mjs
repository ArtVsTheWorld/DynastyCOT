// Only confirmed player-facing names. Unknown internal enum identifiers are
// omitted rather than turned into an invented diagnosis by camel-case splitting.
const verified=new Map([['kneemclpartialtear','Partial MCL Tear']]);
export function injuryName(value){
 if(typeof value!=='string')return null;
 const known=verified.get(value.replace(/[_\s-]/g,'').toLowerCase());if(known)return known;
 if(/^(?:Invalid_|None|Unknown)$/i.test(value))return null;
 // A captured already-localized label can be retained as supplied.
 if(/^[A-Z][A-Za-z]+(?: [A-Z][A-Za-z]+)+$/.test(value)&&!/[a-z][A-Z]/.test(value))return value;
 return null;
}
