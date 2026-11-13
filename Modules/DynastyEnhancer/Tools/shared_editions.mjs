// Public published copy can be viewed from another school, never its private
// recruiting snapshot. Original blobs, account IDs and body text stay intact.
import {scopeCatalog,discoveryViews} from './social_discovery.mjs';
import {scopeViews} from './feed_selection.mjs';
import {phaseIndex,comparePublication} from './publication_timeline.mjs';
export function sharedEdition(feed,s){
 const now={phase:s.season.CurrentWeekType,week:s.season.CurrentWeek};
 if(feed.leagueId!==s.leagueId||feed.season!==s.season.CurrentSeasonYear||phaseIndex(feed.phase)<0||comparePublication(feed,now)>0)return null;
 const conference=s.teams.find(t=>t.id===s.userTeamIndex)?.conferenceId;
 const members=new Set((s.conferences??[]).find(c=>c.id===conference)?.teams??[]);
 const stories=feed.stories.map(p=>{const ids=p.teamIds??[];const scopes=(p.scopes??[]).filter(x=>!['team','conference','relevant'].includes(x));if(ids.includes(s.userTeamIndex))scopes.push('team');if(ids.some(id=>members.has(id)))scopes.push('conference');return {...p,userRelevant:false,scopes};});
 const options=scopeCatalog(s),views={...scopeViews(stories,{userTeamIndex:s.userTeamIndex}),...discoveryViews(stories,options)};
 return {...feed,userTeamIndex:s.userTeamIndex,stories,views,scopeOptions:options,posts:views.all.map(id=>stories.find(p=>p.id===id)),contentStatus:'recovered',recoveredFromTeam:feed.userTeamIndex};
}
