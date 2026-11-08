import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import {socialDisplayText} from '../Tools/prose_cleanup.mjs';
import {contentPack,chooseTemplate} from '../Tools/story_templates.mjs';
import {renderStories} from '../Tools/story_renderer.mjs';

test('only the confirmed unsupported arrow changes; names, native punctuation and line breaks survive',()=>{
 for(const [before,after] of [
  ['Ranking movement: 5 → 3','Ranking movement: 5 to 3'],
  ['No. 5→No. 3','No. 5 to No. 3'],
  ['José Fernández\t →  São José','José Fernández to São José'],
  ['A→B→C','A to B to C'],
  ['First line\n5→3\nLast line','First line\n5 to 3\nLast line'],
  ['José, Łukasz, D’André, café — 31–26; e\u0301','José, Łukasz, D’André, café — 31–26; e\u0301']
 ]){assert.equal(socialDisplayText(before),after);assert.equal(socialDisplayText(socialDisplayText(before)),after);}
});
test('runtime glyph repair follows editorial originalBody guards without changing source packs or template identities',()=>{
 const folder=new URL('../Content/Runtime/',import.meta.url),files=fs.readdirSync(folder).filter(f=>f.endsWith('.json'));
 const raw=files.map(f=>[f,fs.readFileSync(new URL(f,folder))]),arrows=raw.flatMap(([file,bytes])=>{const p=JSON.parse(bytes);return Object.entries(p.families??{}).flatMap(([family,lines])=>Array.isArray(lines)?lines.filter(t=>t.body?.includes('\u2192')).map(t=>({file,family,id:t.id})):[]);});
 assert.ok(arrows.length>0,'The raw reviewed source remains intact');
 for(const lines of Object.values(contentPack.families))for(const t of lines)assert.ok(!t.body.includes('\u2192'),t.id);
 const patch=JSON.parse(fs.readFileSync(new URL('editorial011-pack.json',folder))),lookup=new Map(Object.entries(contentPack.families).flatMap(([family,lines])=>lines.map(t=>[t.id,{family,t}])));
 for(const override of patch.overrides){const actual=lookup.get(override.id);assert.equal(actual.family,override.family);assert.equal(actual.t.body,socialDisplayText(override.body));}
 const loader=fs.readFileSync(new URL('../Tools/story_templates.mjs',import.meta.url),'utf8');assert.ok(loader.indexOf('old.t.body!==patch.originalBody')<loader.indexOf('template.body=socialDisplayText(template.body)'));
 for(const [file,bytes] of raw)assert.ok(bytes.equals(fs.readFileSync(new URL(file,folder))));
});
test('expanded dynamic arrow text still obeys text limits and uses an eligible factual fallback',()=>{
 const family='test.glyph-budget';contentPack.families[family]=[{id:family+'.long',body:'{detail}'},{id:family+'.short',body:'Rank 5 to rank 3.',fallback:true}];
 try{const detail='x'.repeat(278)+'→';assert.equal([...detail].length,279);const chosen=chooseTemplate(family,{detail},'glyph-budget',{handle:'@test',week:2,transformBody:socialDisplayText});assert.equal(chosen.templateId,family+'.short');assert.ok([...chosen.body].length<=280&&Buffer.byteLength(chosen.body)<=512);}finally{delete contentPack.families[family];}
});
test('new publications repair dynamic text while same-week published stories remain byte-identical',()=>{
 const family='test.glyph-publication';contentPack.families[family]=[{id:family+'.one',body:'{detail}'}];
 const s={format:2,gameYear:27,sourceKind:'runtime',sourceUnchanged:true,sha256:'a'.repeat(64),leagueId:70613,userTeamIndex:1,season:{CurrentYear:0,CurrentSeasonYear:2026,CurrentWeekType:'RegularSeason',CurrentWeek:2,CurrentStage:'NFLSeason'},teams:[{id:1,name:'São José',logo:1}],players:[],games:[],performances:[],heisman:[],offers:[],commitments:[]},e={id:'glyph',category:'polls',teamIds:[1],scopes:['team'],priority:90,facts:{family,voice:'beat',detail:'José Fernández: 5→3'}};
 try{const generated=renderStories(s,[e]);assert.equal(generated.stories[0].body,'José Fernández: 5 to 3');const legacy={...generated.stories[0],body:'José Fernández: 5→3'},sameWeek={events:[e],stories:[legacy]},before=JSON.stringify(sameWeek);const refreshed=renderStories(s,[e],{sameWeek,authors:generated.authors});assert.equal(JSON.stringify(sameWeek),before);assert.deepEqual(refreshed.stories[0],legacy);assert.ok(refreshed.stories[0].body.includes('\u2192'));}finally{delete contentPack.families[family];}
});
