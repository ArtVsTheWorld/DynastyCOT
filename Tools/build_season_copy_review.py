"""Finite reviewed corrections, retaining template IDs, guards and original proposal provenance."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
pack=json.loads((ROOT/'Build/RC016/content-before.json').read_text(encoding='utf-8'))
exact={
 'analysis.player.passing-yards-leader.variety.2':"The passing-yardage list has {school}'s {playerName} at the front. He {lead} {field} with {value} {metricLabel}.",
 'analysis.player.passing-yards-leader.variety.6':"At {school}, the name atop the passing list is {playerName}. With {value} {metricLabel}, he {lead} {field}.",
 'analysis.player.passing-yards-leader.variety.9':"If you're checking passing leaders in {field}, start with {playerName} at {school}: {value} {metricLabel}.",
 'analysis.player.passing-td-leader.variety.5':"A passing touchdown list with {playerName} at the front: {value} {metricLabel} for {school}. He {lead} {field}.",
 'analysis.player.passing-td-leader.variety.8':"Passing touchdowns keep {playerName} of {school} on this leaderboard: he {lead} {field} with {value} {metricLabel}.",
 'analysis.player.passing-td-leader.variety.9':"For {school}'s {playerName}, the total is {value} {metricLabel}. He {lead} {field}.",
 'analysis.player.rushing-yards-leader.variety.2':"The rushing leaderboard in {field} has {school}'s {playerName} out front: {value} {metricLabel}.",
 'analysis.player.rushing-yards-leader.variety.9':"Up front in {field}'s rushing totals: {playerName} from {school}, {value} {metricLabel}.",
 'analysis.player.receiving-yards-leader.variety.2':"Receiving yards: {playerName} of {school} {lead} {field} with {value} {metricLabel}.",
 'analysis.player.receiving-yards-leader.variety.10':"Atop the {field} receiving list is {playerName} from {school}. The total: {value} {metricLabel}.",
 'analysis.player.sacks-leader.variety.4':"Sack leaders: {playerName} from {school} {lead} {field} with {value} {metricLabel}.",
 'analysis.player.sacks-leader.variety.5':"Put {playerName} on the pass-rush watch list. With {value} {metricLabel} for {school}, he {lead} {field}.",
 'analysis.player.sacks-leader.variety.6':"On the sack leaderboard: {school}'s {playerName}, {value} {metricLabel}. He {lead} {field}.",
 'analysis.player.sacks-leader.variety.8':"The pass rush has a name to know in {field}: {school}'s {playerName}, {value} {metricLabel}.",
 'analysis.player.sacks-leader.variety.10':"In {field}, {playerName} is setting the sack pace for {school}: {value} {metricLabel}.",
 'analysis.player.interceptions-leader.variety.2':"The interception leaders in {field} include a name on top: {school}'s {playerName}, {value} {metricLabel}.",
 'analysis.player.interceptions-leader.variety.10':"Among the turnover makers, {school}'s {playerName} {lead} {field} with {value} {metricLabel}.",
 'buzz.pick-2.compact.2':"I'm picking {pick}. {lineContext}",
 'carry.training.eventcopy.10':"{position} {playerName} improved {area} at {school} during {period}.",
 'roster.commit.cot104.5f65e3197221cb55':"{playerName} has chosen {school}. A commitment gives this class a name to build around; signing is still ahead.",
 'roster.major.cot104.2f83d5d005445672':"5-star {prospectType}{position} {playerName} has chosen {school}. That's a major commitment for this class.",
 'review.title-debate.retrospective.8':"{winner}, national champion. {score} over {loser} in the title game.",
 'draft.top-ten.eventcopy.9':"TOP 10: The {nflTeam} select {position} {playerName} from {school} at No. {draftPick}.",
}
patches=[]
for family,lines in pack['families'].items():
 for t in lines:
  old=t['body'];new=exact.get(t['id'],old)
  # Remove repeated assertions of certainty; the entry itself is the fact.
  for tail in [' No new destination is confirmed.',' No new destination announced here.',' No new destination announced.',' No new destination yet.',' The Heisman decision is final.']:
   new=new.replace(tail,'')
  if family=='draft.irrelevant':new=new.replace(', the final selection.','.')
  if new!=old:patches.append(dict(id=t['id'],family=family,originalBody=old,body=new))
doc=dict(format=1,revision='season-review-0.1.16',newVariants=0,overrides=patches)
(ROOT/'Modules/DynastyEnhancer/Content/Runtime/season-fixes-pack.json').write_text(json.dumps(doc,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(f'{len(patches)} reviewed corrections; no new variants or posts')
