"""Reproduce the reviewed editorial proposal; inputs stay outside distribution.

Usage: python Tools/build_editorial011_content.py proposal.zip original-catalog.json
Only the generated runtime pack and decision summary belong in source control.
"""
import collections
import hashlib
import json
from pathlib import Path
import re
import sys
import zipfile

ROOT = Path(__file__).resolve().parents[1]
proposal = json.loads(zipfile.ZipFile(sys.argv[1]).read('all-edits.json'))
raw = Path(sys.argv[2]).read_bytes()
assert hashlib.sha256(raw).hexdigest() == proposal['source']['sha256']
catalog = json.loads(raw)['families']
by_id = {t['id']: (family, t) for family, ts in catalog.items() for t in ts}
overrides, decisions = [], []
slots = lambda s: collections.Counter(re.findall(r'\{([A-Za-z][A-Za-z0-9]*)\}', s))
humor = re.compile(r'dignity|spare me|souvenir|logos|group chat|receipt|apolog|insufferable|unbearable|petty|pettiness|mute|keyboard|rent.free|cope|bragging|delusion|no chill|ranking.*comfort', re.I)
for edit in proposal['edits']:
    family, original = by_id[edit['id']]
    assert family == edit['family'] and original['body'] == edit['originalBody']
    assert slots(original['body']) == slots(edit['body'])
    body, reason = edit['body'], 'accepted'
    # Keep the original authored voice where a general shortening would change
    # its promise to the selector. New variants below carry explicit voice tags.
    if original.get('style') or original.get('voices'):
        body, reason = original['body'], 'retained authored voice/angle'
    if humor.search(original['body']) and not humor.search(edit['body']):
        body, reason = original['body'], 'retained stronger humor/personality'
    if family.startswith(('casual.', 'archive.', 'history.real', 'tradition.')):
        body, reason = original['body'], 'retained verified historical/casual wording'
    if family in ('editorial.heisman-slip','ending.player.senior','ending.player.senior-perfect','draft.irrelevant','draft.undrafted','review.retention-watch'):
        body, reason = original['body'], 'retained meaningful honor, status or uncertainty detail'
    if re.search(r'\b(model|projection|simulated)\b',original['body'],re.I) and not re.search(r'\b(model|projection|simulated)\b',body,re.I) and not family.startswith('recruit.game.'):
        body, reason = original['body'], 'retained opinion or mod-result provenance'
    if 'transfer' in original['body'] and ('formerSchool' in original['body'] or 'originDetail' in original['body']):
        body, reason = original['body'], 'retained explicit transfer origin'
    if 'Coaches poll' in original['body'] and 'Coaches poll' not in body:
        body, reason = original['body'], 'retained explicit poll identity'
    chronology = re.compile(r'\b(today|tonight|yesterday|tomorrow|Monday|Tuesday|Wednesday|Thursday|Friday|Saturday|Sunday)\b', re.I)
    if {m.group().lower() for m in chronology.finditer(body)} - {m.group().lower() for m in chronology.finditer(original['body'])}:
        body, reason = original['body'], 'retained supported timing; no added day/date claim'
    if family == 'beat.performance':
        body = body.replace('{playerName} ({opponent})', '{playerName} vs. {opponent}')
        body = body.replace('{playerName}: {statLine}. {resultLine} vs. {opponent}.', '{playerName}: {statLine} vs. {opponent}. {resultLine}')
        reason = 'reviewed opponent attribution and complete result sentence'
    if family == 'analysis.team.fourth-quarter-losses':
        # A loss after a Q3 lead can finish in overtime, with any clock setting.
        body, reason = original['body'], 'retained supported lead-after-three timing'
    if family == 'recruiting.catchup.commit':
        body, reason = original['body'], 'retained observation interval, not commitment date'
    if family in ('context.disruption', 'context.ball-security'):
        # Spell out denominators on EVERY average, including unchanged originals.
        body = body.replace('{intpg} INT', '{intpg} INT/G').replace('INT/G/G', 'INT/G')
        body = body.replace('{frpg} FR', '{frpg} FR/G').replace('FR/G/G', 'FR/G')
        body = body.replace('{lostpg} lost fum.', '{lostpg} fumbles lost/G').replace('lost/G/G', 'lost/G')
        body = body.replace('{rate}% INT rate', '{rate}% of pass attempts intercepted').replace('INT rate {rate}%', '{rate}% of pass attempts intercepted').replace('{rate}% picks', '{rate}% of pass attempts intercepted')
        reason = 'explicit per-game and pass-attempt denominators'
    if family == 'context.passing':
        # Passing share is a share of recorded pass/rush attempts, not all plays.
        body, reason = original['body'], 'retained recorded-attempt denominator'
    decisions.append({'id': edit['id'], 'family': family, 'decision': reason, 'changed': body != original['body']})
    if body != original['body']:
        overrides.append({'id': edit['id'], 'family': family, 'originalBody': original['body'], 'body': body})

# Supplemental corrections also cover proposal-unchanged templates.
patches = {e['id']: e for e in overrides}
for family in ('context.disruption', 'context.ball-security'):
    for original in catalog[family]:
        body = patches.get(original['id'], original)['body']
        body = body.replace('{frpg} fumble recoveries a game', '{frpg} fumble recoveries/G')
        body = body.replace('{intpg} INT', '{intpg} INT/G').replace('INT/G/G', 'INT/G')
        body = body.replace('{lostpg} lost fumbles', '{lostpg} fumbles lost/G')
        if body != original['body']:
            patches[original['id']] = {'id': original['id'], 'family': family, 'originalBody': original['body'], 'body': body}

families = {}
def add(family, lines, *, voices=None, when=None, style=None, fallback=False):
    dest = families.setdefault(family, [])
    for angle, body in lines:
        t = {'id': f'{family}.editorial011.{len(dest)+1}', 'body': body, 'angle': angle}
        if voices: t['voices'] = voices
        if when: t['when'] = when
        if style: t['style'] = style
        if fallback: t['fallback'] = True
        dest.append(t)

# Twenty priorities selected from the 31-edition actual native-view replay.
# Six different constructions each; conditions use already captured facts.
add('narrative.game', [
 ('result', '{score} is the final. {winner} gets the win over {loser}.'),
 ('turning-point', '{turningPoint} That is the backdrop to {winner} beating {loser}, {score}.'),
 ('result', 'Put a win beside {winner} and a loss beside {loser}: {score}.'),
 ('turning-point', 'Start with {winner} over {loser}, {score}; then consider the path there. {turningPoint}'),
 ('outcome', 'For {loser}, the result is a defeat to {winner}. Final score: {score}.'),
 ('outcome', '{winner} takes the result from this matchup. {loser} falls, {score}.')], voices=['wire', 'local', 'cautious'])
add('cycle.signed-spotlight', [
 ('welcome', 'Make room for {tagHandle}. A signed member of our next {nickname} class.'),
 ('pride', 'The name is on the signing list: {tagHandle}. Glad you chose the {nickname}.'),
 ('announcement', 'Our incoming class includes {tagHandle}. Signed with {school}.'),
 ('pride', '{tagHandle}, your next colors are ours. Welcome to the {nickname}.'),
 ('welcome', 'One for the {nickname} family to welcome: {tagHandle} is signed.'),
 ('announcement', 'Introducing another signed {nickname} recruit. {tagHandle}, welcome aboard.')], voices=['announcement','pride'])
add('roster.commit', [
 ('status', '{playerName} has chosen {school}. A commitment from the {starDescription}{prospectType}{position}{originDetail}.{recruitRankDetail}'),
 ('position', 'The {position} recruiting picture at {school} now includes a pledge from {playerName}.{recruitRankDetail}'),
 ('status', 'Committed to {school}: {playerName}. The {starDescription}{prospectType}{position} is an addition to the class.{recruitRankDetail}'),
 ('evaluation', 'A pledge is in for {school}, from {position} {playerName}. How the player develops remains ahead.'),
 ('class', '{school} has another committed name to list. {playerName}, {starDescription}{prospectType}{position}{originDetail}.'),
 ('uncertainty', '{playerName} is committed to {school}. That settles the current pledge; it does not settle a future role.')], voices=['wire','local','development','evidence'])
add('team.win', [
 ('announcement', 'FINAL | {school} beats {opponent}, {score}.'),
 ('pride', 'A win to take with us. {nickname} over {opponent}, {score}.'),
 ('pride', 'This one goes in our column. {school} {score} over {opponent}.'),
 ('announcement', '{opponent} matchup: complete. Result: a {score} {nickname} victory.'),
 ('pride', 'Enjoy this one, {nickname}. We beat {opponent}, {score}.'),
 ('announcement', 'The final belongs to {school}: {score} against {opponent}.')], voices=['announcement','pride'])
add('world.award', [
 ('recognition', 'Week {awardWeek} honors go to {school}: {playerName} is the {award}.'),
 ('recognition', '{award} — {playerName}, {school}. The selection covers Week {awardWeek}.'),
 ('player', '{playerName} adds a weekly honor for {school}, taking the {award} for Week {awardWeek}.'),
 ('recognition', 'The Week {awardWeek} {award} selection is {school}\'s {playerName}.'),
 ('player', 'A Week {awardWeek} award for {playerName}. The {school} player is named {award}.'),
 ('recognition', 'Honored for Week {awardWeek}: {playerName} of {school}, {award}.')], voices=['wire','local','development'])
add('cycle.recruit-fit', [
 ('development', 'Signed with {school}, {playerName} is part of the next roster. A signature is the start of that evaluation.'),
 ('status', 'Among the incoming names at {school}: {starDetail}{playerName}, a signed recruit.'),
 ('uncertainty', 'The signing is confirmed for {playerName} at {school}. An eventual role is a separate question.'),
 ('roster', '{school}\'s next roster will include signed recruit {playerName}. Keep the arrival and the projection separate.'),
 ('evaluation', '{starDetail}{playerName} has signed with {school}. There is a player to evaluate beyond the recruiting label.'),
 ('status', 'One name for the incoming-class list is settled: {playerName}, signed with {school}.')], voices=['development','evidence','wire'])
add('team.narrow_win', [
 ('pride', 'Exhale, {nickname}. {score} over {opponent}.'),
 ('announcement', 'A close final, a {school} win: {score} against {opponent}.'),
 ('pride', 'Every win counts. This {score} one against {opponent} counts for the {nickname}.'),
 ('announcement', '{school} finishes on the winning side of a tight one with {opponent}. Final: {score}.'),
 ('pride', 'Take the W with us, {nickname}. {opponent} falls, {score}.'),
 ('announcement', 'The margin was small. The result is a {score} {school} victory over {opponent}.')], voices=['announcement','pride'])
add('cycle.team-welcome', [
 ('welcome', '{tagHandle} has signed. {nickname}, give our newest addition a welcome.'),
 ('pride', 'Your name. Our colors. {tagHandle} is a signed {nickname} recruit.'),
 ('announcement', 'Signing confirmed: {tagHandle} joins the incoming class at {school}.'),
 ('welcome', 'A new name to cheer for, {nickname}. Welcome to {school}, {tagHandle}.'),
 ('announcement', 'Please welcome {tagHandle}, signed with {school}.'),
 ('pride', 'Glad the choice was {school}. {tagHandle}, welcome to the {nickname} family.')], voices=['announcement','pride'])
add('draft.pick', [
 ('selection', '{playerName} is headed to the {nflTeam}. {position}, {school}; pick {draftPick} overall, Round {draftRound}.'),
 ('selection', 'The {nflTeam}\' selection at No. {draftPick}: {school} {position} {playerName}. Round {draftRound}.'),
 ('selection', 'No. {draftPick} overall goes to {playerName}, {position} from {school}. Selected by the {nflTeam} in Round {draftRound}.'),
 ('player', 'From {school} to the {nflTeam}: {position} {playerName}. Draft slot: {draftPick} overall, Round {draftRound}.'),
 ('selection', 'Round {draftRound} selection for the {nflTeam}: {playerName} of {school}, {position}, No. {draftPick} overall.'),
 ('player', '{school} has {position} {playerName} selected. The {nflTeam} use overall pick {draftPick}, Round {draftRound}.')], voices=['wire','announcement'])
add('preseason.honor.beat', [
 ('recognition', 'Before a game is played, {school}\'s {playerName} has a {award} selection.'),
 ('expectation', '{award} recognition puts an early expectation beside {playerName} at {school}.'),
 ('recognition', 'Preseason selection: {playerName}, {school}, {award}.'),
 ('expectation', 'There is preseason recognition for {playerName}. The {school} player is a {award} selection; results are still to come.'),
 ('player', '{school} opens with {playerName} among the {award} selections.'),
 ('expectation', 'An early distinction for {playerName} at {school}: {award}. It is an expectation to play toward.')], voices=['wire','local','cautious'])
add('history.team-counter', [
 ('milestone', 'Program total updated: {school} now has its {ordinal} {achievement}.'),
 ('milestone', 'The latest entry is the {ordinal} {achievement} in {school}\'s history.'),
 ('history', 'Another entry in the {school} record: its {ordinal} {achievement}.'),
 ('history', '{school} reaches a program milestone, recording its {ordinal} {achievement}.'),
 ('milestone', 'For the program history at {school}, count this as the {ordinal} {achievement}.'),
 ('history', 'Add the {ordinal} {achievement} to {school}\'s all-time tally.')], voices=['wire','local'])
add('team.rivalry_win', [
 ('pride', 'Keep this score handy, {nickname}: {school} {winnerScore}, {opponent} {loserScore}.'),
 ('pride', 'A rivalry win for our side. {school} {winnerScore}–{loserScore} {opponent}.'),
 ('announcement', 'Rivalry final | {school} {winnerScore}, {opponent} {loserScore}.'),
 ('pride', 'This is a score worth remembering. {school} {winnerScore}, {opponent} {loserScore}.'),
 ('pride', 'For everyone who wears our colors: a win over {opponent}. {school} takes it, {winnerScore}–{loserScore}.'),
 ('announcement', '{opponent} matchup settled in favor of {school}, {winnerScore}–{loserScore}.')], voices=['announcement','pride'])
add('team.poll_up', [
 ('movement', 'A climb of the poll for {school}: No. {previousRank} to No. {rank} in the {pollName}.'),
 ('announcement', '{pollName} update | {nickname} No. {rank}, up from No. {previousRank}.'),
 ('pride', 'Higher on the list. {school} moves from No. {previousRank} to No. {rank} in the {pollName}.'),
 ('movement', 'The new {pollName} has {school} at No. {rank}. Last listing: No. {previousRank}.'),
 ('pride', 'Keep climbing, {nickname}. No. {rank} in the {pollName}, previously No. {previousRank}.'),
 ('announcement', 'No. {previousRank} becomes No. {rank} for {school} in the {pollName}.')], voices=['announcement','pride'])
add('transactions.coach-deal', [
 ('continuity', 'A new contract keeps {coachName} at {school} as {role}.{term}'),
 ('contract', '{role} {coachName} has a new deal with {school}.{term}'),
 ('continuity', '{school} retains its {role}. {coachName} stays under a new contract.{term}'),
 ('contract', 'Contract update at {school}: a new agreement for {role} {coachName}.{term}'),
 ('continuity', 'The {role} position at {school} remains with {coachName}, on a new deal.{term}'),
 ('contract', 'New terms, same school for {coachName}. {school} retains the {role}.{term}')], voices=['wire','local'])
add('launch.kickoff', [
 ('pride', 'Our colors. A new season. Ready for {nickname} football.'),
 ('announcement', '{school} football begins a new season.'),
 ('pride', 'However far you follow us from: welcome back to {nickname} football.'),
 ('pride', 'There is a season ahead of us, {nickname}. Proud to start it together.'),
 ('announcement', 'The next season of {school} football is here.'),
 ('pride', 'Time to wear those colors again. {nickname} football is back.')], voices=['announcement','pride'])
add('portal.unsigned-local', [
 ('status', 'No commitment is recorded for {position} {playerName}. The high school prospect is No. {nationalRecruitRank} nationally.'),
 ('evaluation', '{playerName} remains an uncommitted high school prospect at {position}. National rank: {nationalRecruitRank}; a ranking is only part of an evaluation.'),
 ('status', 'Still available on the high school recruiting board: {playerName}, {position}, No. {nationalRecruitRank} nationally.'),
 ('uncertainty', '{position} {playerName} has yet to commit. That status does not identify a favorite. National ranking: {nationalRecruitRank}.'),
 ('status', 'The listing for {playerName} still reads uncommitted. High school {position}, national No. {nationalRecruitRank}.'),
 ('evaluation', 'One unsigned high school name at {position}: {playerName}. No. {nationalRecruitRank} nationally, with a decision still unrecorded.')], voices=['wire','local','development','evidence'])
add('injury.report', [
 ('availability', '{playerName} is on {school}\'s injury report.{injuryDetail} {timetable}'),
 ('timetable', 'The recorded outlook for {school}\'s {playerName}: {timetable}{injuryDetail}'),
 ('availability', '{school} availability: an injury entry for {playerName}.{injuryDetail} {timetable}'),
 ('uncertainty', 'An injury is recorded for {playerName} at {school}.{injuryDetail} {timetable}'),
 ('availability', 'Player update — {playerName}, {school}.{injuryDetail} {timetable}'),
 ('timetable', '{school}\'s {playerName} has this listed injury outlook: {timetable}{injuryDetail}')], voices=['wire','local','cautious'])
add('transactions.signed', [
 ('signing', 'A signature is in for {school}: {starDetail}{playerName}.{nilDetail}'),
 ('signing', '{playerName} has signed with {school}.{nilDetail}'),
 ('status', 'Signed status confirmed for {playerName} at {school}.{nilDetail}'),
 ('class', '{school}\'s signed class includes {starDetail}{playerName}.{nilDetail}'),
 ('signing', 'The signing list at {school} now includes {playerName}.{nilDetail}'),
 ('status', '{starDetail}{playerName}: signed with {school}.{nilDetail}')], voices=['wire','local'])
add('buzz.team-pregame', [
 ('pride', '{eventLabel}: a matchup with {opponent}. Ready to follow the {nickname}.'),
 ('announcement', '{school} faces {opponent} in the {eventLabel}.'),
 ('pride', 'This stage, these colors. {nickname} against {opponent}, {eventLabel}.'),
 ('pride', 'Wherever you are watching, you are part of this. {nickname} vs. {opponent} in the {eventLabel}.'),
 ('announcement', 'The {eventLabel} matchup is {school} and {opponent}.'),
 ('pride', 'The {eventLabel} brings our {nickname} a matchup with {opponent}.')], voices=['announcement','pride'])
add('reply.player.award', [
 ('gratitude', '{parentHandle} An honor I appreciate. Thank you.'),
 ('team', '{parentHandle} Proud of the recognition. Proud to be part of this team.'),
 ('gratitude', '{parentHandle} Seeing my name there means a lot.'),
 ('team', '{parentHandle} Glad to represent our group on that list.'),
 ('gratitude', '{parentHandle} I appreciate the award and everyone supporting me.'),
 ('humility', '{parentHandle} Grateful for the honor. Still plenty for me to improve.')], voices=['measured'], style='measured')
assert len(families) == 20 and sum(map(len, families.values())) == 120

# Thirty focused exceptions: real short factual paths and parent-engaged fan
# reactions. These are within the SAME 150 cap, not an additional expansion.
add('beat.performance', [('production','{playerName}: {shortStatLine} vs. {opponent}.'),('outcome','{resultLine} {playerName}: {shortStatLine}.'),('production','Against {opponent}, {playerName} recorded {shortStatLine}.')], when=['shortStatLine'], fallback=True)
add('context.passing', [('efficiency','{school}: {ypa} yards per pass attempt; {tdpg} passing TD/G, {intpg} INT/G.'),('volume','Passing at {school}: {ypg} yards/G. Throws: {share}% of recorded pass/rush attempts.'),('production','{ypg} passing yards/G for {school}, at {ypa} yards/attempt.')], fallback=True)
add('context.ball-security', [('turnovers','{school}: {givepg} giveaways/G; {rate}% of pass attempts intercepted.'),('turnovers','Per game at {school}: {intpg} INT thrown and {lostpg} fumbles lost.'),('turnovers','{givepg} giveaways a game for {school}. {lostpg} are fumbles lost/G.')], fallback=True)
add('context.disruption', [('takeaways','{school}: {takepg} takeaways/G, including {intpg} INT/G and {frpg} fumble recoveries/G.'),('pressure','The {school} defense records {sackpg} sacks a game.'),('takeaways','Defensive takeaways per game for {school}: {intpg} interceptions, {frpg} fumble recoveries.')], fallback=True)
add('context.nil', [('allocation','{school} roster NIL total: {total}.'),('allocation','The largest position-group allocation at {school}: {first}, {firstMoney} ({firstShare}%).'),('allocation','{school} allocates {secondMoney} to the {second} group, {secondShare}% of roster NIL.')], fallback=True)
add('reply.fan.upset', [('celebration','{score}. I am going to be a deeply annoying {winner} fan about this.'),('pride','The ranking belonged to {loser}. The win belongs to {winner}. I know which one I want.'),('humor','Please respect our privacy while we make {winner} beating {loserLabel} our entire personality.')], voices=['excited','wry','rival'])
add('reply.fan.upset-loss', [('frustration','{score} against {winner}. I do not need a reminder of how famous {loser} is.'),('skepticism','If the logo could win the game, {loser} would not be explaining this loss to {winner}.'),('respect','Credit to {winner}. That does not make being a {loser} fan any less miserable about {score}.')], voices=['skeptical','wry'])
add('context.upset-rival', [('pettiness','I have no connection to {winner} except the enormous public service of beating {loserLabel}.'),('humor','{loserLabel} lost to {winner}. Suddenly I am very interested in checking on our rivals.'),('pettiness','{winner}, thank you. {loser}, I assume we will all be discussing {score} very calmly.')], voices=['rival','wry'])
add('reply.player.offer', [('gratitude','{parentHandle} Thank you for the offer and the opportunity.'),('gratitude','{parentHandle} Grateful to be considered. I appreciate it.'),('respect','{parentHandle} Thank you. It means a lot to have that opportunity.')], style='measured', voices=['measured'])
add('matchup.receiver-secondary-recap', [('receiving','{receiver} vs. {opponent}: {actualCatches} catches, {actualYards} receiving yards.'),('defense','{opponent}\'s {defender} recorded {actualBreakups} PBU and {actualInts} INT against {school}. No individual coverage assignment implied.'),('receiving','Receiving production for {receiver}: {actualYards} yards on {actualCatches} catches against {opponent}.')], fallback=True)
# The matchup slots are checked against the existing family before writing.
valid_slots = {f: set().union(*(set(slots(t['body'])) for t in ts)) for f, ts in catalog.items()}
for keys in valid_slots.values(): keys.update(['school','nickname'])  # teamValues(), always provided for these team families
valid_slots['beat.performance'].add('shortStatLine')
for f, ts in families.items():
    for t in ts:
        assert set(slots(t['body'])) <= valid_slots[f], (f, set(slots(t['body'])) - valid_slots[f])
assert sum(map(len, families.values())) == 150
pack = {'format':1, 'revision':'cot-0.1.11-editorial-1', 'sourceSha256':proposal['source']['sha256'], 'overrides':list(patches.values()), 'families':families}
target = ROOT/'Modules/DynastyEnhancer/Content/Runtime/editorial011-pack.json'
target.write_text(json.dumps(pack,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
summary = {'sourceSha256':pack['sourceSha256'],'proposalEdits':len(decisions),'changedExisting':len(patches),'newVariants':150,'primaryFamilies':list(families)[:20],'decisions':decisions}
(ROOT/'Build/Editorial011/editorial-decisions.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps({k:v for k,v in summary.items() if k!='decisions'}))
