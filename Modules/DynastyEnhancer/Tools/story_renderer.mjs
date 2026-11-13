import {injuryName} from './injury_names.mjs';
import {weeklyBudget,budgetEventOrder,enforceWeeklyBudget,protectedNews} from './weekly_budget.mjs';
import {programExpectations} from './season_context.mjs';
import {playerReferenceQualifier,publicPosition} from './player_references.mjs';
import {heismanContext} from './heisman_context.mjs';
import {cleanLanguage} from './clean_language.mjs';
import {editorialAssignment,nationalCoverage} from './national_editorial.mjs';
import {createHash} from 'node:crypto';
import {recruitRankDetail} from './recruit_ranks.mjs';
import {hash,authorRegistry,analystPersona} from './story_authors.mjs';
import {likesFor,replyLikesFor,ENGAGEMENT_VERSION,engagementContext} from './story_engagement.mjs';
import {avatarFor} from './story_avatars.mjs';
import {chooseTemplate,templateHistory,contentPack as pack} from './story_templates.mjs';
import {addConversations} from './story_conversations.mjs';
import {publishOffer,publishReply} from './publication_policy.mjs';
import {canonicalTopic} from './conversation_topics.mjs';
import {personalVoice,finishPersonalText,accountVoice} from './voice_style.mjs';
import {rememberVariety} from './story_variety.mjs';
import {capitalizeSentences,socialDisplayText,proofreadNewText} from './prose_cleanup.mjs';
import {conferenceGrammar} from './conference_language.mjs';
import {teamNounGrammar} from './team_language.mjs';
import {scopeViews} from './feed_selection.mjs';
import {nativeViewIds} from './feed_visibility.mjs';
import {milestoneTotalDetail} from './milestone_text.mjs';
import {supporterContext,capturedSupporterContext} from './supporter_voice.mjs';
export const CONTENT_REVISION=pack.revision;
export const stampFor=category=>({'recruit-film':'COT HS Game','draft-discourse':'Draft','draft':'Draft','nil':'NIL',transfer:'Portal Watch','results':'Recap','team-stats':'Recap','player-stats':'Recap',recruiting:'Recruiting',offers:'Recruiting',schedule:'Upcoming',polls:'Rankings',heisman:'Heisman',cfp:'Playoff Watch',injuries:'Injury Report',standings:'Standings',watch:'Spotlight',lines:'Odds',holiday:'Calendar',analytics:'Analysis',upsets:'Upset','matchup-recap':'Matchup Recap','matchup-preview':'Matchup','season-end':'Season Review',coaching:'Coaching',postseason:'Postseason','conference-talk':'Conference Talk',offseason:'Offseason',history:'Milestone',championship:'Title Race',bowls:'Bowl Watch',records:'Record Book',streaks:'Streak',awards:'Award'}[category]??'News');
const personalities=new Set(['Leader','Intense','Entertainer','TeamPlayer','Unpredictable']);
const final=g=>g&&g.published!==false&&['HomeWon','AwayWon','Tie'].includes(g.status);
const ranked=n=>Number.isInteger(n)&&n>0&&n<=25;
const numeric=x=>Number.isFinite(x);
const editorialKeys=new Set(['family','voice','persona','analystPersona','importance','huge','programDisparity','eventFocus','focusTier','focusTeams']);
function unchangedPublishedFacts(old,current){if(old===null||typeof old!=='object')return old===current;if(!current||typeof current!=='object')return false;return Object.entries(old).every(([k,v])=>editorialKeys.has(k)||unchangedPublishedFacts(v,current[k]));}
// New selection/budget policies do not retire an existing publication. Keep its
// original account/text only while its captured facts and actual parent survive.
export function preserveSameWeekStories(stories,events,sameWeek){
 if(!sameWeek)return stories;
 const current=new Map(events.map(e=>[e.id,e])),prior=new Map((sameWeek.events??[]).map(e=>[e.id,e])),eligible=new Map();
 for(const p of sameWeek.stories??[]){const old=prior.get(p.eventId),now=current.get(p.eventId);if(old&&now&&unchangedPublishedFacts(old.facts,now.facts))eligible.set(p.id,p);}
 const retained=new Map();let changed=true;
 while(changed){changed=false;for(const p of eligible.values())if(!retained.has(p.id)&&(!p.replyTo||retained.has(p.replyTo))){retained.set(p.id,p);changed=true;}}
 const result=stories.map(p=>retained.get(p.id)??p),ids=new Set(result.map(p=>p.id));for(const p of retained.values())if(!ids.has(p.id)){result.push(p);ids.add(p.id);}return result;
}
export function renderStories(s,events,options={}){
 if(options.sameWeek){const priorEvents=new Set((options.sameWeek.events??[]).map(e=>e.id));events=events.filter(e=>priorEvents.has(e.id)||!e.facts.opinion&&!e.facts.calendarKey&&e.category!=='recruit-film');}
 const finish=stories=>enforceWeeklyBudget(preserveSameWeekStories(stories,events,options.sameWeek),events,new Set(options.sameWeek?.stories?.map(p=>p.id)??[]));
 const initial=renderPass(s,events,options),budgeted=finish(initial.stories),views=scopeViews(budgeted,{phase:s.season.CurrentWeekType,userTeamIndex:s.userTeamIndex}),byId=new Map(initial.stories.map(p=>[p.id,p])),visible=new Set();
 for(const view of ['all','team','conference','national'])for(const id of nativeViewIds({stories:budgeted,views},view))visible.add(id);
 if(!visible.size)return {...initial,stories:finish(initial.stories)};
 // Hidden CPU candidates must not exhaust wording before the user sees it.
 // Refine actual displayed events, then keep every other stored card.
 const eventOrder=new Map();for(const id of visible){const event=byId.get(id)?.eventId;if(event&&!eventOrder.has(event))eventOrder.set(event,eventOrder.size);}
 const selected=events.filter(e=>eventOrder.has(e.id)).sort((a,b)=>eventOrder.get(a.id)-eventOrder.get(b.id));
 const refined=renderPass(s,selected,{...options,authors:initial.authors,visibleIds:visible}),replacement=new Map(refined.stories.filter(p=>visible.has(p.id)).map(p=>[p.id,p]));
 return {...initial,authors:refined.authors,stories:finish(initial.stories.map(p=>replacement.get(p.id)??p))};
}
function renderPass(s,events,{authors={},history=[],sameWeek=null,onTemplates=null,visibleIds=null}={}){
 if(!pack.enabled)return {stories:[],authors};
 const qualify=playerReferenceQualifier(s.players,{staffNames:s.teams.flatMap(t=>[t.headCoach?.name,t.offensiveCoordinator?.name,t.defensiveCoordinator?.name]).filter(Boolean)});
 const coverage=nationalCoverage(s),registry=authorRegistry(authors),teams=new Map(s.teams.filter(t=>t.id!==255).map(t=>[t.id,t])),players=new Map(s.players.map(p=>[p.id,p])),games=new Map(s.games.map(g=>[g.id,g]));
 const budget=weeklyBudget();
 const stories=[],byId=new Map(),reactionGames=new Set(),welcomedRecruits=new Set(),sameStories=new Map((sameWeek?.stories??[]).map(p=>[p.id,p])),publishedEvents=new Map((sameWeek?.events??[]).map(e=>[e.id,e])),reactionAccounts=new Map(),reactionTopics=new Map(),replyCounts=new Map();
 const edition=`${s.leagueId}/${s.season.CurrentSeasonYear}/${s.season.CurrentWeekType}/${s.season.CurrentWeek}`,usedBodies=new Set(),publication={leagueId:s.leagueId,userTeamIndex:s.userTeamIndex,season:s.season.CurrentSeasonYear,phase:s.season.CurrentWeekType,week:s.season.CurrentWeek};
 const usage=templateHistory(history,publication,s.season.CurrentWeek);
 const engagement=engagementContext(s),playersByName=new Map(),performancesByGame=new Map();
 for(const p of s.players){const list=playersByName.get(p.name)??[];list.push(p);playersByName.set(p.name,list);}
 const healthyRoster=new Map();for(const p of s.players)if(p.injury==='Uninjured'){const list=healthyRoster.get(p.team)??[];list.push(p);healthyRoster.set(p.team,list);}for(const list of healthyRoster.values())list.sort((a,b)=>a.id.localeCompare(b.id));
 for(const e of events)if(e.category==='player-stats'){const list=performancesByGame.get(e.facts.gameId)??[];list.push(e);performancesByGame.set(e.facts.gameId,list);}
 const staff=s.teams.map(t=>t.headCoach).filter(c=>c?.name&&c.lastName),coachReferences=new Map();
 const surnames=new Map();for(const c of staff){const names=surnames.get(c.lastName)??new Set();names.add(c.name);surnames.set(c.lastName,names);}for(const c of staff){const ambiguous=surnames.get(c.lastName).size>1;coachReferences.set(c.name,'Coach '+(ambiguous?c.name:c.lastName));}
 const teamValues=t=>({school:t?.name,nickname:t?.nickname??t?.name,coachLastName:t?.headCoach?.lastName??null,hashtag:t?.hashtags?.[0]??''});
 function add(e,family,values,{role='beat',team=teams.get(e.teamIds[0]),player=null,suffix='main',priority=e.priority,curated=e.curated,replyTo=null,persona=0,important=false}={}){
  if(sameWeek&&publishedEvents.has(e.id)&&!sameStories.has(e.id+':'+suffix)&&unchangedPublishedFacts(publishedEvents.get(e.id).facts,e.facts))return null;
  if(role==='official'&&e.facts.returningPortal)return null;
  if(sameWeek&&publishedEvents.has(e.id)&&sameStories.has(e.id+':main')&&!sameStories.has(e.id+':'+suffix)&&['reply.fan.upset-loss','context.upset-rival','context.upset-expectation'].includes(family))return null;
  if(!team&&['official','fan'].includes(role)||['player','recruit'].includes(role)&&!player)return null;
  const parent=replyTo?byId.get(replyTo):null;if(replyTo&&(!parent||!(important&&e.category==='upsets')&&!publishReply(s,e.id+':'+suffix,e.id.startsWith('portal-opening:')?parent.canonicalTopic??replyTo:replyTo)))return null;
  ({role,persona}=editorialAssignment(s,e,family,role,team,persona,coverage));
  if(!budget.allows(e,role,{reply:!!replyTo,published:sameStories.has(e.id+':'+suffix)}))return null;
  const publishedAccount=sameStories.get(e.id+':'+suffix);if(publishedAccount){role=publishedAccount.voice;const desk=analystPersona(publishedAccount.author);if(role==='analyst'&&desk>=0)persona=desk;}
  // A previously published personal main post can be revisited through a
  // reporter branch that deliberately did not pass a speaker. Resolve only
  // the event's actual player after restoring its published voice.
  if(['player','recruit'].includes(role)){player??=players.get(e.playerId)??null;if(!player)return null;}
  const account=registry.get(s,role,team,player,persona),id=e.id+':'+suffix;if(!account)return null;
  const style=['player','recruit'].includes(role)?personalVoice(account,player):null;
  if(role==='fan'&&!replyTo&&!sameStories.has(id)&&family!=='fan.picks'){const game=games.get(e.facts.gameId??(e.thread?.startsWith('game:')?e.thread.slice(5):null));const opponent=game&&teams.get(team?.id===game.home?game.away:game.home);const supporter=supporterContext(game,team,opponent,e.facts)??capturedSupporterContext(e,team,teams);if(supporter){family=supporter.family;values={...values,...supporter.values};}}
  const filled={rival:role==='fan'?(team?.rivals??[]).map(id=>teams.get(id)?.name).filter(Boolean).sort()[0]??null:null,recruitRankDetail:recruitRankDetail(values,e.id,{excludeNational:family==='preseason.recruit.watch'}),starDescription:Number.isInteger(values.stars)?values.stars+'-star ':'',...values,...(values.position?{position:publicPosition(values.position)}:{}),...(parent?{parentHandle:parent.handle}:{})};
  if(role==='player'&&player&&typeof filled.playerName==='string'){const subject=(playersByName.get(filled.playerName)??[]).find(p=>p.team===player.team||p.team===255&&['offers','recruiting'].includes(e.category)&&e.teamIds.includes(player.team));if(subject&&subject.id!==player.id)filled.playerName=subject.firstName||subject.name.trim().split(/\s+/)[0];}
  const transformBody=body=>{
   if(role==='fan'&&coachReferences.has(filled.coachName)){const full=filled.coachName,reference=coachReferences.get(full);body=body.includes('Coach '+full)?body.replaceAll('Coach '+full,reference):body.replaceAll(full,reference);}
   if(family!=='transactions.position-change')body=qualify(body,{playerId:e.playerId,teamIds:e.teamIds,includePosition:['recruiting','offers','signings','transfer','draft','injuries','depth-chart','watch'].includes(e.category)||family.includes('all-american')||e.category==='preseason'&&!['player','recruit'].includes(role)});
   if(filled.prospectType&&filled.recruitKind!=='high school'&&!filled.returningPortal&&filled.playerName&&!['player','recruit'].includes(role)){
    const p=players.get(e.playerId),ref=p&&publicPosition(p.position)+' '+p.name;
    if(ref&&body.includes(ref)&&!body.toLowerCase().includes((filled.prospectType+ref).toLowerCase()))body=body.replace(ref,filled.prospectType+ref);
    if(['analyst','beat','official'].includes(role)&&filled.recruitKind==='transfer'&&filled.formerSchool&&!body.includes(filled.formerSchool))body+=' Previously at '+filled.formerSchool+'.';
   }body=proofreadNewText(socialDisplayText(conferenceGrammar(teamNounGrammar(body,e.teamIds.map(id=>teams.get(id))).replace(/\b(Jr|Sr|St)\.\.(?=\s|$)/g,'$1.'))));return capitalizeSentences(style?finishPersonalText(body,style):body);
  };
  const renderKey=createHash('sha256').update(JSON.stringify({family,values:filled,handle:account.handle,style})).digest('hex');
  const oldStory=sameStories.get(id),baseOld=oldStory?.rawBody?{...oldStory,body:oldStory.rawBody}:oldStory,oldEvent=publishedEvents.get(e.id),preservePublished=oldStory&&oldEvent&&unchangedPublishedFacts(oldEvent.facts,e.facts);
  const nativeVoice=accountVoice(account,role,player,persona);account.character??={version:1,voice:nativeVoice};
  const voice=['player','recruit'].includes(role)?nativeVoice:account.character.voice,varietyTopic=(team?.id??'national')+'|'+e.category;
  // Context is additive and never changes the legacy factual render key.
  // Only expose details actually present in the parent as reply anchors.
  const anchor=parent?[values.statLine,values.score,values.playerName,values.opponent,values.school,values.winner,values.loser].find(x=>typeof x==='string'&&x.length>=3&&parent.body.includes(x)):null;
  const rankMovement=ranked(values.rank)?values.movement==='up'&&ranked(values.previousRank)&&values.rank<values.previousRank?`Up from No. ${values.previousRank}.`:values.movement==='down'&&ranked(values.previousRank)&&values.rank>values.previousRank?`Down from No. ${values.previousRank}.`:['same','unchanged'].includes(values.movement)?`Unchanged at No. ${values.rank}.`:values.movement==='entry'?'A new Top-25 entry.':null:null;
  const detailSubject=players.get(e.playerId),individualDetail=!!detailSubject&&typeof values.detail==='string'&&!values.detail.startsWith('Giveaways:')&&values.detail.startsWith(detailSubject.name+':');
  const contextual={...filled,totalDetail:milestoneTotalDetail(family,filled),roleFan:role==='fan',rolePersonal:['player','recruit'].includes(role),roleAnalyst:role==='analyst',roleLocal:['beat','official'].includes(role),roleBeat:role==='beat',roleOfficial:role==='official',parentDetail:anchor??null,rankMovement,individualDetail,fiveStarKnown:values.stars===5,personalmeasured:style==='measured',personalconversational:style==='conversational',personalexpressive:style==='expressive',rivalPerspective:parent&&e.facts.rivalReplyTeam===team?.id,
   shortStatLine:typeof filled.statLine==='string'?filled.statLine.split(', ')[0]:null,
   recordContext:team&&Number.isInteger(team.wins)&&Number.isInteger(team.losses)&&team.wins+team.losses>0?`${team.wins}–${team.losses}`:null};
  if(parent&&e.category==='results'&&team){const game=games.get(e.facts.gameId);if(game){contextual.opponent=team.id===game.home?game.awayName:game.homeName;contextual.score=Math.max(game.homeScore,game.awayScore)+'–'+Math.min(game.homeScore,game.awayScore);}}
  let chosen=chooseTemplate(family,contextual,edition+'|'+id+'|'+account.handle,{handle:account.handle,week:s.season.CurrentWeek,history,sameStory:baseOld,renderKey,transformBody,style,voice,topic:varietyTopic,avoidBodies:usedBodies,publication,onCandidates:onTemplates,usage});if(!chosen)return null;
  let voiceTemplateId=null;const rawBody=cleanLanguage(chosen.body);
  if(oldStory?.voiceTemplateId&&role==='analyst'&&[2,3,5].includes(persona)&&!family.startsWith('playoff.')&&!family.startsWith('buzz.')&&!family.startsWith('review.')&&!family.startsWith('draft-talk.')&&!family.startsWith('roster.')){const negative=values.verdict==='negative'||values.negative===true||values.metric==='turnoverMargin'&&values.margin<0||values.metric==='thirdDownConversionRate'&&values.rate<=.3||values.metric==='redZoneTouchdownRate'&&values.rate<=.45||/third_low|red_low|turnover_minus/.test(family);const positive=values.verdict==='positive'||!values.verdict&&(e.category==='awards'||/third_high|red_high|turnover_plus/.test(family));const voiceFamily='desk.'+({2:'marcus',3:'tina',5:'chris'})[persona]+(negative?'.negative':positive?'.positive':'.neutral');const voiced=negative||positive?chooseTemplate(voiceFamily,{...filled,analysisText:rawBody},edition+'|'+id,{handle:account.handle,week:s.season.CurrentWeek,history,sameStory:oldStory?.voiceTemplateId?{...oldStory,templateId:oldStory.voiceTemplateId}:null,renderKey,transformBody,publication,usage,onCandidates:onTemplates}):null;if(voiced){voiceTemplateId=voiced.templateId;chosen={...chosen,body:voiced.body};}}
  chosen={...chosen,body:capitalizeSentences(socialDisplayText(cleanLanguage(chosen.body)))};
  const reaction=['player','fan'].includes(role),accountKey=account.handle.toLowerCase(),topic=e.facts.gameId?e.facts.gameId+':'+team?.id:null;
  if(reaction&&!important&&((reactionAccounts.get(accountKey)??0)>=pack.publicationPolicy.reactionPostsPerAccount||(topic&&(reactionTopics.get(topic)??0)>=pack.publicationPolicy.reactionsPerGameTeam)))return null;
  if(parent&&(replyCounts.get(parent.id)??0)>=pack.publicationPolicy.maximumRepliesPerParent)return null;
  const p={id,eventId:e.id,category:e.category,templateFamily:family,voice:role,author:account.author,handle:account.handle,...chosen,...(voiceTemplateId?{rawBody}:{}),voiceTemplateId,renderKey,stamp:parent?'Reply':stampFor(e.category),...avatarFor(role,team,player,persona),likes:likesFor(s,id,role,team,e,sameStories.get(id),engagement),priority,scopes:parent?parent.scopes:e.scopes,teamIds:e.teamIds,playerId:['player','recruit'].includes(role)?player.id:e.playerId,eventWeek:e.eventWeek,thread:e.thread,curated:curated!==false&&parent?.curated!==false,primaryScope:e.primaryScope,contentRevision:CONTENT_REVISION,...(e.facts.eventFocus?{eventFocus:e.facts.eventFocus,focusTier:e.facts.focusTier,focusTeams:e.facts.focusTeams}:{}),newsCycle:e.newsCycle??null};
  p.userRelevant=!!(s.recruitInterests?.includes(e.playerId)||parent?.userRelevant);p.canonicalTopic=canonicalTopic(e);p.budgetProtected=!parent&&protectedNews(e,role);
  // Verified honors and primary commitment/signing announcements remain in
  // the browsable edition when flavor budgets are full. This does not expand
  // native view caps or create additional posts/reactions.
  if(!parent&&['awards','upsets','records','injuries'].includes(e.category)&&['beat','analyst','official'].includes(role))p.essentialNews=true;
  if(!parent&&e.category==='recruiting'&&(e.facts.flipped||e.facts.decommitted)&&role!=='fan')p.essentialNews=true;
  if(!parent&&suffix==='main'&&['recruiting','signings'].includes(e.category)&&role!=='fan'&&(e.facts.signed===true||e.facts.newlyObserved&&['HardCommitted','Signed'].includes(e.facts.commitment)))p.essentialNews=true;
  if(e.category==='heisman')p.heismanContextVersion=1;
  if(e.category==='draft-discourse'&&role==='analyst')p.transaction=true;
  if(!parent&&(e.id.startsWith('commit:')||e.category==='signings')&&(['beat','analyst'].includes(role))){p.coverageExempt=true;}
  // Captured roster/alignment reports must remain browsable even when
  // preseason training reactions fill a small program's flavor allowance.
  if(!parent&&['departure','realignment','alignment-history','rename'].includes(e.facts.carryoverKind)&&['beat','analyst'].includes(role))p.coverageExempt=true;
  if(family==='transactions.position-change')p.coverageExempt=true;
  if(!parent&&['analyst','beat'].includes(role)&&e.category==='coaching')p.coverageExempt=true;
  if(role==='nflDraft'){p.transaction=true;p.draftPick=values.draftPick;p.draftRound=values.draftRound;}
  if(e.category==='recruit-film'&&e.facts.recordSource==='mod-modeled-hs')p.recordSource='mod-modeled-hs';
  if(['analyst','beat'].includes(role)&&(e.category==='coaching'||e.category==='transactions'||e.category==='nil'||family==='transactions.signed'))p.transaction=true;
  if(parent)p.likes=replyLikesFor(s,id,parent.likes,sameStories.get(id));p.engagementVersion=ENGAGEMENT_VERSION;
  if(parent){p.replyTo=parent.id;replyCounts.set(parent.id,(replyCounts.get(parent.id)??0)+1);}
  if(reaction){reactionAccounts.set(accountKey,(reactionAccounts.get(accountKey)??0)+1);if(topic)reactionTopics.set(topic,(reactionTopics.get(topic)??0)+1);}
  p.varietyTopic=varietyTopic;p.authorVoice=voice;
  const published=preservePublished||oldStory?.renderKey===renderKey?oldStory:p;
  rememberVariety(usage.memory,published.variety,visibleIds?visibleIds.has(published.id)?1:0:published.scopes?.length?1:0,varietyTopic);
  budget.record(e,{reply:!!replyTo});stories.push(published);usedBodies.add(published.body);byId.set(published.id,published);return published;
 }
 function performanceLine(stats){
  const parts=[];
  if(numeric(stats.passingYards)&&numeric(stats.passingTouchdowns))parts.push(`${stats.passingYards} passing yards and ${stats.passingTouchdowns} TD`);
  if(numeric(stats.rushingYards)&&numeric(stats.rushingTouchdowns))parts.push(`${stats.rushingYards} rushing yards and ${stats.rushingTouchdowns} TD`);
  if(numeric(stats.receivingYards)&&numeric(stats.receivingTouchdowns))parts.push(`${stats.receivingYards} receiving yards and ${stats.receivingTouchdowns} TD`);
  if(numeric(stats.sacks)&&stats.sacks>0)parts.push(`${stats.sacks} sacks`);
  if(numeric(stats.interceptionsCaught)&&stats.interceptionsCaught>0)parts.push(`${stats.interceptionsCaught} interceptions`);
  if(numeric(stats.soloTackles)&&stats.soloTackles>=10)parts.push(`${stats.soloTackles} solo tackles`);
  // Keep the strongest two lines; a shorter factual form is tried if names are long.
  return parts.slice(0,2).join(', ');
 }
 function perfContext(e){const f=e.facts,g=games.get(f.gameId),p=players.get(e.playerId);if(!p||!final(g))return null;const t=teams.get(e.teamIds[0]);if(!t)return null;
  const win=g.homeScore===g.awayScore?null:g.homeScore>g.awayScore?g.home:g.away,opponent=t.id===g.home?g.awayName??teams.get(g.away)?.name:g.homeName??teams.get(g.home)?.name;
  return {...teamValues(t),...f,opponent,statLine:performanceLine(f.stats),win:win===t.id,resultLine:win===null?'The game finished tied.':win===t.id?'A team win to go with it.':'The individual numbers came in a team loss.'};}
 for(const e of budgetEventOrder(events)){if(e.category==='offers'&&!publishOffer(s,e))continue;const f=e.facts,t=teams.get(e.teamIds[0]),v={...teamValues(t),...f,giveawayLabel:e.facts.giveaways===1?'giveaway':'giveaways'};
  if(f.family){const role=f.voice??(e.category==='analytics'?'analyst':'beat'),p=players.get(e.playerId),values={...v,...f,...(p&&t?{playerHandle:registry.get(s,'player',t,p).handle}:{})};if(f.tagPlayerId){const tagged=players.get(f.tagPlayerId);if(tagged)values.tagHandle=registry.get(s,tagged.team===255?'recruit':'player',teams.get(tagged.team)??t,tagged).handle;}const parent=add(e,f.family,values,{role,player:['player','recruit'].includes(role)?p:null,persona:f.persona??(role==='analyst'?1:0),important:f.seasonEnding===true});if(parent&&p&&f.signed&&(e.category==='signings'||f.family==='postseason.signed')&&!welcomedRecruits.has(p.id)){welcomedRecruits.add(p.id);add(e,'cycle.team-welcome',{...values,tagHandle:registry.get(s,'recruit',t,p).handle},{role:'official',suffix:'signed-welcome',priority:e.priority+1});}if(parent&&f.buzzReply&&teams.has(f.replyTeam))add(e,'buzz.pick-reply',{...values,school:teams.get(f.replyTeam).name},{role:'fan',team:teams.get(f.replyTeam),suffix:'pick-reply',replyTo:parent.id,priority:e.priority-1});if(parent&&Number.isInteger(f.rivalReplyTeam)){const other=teams.get(f.rivalReplyTeam);if(other)add(e,'late.fan-reply',{...values,school:other.name,opponent:t?.name},{role:'fan',team:other,suffix:'rival-reply',replyTo:parent.id,priority:e.priority-1});}if(parent&&f.debate&&t)add(e,'preseason.debate.fan',{...values,analystHandle:parent.handle},{role:'fan',team:t,suffix:'reply',replyTo:parent.id,priority:e.priority-1});continue;}
  if(e.category==='results'){
   const g=games.get(f.gameId);if(!final(g))continue;
   if(f.tie){add(e,'beat.tie',v);continue;}
   if(s.reconstructed){Object.assign(v,{score:Math.max(g.homeScore,g.awayScore)+'\u2013'+Math.min(g.homeScore,g.awayScore)});add(e,f.margin>=21?'beat.result_wide':f.margin<=7?'beat.result_close':'beat.result',v);continue;}
   const winningId=g.homeScore>g.awayScore?g.home:g.away,winningTeam=teams.get(winningId);
   Object.assign(v,teamValues(winningTeam),{score:Math.max(g.homeScore,g.awayScore)+'–'+Math.min(g.homeScore,g.awayScore),winnerScore:Math.max(g.homeScore,g.awayScore),loserScore:Math.min(g.homeScore,g.awayScore),opponent:f.loser});
   if(f.gameStory){const n=f.gameStory;add({...e,playerId:n.player??e.playerId},'narrative.game',{...v,...n},{role:'beat',team:winningTeam??t,suffix:'box-story',priority:e.facts.eventFocus?150:e.priority+2});}
   if(winningTeam){const family=v.loserScore===0?'team.shutout':f.rivalry?'team.rivalry_win':f.margin<=7?'team.narrow_win':'team.win';add(e,family,v,{role:'official',team:winningTeam});}
   else add(e,f.margin>=21?'beat.result_wide':f.margin<=7?'beat.result_close':'beat.result',v,{team:winningTeam??teams.get(e.teamIds.find(id=>teams.has(id)))});
   if(reactionGames.has(g.id))continue;reactionGames.add(g.id);
   for(const own of [teams.get(g.home),teams.get(g.away)].filter(Boolean)){const userWon=winningId===own.id;
   const ownScore=own.id===g.home?g.homeScore:g.awayScore,opponentScore=own.id===g.home?g.awayScore:g.homeScore,opponent=own.id===g.home?f.away:f.home;
   const performances=(performancesByGame.get(g.id)??[]).filter(x=>players.get(x.playerId)?.team===own.id).sort((a,b)=>b.priority-a.priority||a.id.localeCompare(b.id));
   const best=performances[0],bestPlayer=best&&players.get(best.playerId);
   const values={...v,...teamValues(own),opponent,ownScore,opponentScore,playerName:bestPlayer?.name??null,score:ownScore+'–'+opponentScore};
   const seed=hash(edition+'|reaction|'+g.id);
   if(userWon&&seed%2===0){
    const roster=healthyRoster.get(own.id)??[];
    const author=roster.length?roster[seed%roster.length]:bestPlayer;
    if(author){const personality=personalities.has(author.personality)?author.personality:'Neutral';let family='player.win.'+personality;
     if(author.position==='QB'&&['Leader','Intense','Neutral'].includes(personality))family=f.rivalry&&personality==='Intense'?'player.rivalry':'player.win.QB';
     // Personal production is only available when the author actually owns that stat line.
     const self=performances.find(x=>x.playerId===author.id);const receivingYards=self?.facts.stats.receivingYards??null;
     add(e,family,{...values,playerName:bestPlayer?.id===author.id?null:values.playerName,receivingYards},{role:'player',team:own,player:author,suffix:'reaction:'+own.id,priority:e.priority-8});}
   }else{
    let family=userWon?f.rivalry?'fan.rivalry':f.margin<=7?'fan.narrow_win':'fan.win':'fan.loss';
    const expectation=programExpectations(s,own);if(!userWon&&expectation.known&&expectation.verdict==='negative')family='cycle.fan-loss-standard';values.expectationNote=expectation.note;
    const twoPicks=performances.find(x=>x.facts.stats.interceptionsThrown===2);if(!userWon&&twoPicks&&own.headCoach?.lastName){family='fan.picks';values.playerName=twoPicks.facts.playerName;}
    add(e,family,values,{role:'fan',team:own,suffix:'reaction:'+own.id,priority:e.priority-8});
   }
   }
  }else if(e.category==='team-stats'){
   const g=games.get(f.gameId);v.opponent=g&&(t?.id===g.home?g.awayName??teams.get(g.away)?.name:g.homeName??teams.get(g.home)?.name);add(e,'beat.team_stats',v,{curated:false});
  }else if(e.category==='player-stats'){
   const c=perfContext(e);if(!c||!c.statLine)continue;const p=players.get(e.playerId),team=teams.get(p.team);add(e,'beat.performance',c,{team});
  }else if(e.category==='polls'){
   v.places=Math.abs(f.rank-f.previousRank);v.placeLabel=v.places===1?'place':'places';if(f.movement==='up')add(e,'team.poll_up',v,{role:'official'});
   else add(e,'beat.poll_'+({up:'up',down:'down',entry:'entry',unchanged:'same'}[f.movement]),v);
  }else if(e.category==='offers'||e.category==='recruiting'){
   const p=players.get(e.playerId),team=teams.get(e.teamIds.find(id=>teams.get(id)?.name===f.school));if(!p||!team)continue;
   const values={...teamValues(team),...f},personality=personalities.has(p.personality)?p.personality:'Neutral',account=registry.get(s,'recruit',team,p);
   if(e.category==='offers')add(e,f.recruitKind==='transfer'&&f.formerSchool?'roster.transfer-offer':'recruit.offer.'+(account.faithLanguage?'faith':personality),values,{role:'recruit',team,player:p});
   else if(f.newlyObserved){
    const rivalry=f.recruitKind==='transfer'&&f.rivalTransfer&&f.formerSchool;
    const hard=f.commitment==='HardCommitted';
    if(hard)add(e,(f.recruitKind==='transfer'&&f.formerSchool?'roster.transfer-announce.':'roster.announce.')+personality,values,{role:'recruit',team,player:p});
    const reportEvent={...e,facts:{...f,...(f.stars===5?{analystPersona:0}:{})}};
    add(reportEvent,rivalry?'roster.rival-transfer':f.stars===5?'roster.major':f.recruitKind==='transfer'&&f.formerSchool?'roster.destination':'roster.commit',values,{role:f.stars>=4?'analyst':'beat',team,persona:f.stars===5?0:hash(e.id+'|recruit-writer')%3===0?3:5,suffix:hard?'report':'main'});
   }
  }else if(e.category==='injuries'){
   v.injuryDetail=injuryName(f.injuryType)?' Listed injury: '+injuryName(f.injuryType)+'.':'';v.timetable=f.timetable?.text??'A return timetable has not been established.';add(e,f.recovered?'beat.recovery':'injury.report',v);
  }else if(e.category==='schedule'){
   const g=games.get(f.gameId);if(!g||final(g))continue;
   v.awayLabel=(ranked(f.awayRank)?'#'+f.awayRank+' ':'')+f.away;v.homeLabel=(ranked(f.homeRank)?'#'+f.homeRank+' ':'')+f.home;v.rivalryDetail=f.rivalry?' Rivalry week. That one carries a little extra weight.':'';
   if(t){Object.assign(v,teamValues(t),{opponent:t.id===g.home?f.away:f.home});add(e,'team.upcoming',v,{role:'official',team:t});}else add(e,'beat.preview',v);
  }else if(e.category==='watch'){
   v.watchDetail=f.basis==='heismanWinner'?" He's this season's Heisman Trophy winner.":f.basis==='heisman'?' He’s already part of the Heisman conversation.':' His recent production put him on this week’s watch list.';add(e,'beat.watch',v);
  }else if(e.category==='standings'){
   v.leaders=f.leaders.map(x=>`${x.school} (${x.wins}–${x.losses})`).join(', ');add(e,f.independent?'beat.independents':'beat.standings',v,{team:t});
  }else if(e.category==='cfp')add(e,'beat.cfp',v);
  else if(e.category==='heisman'){
   const c=heismanContext(s,e.playerId);
   if(c?.recentLine&&c.opponent){const moved=numeric(f.previousRank)&&f.previousRank!==f.rank,family=moved?f.rank>f.previousRank&&c.verdict==='positive'&&c.result==='a win'?'editorial.heisman-slip':'editorial.heisman-movement':c.seasonLine?'context.heisman':'context.heisman-recent';add(e,family,{...v,...c,movement:f.previousRank>f.rank?'up':'down'},{role:'analyst'});}
   else{if(!Number.isInteger(f.rank)||f.rank<1||f.rank>5||!f.playerName||!f.school)continue;v.rankChange=numeric(f.previousRank)&&f.previousRank!==f.rank?`moves ${f.previousRank>f.rank?'up':'down'} from No. ${f.previousRank} to No. ${f.rank}`:`is No. ${f.rank}`;add(e,'beat.heisman',v);}
  }else if(e.category==='lines'){
   const g=games.get(f.gameId);if(!g||final(g))continue;Object.assign(v,{home:g.homeName??teams.get(g.home)?.name,away:g.awayName??teams.get(g.away)?.name});add(e,f.pickem?'model.pickem':'model.line',v,{role:'model'});
  }else if(e.category==='holiday'){
   const own=teams.get(s.userTeamIndex),seed=hash(e.id+'|calendar-voice'),roster=s.players.filter(p=>p.team===own?.id&&p.injury==='Uninjured').sort((a,b)=>a.id.localeCompare(b.id)),role=seed%4===0?'official':seed%4===1&&roster.length?'player':'fan',person=role==='player'?roster[seed%roster.length]:null;
   add(e,f.calendarKey?'calendar.'+f.calendarKey:'fan.holiday',v,{role,team:own,player:person,important:!!f.calendarKey});
  }
  else if(e.category==='analytics'){
   let family;
   if(f.metric==='rushingYardsPerGame'){v.rushingYardsPerGame=f.rushingYardsPerGame.toFixed(1);v.rankPhrase=(f.nationalRankTied?'tied for No. ':'No. ')+f.nationalRank;family=ranked(f.pollRank)&&ranked(f.previousPollRank)&&f.pollRank<f.previousPollRank?'analyst.rushing_poll':'analyst.rushing';}
   else if(f.metric==='thirdDownConversionRate'){v.percent=(f.rate*100).toFixed(1);family=f.rate<=.3?'analyst.third_low':'analyst.third_high';}
   else if(f.metric==='redZoneTouchdownRate'){v.percent=(f.rate*100).toFixed(1);family=f.rate<=.45?'analyst.red_low':'analyst.red_high';}
   else if(f.metric==='turnoverMargin')family=f.margin>=0?'analyst.turnover_plus':'analyst.turnover_minus';
   else if(f.kind==='season_largest_winning_margin'){v.score=f.winnerScore+'–'+f.loserScore;family='analyst.margin';}
   if(family){const parent=add(e,family,v,{role:'analyst',persona:['thirdDownConversionRate','redZoneTouchdownRate'].includes(f.metric)?1:f.metric==='turnoverMargin'?2:0});
    if(parent&&family==='analyst.rushing_poll'&&t&&s.games.some(g=>final(g)&&g.week===s.season.CurrentWeek-1&&((g.home===t.id&&g.homeScore>g.awayScore)||(g.away===t.id&&g.awayScore>g.homeScore))))add(e,'fan.poll_reply',{...v,analystHandle:parent.handle},{role:'fan',team:t,suffix:'reply',priority:e.priority+1,replyTo:parent.id});
   }
  }
 }
 const dialogueEventOrder=s.reconstructed?[]:addConversations({s,events,stories,byId,teams,players,games,registry,add,teamValues,perfContext,edition,schedule:sameWeek?.dialogueEventOrder??[],published:sameStories});
 // Dependent replies only exist alongside the actual generated parent.
 return {stories:stories.filter(p=>!p.replyTo||byId.has(p.replyTo)),authors:registry.entries,dialogueEventOrder};
}
