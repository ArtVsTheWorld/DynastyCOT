// Authored copy source, kept with editorial checks, not shipped as a helper.
import fs from 'node:fs';
import {perspectiveId} from './content_ids.mjs';
const families={};
function add(family,copy){const rows=[];for(const line of copy.trim().split('\n')){if(line.includes('|'))rows.push(line);else rows[rows.length-1]+='\\n'+line;}families[family]=rows.map((line,i)=>{const [angle,voices,body]=line.split('|');return {id:`${family}.cot104.${i+1}`,angle,voices:voices.split(','),body:body.replace(/\\n/g,'\n')};});}
function append(family,copy){const previous=families[family]??[];add(family,copy);families[family]=[...previous,...families[family].map((t,i)=>({...t,id:`${family}.cot104.extra.${i+1}`}))];}
add('roster.commit',`
class|wire|Committed: {position} {playerName} to {school}.
position|local|A {position} addition for {school}. {playerName} has made his commitment.
decision|cautious|The decision is in for {playerName}: {school}.
class|wire|{school}'s recruiting class now includes {playerName}, a {position}.
decision|local|Following {playerName}'s recruitment? He has chosen {school}.
position|cautious|{playerName} gives {school} another name at {position}. This is a commitment, ahead of signing.
evaluation|development|{starDescription}{position} {playerName} is headed to {school}. The evaluation on file: {recruitRankDetail}
decision|wire|Recruiting update — {playerName} has picked {school}.
class|local|Put {playerName} on the {school} commitment list. Position: {position}.
position|development|For those tracking {school}'s {position} recruiting, {playerName} is committed.
status|cautious|A school choice to report for {playerName}. Committed to {school}; signing is a separate step.
class|wire|{position}: {playerName}. Destination: {school}. Status: committed.
decision|local|The {playerName} recruitment has a destination. His commitment goes to {school}.
position|wire|{school} has a commitment from {playerName} at {position}.
evaluation|development|Start with the prospect: {starDescription}{position} {playerName}{originDetail}. His choice is {school}.
class|local|An addition to the {school} class, rather than a roster debut: {playerName} has committed.
decision|cautious|No need to project a depth-chart role yet. The news is {playerName}'s commitment to {school}.
class|wire|{playerName} → {school}. A commitment at {position}.
position|local|The latest {school} recruiting decision comes at {position}, where {playerName} has committed.
status|cautious|What is confirmed: {school} is {playerName}'s choice. No playing-time promise to attach to that.
evaluation|development|A {starDescription}{position} for the commitment board: {playerName}, bound for {school}.
decision|wire|School selected. {playerName} commits to {school}.
class|local|Add {playerName} to the names following the {school} recruiting path.
status|cautious|{school} gets the commitment. How {playerName} fits on the field will come later.
position|development|At {position}, one recruitment now points to {school}: {playerName}.
decision|wire|{playerName}'s next school choice is {school}. The {position} is committed.
class|local|A new line on the {school} commitment board reads {playerName}, {position}.
evaluation|cautious|{recruitRankDetail} The school decision belongs to {playerName}: {school}.
position|wire|The commitment from {playerName} brings a {position} prospect to {school}'s class.
status|development|The recruitment has reached a commitment. {playerName} has chosen {school}, with the on-field chapter still ahead.
`);
add('beat.commit',`
decision|wire|Commitment confirmed: {playerName} has chosen {school}.
position|local|{school}'s latest commitment comes at {position}: {playerName}.
status|cautious|A commitment for {school}, with signing still a separate matter. The prospect is {playerName}.
decision|wire|{playerName} has made a school choice. It is {school}.
class|local|On the {school} commitment board, add {position} {playerName}.
evaluation|development|{position} recruiting update for {school}: {playerName} is committed.{recruitRankDetail}
decision|wire|The destination for {playerName} is {school}. Status: committed.
class|local|A class addition to log: {playerName}, {position}, {school}.
status|cautious|{playerName} is a commitment to follow for {school}. No role on the depth chart has been established by that decision.
decision|wire|School choice — {playerName}: {school}.
position|local|Another recruitment on the {position} board has a decision. {playerName} picks {school}.
evaluation|development|The player is {playerName}; the position is {position}. His commitment goes to {school}.{recruitRankDetail}
class|wire|{school} adds a commitment from {playerName}.
status|cautious|{playerName}'s commitment is the confirmed development here. He has chosen {school}.
decision|local|For anyone following {playerName}, the school decision is now {school}.
position|wire|At {position}: {playerName} commits to {school}.
class|local|A fresh name on {school}'s recruiting list. {playerName} is committed.
evaluation|cautious|The commitment tells us the school, not the future role. {playerName} has picked {school}.
decision|wire|{school} is the choice announced by {playerName}.
class|local|Put a commitment beside {playerName}'s name on the {school} board.
position|development|Tracking the {position} group for {school}? {playerName} has committed.
status|cautious|The decision is a commitment to {school}. That is the update on {playerName}.
decision|wire|Recruiting decision filed: {playerName} to {school}.
class|local|{playerName} joins the {school} commitment list as a {position}.
evaluation|development|A prospect decision, before an on-field evaluation at the next level: {playerName} chooses {school}.
decision|wire|One recruitment resolved to a school choice: {playerName} has selected {school}.
position|local|{school} has a new {position} commitment in {playerName}.
status|cautious|For now, the label is committed. {playerName}'s chosen program is {school}.
class|wire|The {school} class has a commitment from {position} {playerName}.
decision|local|{playerName}'s recruiting path now points to {school}.{recruitRankDetail}
`);
add('world.upset',`
result|wire|Upset final: {winnerLabel} defeats {loserLabel}, {score}.
expectation|evidence|The ranking labels favored a different story. The result: {winnerLabel} over {loserLabel}, {score}.
winner|local|Give the winner the headline. {winnerLabel} beats {loserLabel}, {score}.
loser|cautious|A loss for {loserLabel}: {winnerLabel} wins {score}. The next poll is still ahead.
result|wire|{score}, and the upset belongs to {winnerLabel}. {loserLabel} takes the loss.
expectation|race|Public rank and final score point in different directions: {winnerLabel} beats {loserLabel}, {score}.
winner|film|Start the discussion with what happened on the field: {winnerLabel}, {score} winners against {loserLabel}.
loser|evidence|{loserLabel} could not turn its ranking into a win. {winnerLabel} takes it, {score}.
result|wire|Final score from the upset: {winnerLabel} {score} over {loserLabel}.
poll|race|Poll implications can wait for publication. Today's verified result is {winnerLabel} beating {loserLabel}, {score}.
winner|local|The lower-ranked side gets this one. {winnerLabel} defeats {loserLabel}, {score}.
expectation|cautious|Call it an upset on the ranking evidence. {winnerLabel} wins {score} over {loserLabel}.
result|wire|Winner: {winnerLabel}. Opponent: {loserLabel}. Final: {score}.
poll|evidence|One result should not become an invented new ranking. {winnerLabel} has beaten {loserLabel}, {score}.
winner|local|{winnerLabel} gets the result that the ranking gap did not suggest: {score} against {loserLabel}.
loser|cautious|The score is settled; the season judgment is not. {loserLabel} loses {score} to {winnerLabel}.
result|wire|{loserLabel} falls. {winnerLabel} takes the upset, {score}.
expectation|schedule|A reminder that a ranked opponent still has to win the game: {winnerLabel} over {loserLabel}, {score}.
winner|film|Whatever the pregame label, the win goes to {winnerLabel}. {loserLabel} beaten {score}.
poll|race|A ranked-team loss to carry into the next poll discussion: {winnerLabel} beats {loserLabel}, {score}.
result|wire|Put {score} in the final column. {winnerLabel} defeats {loserLabel}.
loser|local|For {loserLabel}, this week's result is an upset loss. {winnerLabel} wins, {score}.
expectation|evidence|The upset is in the result, not a forecast: {winnerLabel} {score} over {loserLabel}.
winner|wire|An upset victory for {winnerLabel}, with {loserLabel} on the other side of a {score} final.
poll|cautious|No poll movement to report yet. There is a result: {winnerLabel} has beaten {loserLabel}, {score}.
expectation|race|Ranked status did not decide this matchup. {winnerLabel} did, {score} over {loserLabel}.
winner|local|The headline starts with {winnerLabel}, ends with a win, and reads {score} against {loserLabel}.
loser|evidence|A ranking is not protection from a loss. {loserLabel} is beaten {score} by {winnerLabel}.
result|wire|Confirmed upset — {winnerLabel} wins; {loserLabel} loses. Score: {score}.
expectation|cautious|There is an upset to assess without rewriting either team's whole season: {winnerLabel} over {loserLabel}, {score}.
`);
add('beat.performance',`
production|wire|{playerName}: {statLine}. Opponent: {opponent}. {resultLine}
outcome|cautious|Keep the individual line beside the team result. {playerName} had {statLine} against {opponent}. {resultLine}
player|local|For {school}, {playerName}'s contribution against {opponent} was {statLine}.
production|film|The work to evaluate from {playerName}: {statLine} versus {opponent}.
outcome|evidence|Two parts to this recap: {playerName} posted {statLine} against {opponent}. {resultLine}
player|development|A player line from the {school} game: {playerName}, {statLine}, against {opponent}.
production|wire|Against {opponent}, {playerName} finished with {statLine}.
outcome|cautious|Production and outcome need their own columns. {playerName}: {statLine} vs. {opponent}. {resultLine}
player|local|{school}'s box score includes {statLine} from {playerName} against {opponent}.
production|film|Start with the recorded production: {statLine} for {playerName} against {opponent}.
outcome|evidence|The numbers belong to {playerName}: {statLine}, against {opponent}. {resultLine}
player|development|{playerName} against {opponent} is worth a look on the {school} sheet: {statLine}.
production|wire|Game line — {playerName}, {school}: {statLine} vs. {opponent}.
outcome|cautious|That is {statLine} for {playerName} against {opponent}, with a separate team story. {resultLine}
player|local|One contribution to the {school} total came from {playerName}: {statLine} against {opponent}.
production|film|Evaluate the performance on this evidence: {playerName}, {statLine}, opponent {opponent}.
outcome|evidence|Before drawing a team conclusion from one player: {playerName} had {statLine} against {opponent}. {resultLine}
player|development|On the player side of the {school} recap, {playerName} records {statLine} versus {opponent}.
production|wire|Recorded against {opponent}: {statLine}. Player: {playerName}.
outcome|cautious|{resultLine} Individually, {playerName} delivered {statLine} against {opponent}.
player|local|A name from the {school} performance report is {playerName}, with {statLine} against {opponent}.
production|film|The {opponent} matchup left this line for {playerName}: {statLine}.
outcome|evidence|Read these together: {statLine} for {playerName} against {opponent}; {resultLine}
player|development|For the individual review: {school}'s {playerName}, {statLine} vs. {opponent}.
production|wire|{statLine} — the final individual line for {playerName} against {opponent}.
outcome|cautious|An individual performance does not tell the entire game story. {playerName} had {statLine} vs. {opponent}. {resultLine}
player|local|In {school}'s game against {opponent}, the entry for {playerName} reads {statLine}.
production|film|The performance evidence is specific: {playerName} produced {statLine} against {opponent}.
outcome|evidence|{playerName}'s production: {statLine}. Context: {opponent}. Team result: {resultLine}
player|development|From the {opponent} game, a {school} player to review is {playerName}: {statLine}.
`);
add('team.upcoming',`
schedule|announcement|On our schedule: {opponent}. {hashtag}
pride|pride|Stand with the {nickname} for the matchup with {opponent}. {hashtag}
schedule|announcement|The next opponent is {opponent}.
pride|pride|Your {nickname}. The challenge: {opponent}. {hashtag}
schedule|announcement|Matchup announcement — {nickname} vs. {opponent}.
pride|pride|Bring your {nickname} pride for {opponent}. {hashtag}
schedule|announcement|Looking ahead to the game against {opponent}. {hashtag}
pride|pride|All eyes on our matchup with {opponent}. Represent the {nickname}.
schedule|announcement|Next on the card\n{nickname} — {opponent}
pride|pride|With the {nickname}, every step. {opponent} is next. {hashtag}
schedule|announcement|The opponent on deck: {opponent}. {hashtag}
pride|pride|Backing the {nickname} against {opponent}.
schedule|announcement|Upcoming game: {nickname} vs. {opponent}. {hashtag}
pride|pride|Our colors. Our {nickname}. Next challenge, {opponent}.
schedule|announcement|A look at what's next: {opponent}.
pride|pride|Meet the next challenge together, {nickname}. We face {opponent}. {hashtag}
schedule|announcement|Preparing for the matchup: {nickname} / {opponent}.
pride|pride|However you follow along, back the {nickname} against {opponent}. {hashtag}
schedule|announcement|Football ahead. Opponent: {opponent}.
pride|pride|Proud to be the {nickname}. Looking toward {opponent}.
schedule|announcement|The schedule turns to {opponent}. {hashtag}
pride|pride|For everyone behind the {nickname}: our next matchup is {opponent}.
schedule|announcement|{opponent} is the next assignment for the {nickname}.
pride|pride|Let your {nickname} colors show. Up next, {opponent}.
schedule|announcement|Opponent preview begins with {opponent}. {hashtag}
pride|pride|One program behind the {nickname}. Next game, {opponent}.
schedule|announcement|Coming up on the football schedule — {opponent}.
pride|pride|The {nickname} take on {opponent}. Be with us. {hashtag}
schedule|announcement|Next matchup listed: {opponent} vs. our {nickname}.
pride|pride|Support travels with the {nickname}, wherever you watch. Next: {opponent}.
`);
add('world.award',`
recognition|wire|Week {awardWeek} {award}: {playerName}, {school}.
player|local|An honor for {school}'s {playerName}. He is the {award} for Week {awardWeek}.
scope|cautious|This is a weekly honor: {playerName} of {school}, {award}, Week {awardWeek}.
recognition|wire|{award} goes to {playerName} for Week {awardWeek}.
player|development|Put {playerName} on the Week {awardWeek} honors list. The {school} player is the {award}.
scope|evidence|The award is for Week {awardWeek}. Its recipient: {school}'s {playerName}, named {award}.
recognition|wire|Award announced — {playerName} is Week {awardWeek}'s {award}.
player|local|{school} has a Week {awardWeek} honoree in {playerName}: {award}.
scope|cautious|A Week {awardWeek} distinction, rather than a season award: {playerName} earns {award}.
recognition|wire|The Week {awardWeek} selection for {award} is {playerName}.
player|development|Individual recognition for {playerName} at {school}, named {award} in Week {awardWeek}.
scope|evidence|Record the category and the week: {award}, Week {awardWeek}. Winner: {playerName}.
recognition|wire|Honors update: {playerName}, {award}, Week {awardWeek}.
player|local|A place on this week's honors board for {school}. {playerName} is the Week {awardWeek} {award}.
scope|cautious|For the weekly record: {playerName} has received {award} for Week {awardWeek}.
recognition|wire|Week {awardWeek} award recipient\n{playerName} — {award}
player|development|Recognition reaches {school}'s {playerName}: {award} in Week {awardWeek}.
scope|evidence|The verified honor is {award}, awarded to {playerName} for Week {awardWeek}.
recognition|wire|Named to the weekly honors: {playerName}, Week {awardWeek} {award}.
player|local|From {school}, {playerName} takes the Week {awardWeek} {award} honor.
scope|cautious|Today's award note concerns Week {awardWeek}: {playerName} is the {award}.
recognition|wire|{playerName} receives the {award} distinction for Week {awardWeek}.
player|development|The honors sheet has {playerName} beside {school}: Week {awardWeek}, {award}.
scope|evidence|Weekly award confirmation — {award} to {playerName}, Week {awardWeek}.
recognition|wire|Recipient: {playerName}. Honor: {award}. Week: {awardWeek}.
player|local|An individual honor to report from {school}: {playerName}, Week {awardWeek} {award}.
scope|cautious|The Week {awardWeek} award entry belongs to {playerName}, selected as {award}.
recognition|wire|Selected for {award}: {playerName}. The honor covers Week {awardWeek}.
player|development|Week {awardWeek} brings {award} recognition for {school}'s {playerName}.
scope|evidence|Keep this on the Week {awardWeek} honors list: {playerName}, {award}.
`);
add('cycle.portal-target',`
status|wire|{playerName}'s recorded top three includes {school}. No transfer commitment yet.
fit|development|A possible addition to evaluate for {school}: transfer prospect {playerName}, who lists the program in his top three.
uncertainty|cautious|There is interest to report, not a destination. {school} is in {playerName}'s current top three.
status|wire|Portal board: {school} remains one of three leading schools for {playerName}.
fit|local|For the {school} portal watch, keep {playerName} in view. His top three includes the school.
uncertainty|evidence|A top-three listing is the available evidence linking {playerName} and {school}. It is not a commitment.
status|wire|Transfer interest — {playerName}, with {school} in the top three.
fit|development|The recruitment involving {playerName} is still open. {school} is one of his recorded top three options.
uncertainty|cautious|Do not add {playerName} to a new roster yet. {school} is a top-three option, not a confirmed destination.
status|wire|One name on {school}'s portal watch: {playerName}. The program appears in his top three.
fit|local|Monitoring the portal for {school}? {playerName} has the school among his three leaders.
uncertainty|evidence|The confirmed connection is a leading-school list: {playerName} includes {school} in his top three.
status|wire|Three schools lead for {playerName}; {school} is one of them.
fit|development|An open transfer recruitment with a {school} connection: {playerName}'s top three includes the program.
uncertainty|cautious|This remains a recruitment. {playerName} lists {school} in the top three, with no commitment recorded.
status|wire|Current portal interest lists {school} among {playerName}'s three leaders.
fit|local|{school} has a place in the leading group for transfer prospect {playerName}.
uncertainty|evidence|A shortlist can change. Right now, {school} is in {playerName}'s recorded top three.
status|wire|Top-three watch — {playerName}: {school} is included.
fit|development|One potential portal fit has a verified interest signal: {playerName}'s top-three listing of {school}.
uncertainty|cautious|The decision still belongs to {playerName}. His current top three has {school} in it.
status|wire|For {playerName}, the leading three options include {school}.
fit|local|A portal recruitment to track alongside the {school} board: {playerName}, currently listing the program in his top three.
uncertainty|evidence|Interest is the word here. {school} appears in transfer prospect {playerName}'s top three.
status|wire|The transfer shortlist for {playerName} has a {school} entry.
fit|development|Before discussing a role at {school}, there is a recruitment to finish. {playerName} has the program in his top three.
uncertainty|cautious|Nothing signed or committed to report here. The update is {school}'s top-three place with {playerName}.
status|wire|Leading schools for {playerName}: {school} is among the recorded three.
fit|local|{playerName} is a transfer name connected to {school} through his current top-three list.
uncertainty|evidence|The list narrows the schools to follow, not the outcome. {school} is in {playerName}'s top three.
`);
add('roster.interest',`
status|wire|Transfer {position} {playerName}: leading schools are {schools}. No decision yet.
position|development|At {position}, the transfer recruitment for {playerName} currently points toward {schools}.
uncertainty|cautious|The leading group is {schools}. {playerName} has not committed to a new school.
status|wire|Portal shortlist — {playerName}, {position}: {schools}.
origin|local|{playerName}{originDetail} is considering a leading group of {schools}. No commitment is recorded.
uncertainty|evidence|A list of leaders is the update for {playerName}. That list is {schools}; it is not a final choice.
status|wire|{schools} lead in the open transfer recruitment of {playerName}.
position|development|For programs tracking a transfer {position}, {playerName}'s leaders are {schools}.
uncertainty|cautious|There is no destination to announce for {playerName}. The current leading schools are {schools}.
status|wire|Leading group recorded: {schools}. Prospect: {position} {playerName}.
origin|local|A transfer note on {playerName}{originDetail}: {schools} form the leading group.
uncertainty|evidence|Keep the status clear on {playerName}: open recruitment, with {schools} leading.
status|wire|Still recruiting: {position} {playerName}. Schools to watch: {schools}.
position|development|{playerName}'s {position} recruitment has {schools} at the front of the list.
uncertainty|cautious|The next decision is still ahead for {playerName}. His current leaders: {schools}.
status|wire|Interest update for transfer prospect {playerName}: {schools}.
origin|local|In the portal, {playerName}{originDetail} has a leading group that includes {schools}.
uncertainty|evidence|These are the leading options, not a commitment order: {schools}, for {playerName}.
status|wire|The {playerName} watch centers on {schools}, his current leading schools.
position|development|Transfer board, {position}: {playerName}. The recorded leaders are {schools}.
uncertainty|cautious|Wait for a school choice before treating this as a move. {playerName}'s leading group is {schools}.
status|wire|A leading-school list for {playerName}: {schools}. Recruitment remains open.
origin|local|Following {playerName}{originDetail}? The schools currently out front are {schools}.
uncertainty|evidence|What is known about {playerName}'s recruitment is the leading group: {schools}.
status|wire|Portal interest board\n{playerName}, {position}\n{schools}
position|development|The transfer options for {position} {playerName} have a leading group: {schools}.
uncertainty|cautious|No school has been chosen yet by {playerName}. Watch {schools}, the current leaders.
status|wire|Recruitment in progress — {playerName}, with {schools} leading.
origin|local|The open recruitment of {playerName}{originDetail} has {schools} in its leading group.
uncertainty|evidence|A destination would be a new development. Right now the {playerName} update is {schools} as leaders.
`);
add('roster.announce',`
decision|measured|I have committed to {school}.
pride|expressive|{school}! That's my choice.
decision|conversational|Made my decision. I'm going to {school}
gratitude|measured|Thankful for the opportunity to continue at {school}.
brevity|conversational|My answer is {school}
pride|expressive|You can put {school} next to my name now.
decision|measured|My commitment is to {school}. Glad to share that with everyone.
gratitude|conversational|Appreciate the support through this. I've picked {school}
brevity|expressive|Decision made: {school}.
pride|measured|Proud to say I have chosen {school}.
decision|conversational|Been waiting to share my choice. {school}
gratitude|expressive|Thank you to everybody in my corner. I'm committed to {school}!
brevity|measured|Committed. {school}.
pride|conversational|I'm ready to represent {school}
decision|expressive|It's official for me. I chose {school}!
gratitude|measured|A lot to be grateful for. My next school will be {school}.
brevity|conversational|Going with {school}. That's the update
pride|expressive|Excited doesn't cover it. Committed to {school}.
decision|measured|I am continuing my career at {school}. That is my decision.
gratitude|conversational|Thanks for sticking with me. {school} is my choice
brevity|expressive|My school: {school}!
pride|measured|Looking forward to being part of {school}.
decision|conversational|No more decision posts from me. I'm committed to {school}
gratitude|expressive|All the support means a lot. Happy to announce {school}!
brevity|measured|I have chosen {school} for my next school.
pride|conversational|Feels good to finally say it. {school}
decision|expressive|Picked my place. {school}, let's do this.
gratitude|measured|I appreciate everyone who has supported me. Committed to {school}.
brevity|conversational|One school name for this post: {school}
pride|expressive|Could not be happier to share my choice. {school}!
`);
add('reply.fan.upset',`
agreement|excited|{parentHandle} Let {winner} enjoy this one. They earned the headline.
expectation|skeptical|{parentHandle} The ranking did not play the game for {loser}.
humor|wry|{parentHandle} My very serious pregame opinion has aged terribly.
caution|optimistic|{parentHandle} Great result for {winner}. I'd still like to see the next one before declaring anything bigger.
contrast|rival|{parentHandle} {loser} fans had a much better evening planned.
question|skeptical|{parentHandle} What does this do to the way people judge {loser}, beyond the next poll?
agreement|excited|{parentHandle} The best part is seeing {winner} get credit instead of only hearing about the favorite losing.
expectation|skeptical|{parentHandle} Calling {loser} safe on ranking alone was the mistake.
humor|wry|{parentHandle} This sport keeps deleting my confident predictions.
caution|optimistic|{parentHandle} Enjoy the win now. The season conclusions can wait.
contrast|rival|{parentHandle} Not going to pretend I'm upset about {loser} losing.
question|skeptical|{parentHandle} Does the discussion stay on {winner}, or become entirely about {loser}?
agreement|excited|{parentHandle} Upsets are fun. Being on the {winner} side of one looks even better.
expectation|skeptical|{parentHandle} A poll label never guaranteed {loser} anything.
humor|wry|{parentHandle} Somewhere a prediction post is quietly being deleted.
caution|optimistic|{parentHandle} Happy for {winner}. One big result still leaves a season to assess.
contrast|rival|{parentHandle} I can appreciate {winner} and enjoy {loser} losing at the same time.
question|skeptical|{parentHandle} How much of the reaction credits {winner} rather than just blaming {loser}?
agreement|excited|{parentHandle} Give me the winner's reaction. {winner} deserves that spotlight.
expectation|skeptical|{parentHandle} People treated the ranking gap like the final score was already entered.
humor|wry|{parentHandle} My bracket of confident opinions has another hole in it.
caution|optimistic|{parentHandle} No need for a season prediction to make this a good day for {winner}.
contrast|rival|{parentHandle} {loser} taking an upset loss is enough news for me today.
question|skeptical|{parentHandle} Can we discuss this result without trying to settle the entire season tonight?
agreement|excited|{parentHandle} This is exactly why I keep watching after the predictions are made.
expectation|skeptical|{parentHandle} The expected winner was still expected to earn it. {loser} didn't.
humor|wry|{parentHandle} I would like to withdraw my earlier confidence, for administrative reasons.
caution|optimistic|{parentHandle} The celebration makes sense. The instant dynasty declarations can take a breath.
contrast|rival|{parentHandle} Rough result for {loser}. I am being extremely normal about enjoying it.
question|skeptical|{parentHandle} Which matters more for the conversation now: the win for {winner} or the loss for {loser}?
agreement|excited|{parentHandle} Actual games keep being better than the version we play in our heads.
expectation|skeptical|{parentHandle} I'm less interested in the word upset than in giving {winner} credit for winning.
humor|wry|{parentHandle} A difficult day for my reputation as a person who knows ball.
caution|optimistic|{parentHandle} Big moment for {winner}. I'd rather enjoy it than attach a promise to next week.
contrast|rival|{parentHandle} The {loser} postgame discussion is going to be a long one.
question|skeptical|{parentHandle} What would it take for {winner} to get the credit here without a qualifier?
agreement|excited|{parentHandle} That's a result worth sitting with for a minute.
expectation|skeptical|{parentHandle} The pregame hierarchy did not survive the actual game.
humor|wry|{parentHandle} My prediction was correct until they started playing football.
caution|optimistic|{parentHandle} I can be excited for {winner} without deciding the rest of the schedule tonight.
`);
add('late.rivalry-team',`
rivalry|announcement|Rivalry matchup: {nickname} vs. {opponent}. {hashtag}
pride|pride|For everyone who wears our colors. {opponent} week. {hashtag}
schedule|announcement|Our next rivalry opponent is {opponent}.
pride|pride|A rivalry brings out the {nickname} in all of us. Next: {opponent}.
rivalry|announcement|On the rivalry schedule — {opponent}. {hashtag}
pride|pride|The opponent is familiar. The pride is ours. {nickname} vs. {opponent}.
schedule|announcement|Upcoming rivalry game: {nickname} / {opponent}.
pride|pride|Carry the {nickname} colors into {opponent} week.
rivalry|announcement|This week's rivalry assignment is {opponent}.
pride|pride|A game for the whole {nickname} family. We face {opponent}. {hashtag}
schedule|announcement|Mark the matchup: {nickname} against rival {opponent}.
pride|pride|You know our side. The {nickname} take on {opponent}.
rivalry|announcement|The rivalry returns to the schedule: {opponent}. {hashtag}
pride|pride|School pride gets a familiar test. {nickname} vs. {opponent}.
schedule|announcement|{opponent} is next for the {nickname}, in a rivalry matchup.
pride|pride|Rivalry week. Stand behind your {nickname} against {opponent}.
rivalry|announcement|A familiar opponent ahead: rival {opponent}.
pride|pride|For the colors you wear and the {nickname} you support. Next game: {opponent}.
schedule|announcement|Football's next chapter on our schedule is the rivalry with {opponent}.
pride|pride|Our program. Our side of this rivalry. {nickname} vs. {opponent}.
rivalry|announcement|The upcoming {opponent} matchup is a rivalry game. {hashtag}
pride|pride|Back the {nickname} when we meet {opponent}. Rivalry pride comes with it.
schedule|announcement|Next rivalry game listed: {nickname} — {opponent}.
pride|pride|Some games come with familiar feelings. The {nickname} face {opponent}.
rivalry|announcement|Rivalry opponent on deck: {opponent}. {hashtag}
pride|pride|However you follow this rivalry, bring your {nickname} pride. {opponent} is next.
schedule|announcement|The schedule now turns toward rival {opponent}.
pride|pride|There's only one side for our {nickname} family. Next: {opponent}.
rivalry|announcement|A rivalry date ahead for {nickname} and {opponent}.
pride|pride|Represent the {nickname} as the rivalry with {opponent} comes around.
`);
add('model.line',`
projection|wire|Model projection: {favorite} -{spread} against {opponent}.
uncertainty|evidence|The model favors {favorite} by {spread}. That is a forecast for {opponent}, not a result.
matchup|schedule|For {favorite} vs. {opponent}, our projected margin is {spread} toward {favorite}.
projection|wire|Projected favorite — {favorite}. Model spread — {spread}. Opponent — {opponent}.
uncertainty|cautious|A number to compare with the game later: {favorite} -{spread} vs. {opponent}.
matchup|evidence|The projection separates {favorite} and {opponent} by {spread}, in {favorite}'s favor.
projection|wire|On the model board: {favorite}, favored by {spread} over {opponent}.
uncertainty|cautious|No final score here. The pregame model has {favorite} -{spread} against {opponent}.
matchup|schedule|The {opponent} matchup comes with a model edge of {spread} for {favorite}.
projection|wire|Our forecast lists {favorite} as the {spread}-point favorite against {opponent}.
uncertainty|evidence|Treat it as an estimate: {favorite} by {spread} against {opponent}.
matchup|schedule|Looking at {favorite} and {opponent}, the model places the margin at {spread} toward {favorite}.
projection|wire|Model favorite: {favorite} over {opponent}, projected margin {spread}.
uncertainty|cautious|The forecast is one input to the preview. It reads {favorite} -{spread} vs. {opponent}.
matchup|evidence|A projected {spread}-point difference favors {favorite} in the matchup with {opponent}.
projection|wire|Pregame number — {favorite} -{spread}; opponent {opponent}.
uncertainty|cautious|There is still a game to play against {opponent}. Our model favors {favorite} by {spread}.
matchup|schedule|For the upcoming meeting with {opponent}, {favorite} carries our model's {spread}-point edge.
projection|wire|The line from our model is {favorite} by {spread} over {opponent}.
uncertainty|evidence|Keep the projection separate from the eventual score: {favorite} -{spread} against {opponent}.
matchup|schedule|{favorite} has the modeled edge over {opponent}. Its size is {spread} points.
projection|wire|Forecast card: {favorite} vs. {opponent}. Pick: {favorite} by {spread}.
uncertainty|cautious|A projected edge can be wrong. This one has {favorite} favored by {spread} against {opponent}.
matchup|evidence|The model's comparison of these opponents lands at {favorite} -{spread} vs. {opponent}.
projection|wire|Margin on the forecast sheet: {spread}, favoring {favorite} over {opponent}.
uncertainty|cautious|Before the result, an expectation to log: {favorite} by {spread} against {opponent}.
matchup|schedule|The {favorite}–{opponent} preview gets a {spread}-point projection toward {favorite}.
projection|wire|Projected matchup\n{favorite} over {opponent}\nMargin: {spread}
uncertainty|evidence|That is the model's expectation, not a guarantee: {favorite} -{spread} against {opponent}.
matchup|schedule|{opponent} faces a model favorite in {favorite}; the projected gap is {spread} points.
`);
add('portal.entrant',`
entry|wire|Portal entry: {position} {playerName}{originDetail}.
status|cautious|In the portal, without a new destination confirmed: {playerName}{originDetail}.
position|development|A {position} is available in the transfer portal: {playerName}.
entry|wire|{playerName}{originDetail} has entered the transfer portal.
status|evidence|Entry is the verified development for {playerName}. It does not establish where he will play next.
position|local|For the {position} portal board, add {playerName}{originDetail}.
entry|wire|Transfer update — {playerName} is a portal entrant.
status|cautious|The school decision is still ahead. {playerName} is in the transfer portal{originDetail}.
position|development|The portal now includes {position} {playerName}{originDetail}.
entry|wire|A new portal name to log: {playerName}.
status|evidence|Do not attach a new-school announcement to this yet. {playerName}'s update is a portal entry.
position|local|At {position}, one transfer name to track is {playerName}{originDetail}.
entry|wire|Status change recorded: {playerName} has joined the transfer portal.
status|cautious|Where {playerName} goes remains open. His portal entry is confirmed{originDetail}.
position|development|Programs evaluating the {position} portal have {playerName} on the list.
entry|wire|Entered the portal — {playerName}{originDetail}.
status|evidence|One confirmed step for {playerName}: entering the portal. A destination would be another development.
position|local|Transfer board, {position}: {playerName}{originDetail}.
entry|wire|{playerName} appears among the transfer portal entrants.
status|cautious|A portal listing is not a commitment. The name in this update is {playerName}.
position|development|The {position} transfer options include a portal entrant in {playerName}.
entry|wire|Portal watch begins for {playerName}{originDetail}.
status|evidence|The next-school question is unanswered here. {playerName} has entered the portal.
position|local|A name for those following transfer {position} prospects: {playerName}.
entry|wire|Transfer portal record updated with {playerName}{originDetail}.
status|cautious|No new home confirmed for {playerName}. The current status is portal entrant.
position|development|A {position} recruitment to follow in the portal: {playerName}{originDetail}.
entry|wire|{playerName}'s name is on the portal list.
status|evidence|Separate the entry from the eventual move. {playerName} is now in the transfer portal.
position|local|One entry on the {position} transfer board reads {playerName}{originDetail}.
`);
add('beat.standings',`
leaders|wire|{conference} leaders: {leaders}.
snapshot|cautious|The current {conference} order starts with {leaders}. This is a standings snapshot, not a title prediction.
race|race|Start the {conference} race discussion with its current leaders: {leaders}.
leaders|local|At the front of the {conference}: {leaders}.
snapshot|evidence|Standings on record in the {conference} show {leaders} at the top.
race|schedule|The chase in the {conference} currently runs through {leaders}.
leaders|wire|Conference board — {conference}: {leaders} lead.
snapshot|cautious|No championship decided by this list alone. The {conference} standings begin with {leaders}.
race|race|The names setting the pace in the {conference} are {leaders}.
leaders|local|Looking up the {conference} table? Its leading group is {leaders}.
snapshot|evidence|For the record, the current leaders in the {conference} are {leaders}.
race|schedule|A starting point for the next {conference} race update: {leaders} lead the standings.
leaders|wire|Top of the standings\n{conference}\n{leaders}
snapshot|cautious|Today's {conference} snapshot puts {leaders} in the leading group.
race|race|Before projecting the conference finish, here is the current {conference} front: {leaders}.
leaders|local|The {conference} table is led by {leaders}.
snapshot|evidence|Current position, rather than a forecast: {leaders} head the {conference} standings.
race|schedule|For programs chasing the {conference} leaders, the names ahead are {leaders}.
leaders|wire|Leading group listed for the {conference}: {leaders}.
snapshot|cautious|The order can still change. Right now {leaders} lead the {conference}.
race|race|A conference race needs a current table. The {conference} leaders are {leaders}.
leaders|local|First names on the {conference} standings sheet: {leaders}.
snapshot|evidence|This update records the {conference} leaders as {leaders}.
race|schedule|The {conference} watch starts at the top, with {leaders}.
leaders|wire|Standings entry — {conference}, led by {leaders}.
snapshot|cautious|Keep the current table separate from the final one. {leaders} now lead the {conference}.
race|race|At this point in the {conference} race, {leaders} occupy the leading places.
leaders|local|A look at who is out front: {conference}, {leaders}.
snapshot|evidence|On the current {conference} sheet, the leading names read {leaders}.
race|schedule|Tracking the conference chase? In the {conference}, begin with {leaders}.
`);
add('cycle.team-welcome',`
signing|announcement|Signed with {school}: {playerName}. Welcome, {tagHandle}.
pride|pride|A new member of the {nickname} family. Welcome to {school}, {tagHandle}!
signing|announcement|The signing is official. {playerName} joins the {nickname}.
pride|pride|Proud to welcome {playerName} to {school}. {tagHandle} {hashtag}
signing|announcement|Signing announcement — {playerName}, {school}.
pride|pride|Our newest {nickname} signing deserves a welcome. Glad to have you, {tagHandle}.
signing|announcement|{playerName} is signed. His school: {school}.
pride|pride|The {nickname} family welcomes {tagHandle}. {playerName} has signed with us.
signing|announcement|Add {playerName} to the signed list for {school}. {tagHandle}
pride|pride|New name. Our colors. Welcome to the {nickname}, {playerName}.
signing|announcement|Official signing update: {school} has signed {playerName}.
pride|pride|Give {tagHandle} a {nickname} welcome. {playerName} is signed.
signing|announcement|Signed prospect\n{playerName}\n{school}
pride|pride|A signing to celebrate with the {nickname}. Welcome aboard, {tagHandle}.
signing|announcement|{school}'s signing board now includes {playerName}.
pride|pride|To everyone who supports {school}: welcome {playerName} to our program. {tagHandle}
signing|announcement|Confirmed on the signed list: {playerName}, {nickname}.
pride|pride|The next {nickname} name to know is {playerName}. Officially signed. {hashtag}
signing|announcement|A {school} signing to introduce: {playerName}. {tagHandle}
pride|pride|Glad to call {playerName} part of {school}. Welcome, {tagHandle}.
signing|announcement|The signed paperwork brings {playerName} to {school}.
pride|pride|Our program gets a new signing in {playerName}. Show {tagHandle} some {nickname} pride.
signing|announcement|Status: signed. Player: {playerName}. Program: {school}.
pride|pride|Welcome to the colors, {playerName}. Signed with the {nickname}. {tagHandle}
signing|announcement|New signing recorded for the {nickname}: {playerName}.
pride|pride|A proud welcome from {school} to {tagHandle}. {playerName} is officially signed.
signing|announcement|Introducing a {school} signee: {playerName}.
pride|pride|From the {nickname} family, welcome {playerName}. Happy to have {tagHandle} signed.
signing|announcement|{playerName} — officially a {school} signee. {tagHandle}
pride|pride|Celebrate our newest signing, {nickname}. Welcome, {playerName}. {hashtag}
`);
add('reply.player.offer',`
agreement|measured|{parentHandle} Congratulations on the offer from {school}.
question|conversational|{parentHandle} what are you looking for most in a school?
welcome|expressive|{parentHandle} Would be good to see you in our colors.
perspective|measured|{parentHandle} An offer gives you another option. Take your time with the choice.
agreement|conversational|{parentHandle} happy you have {school} as an option
question|expressive|{parentHandle} How are you feeling about the decision ahead?
welcome|measured|{parentHandle} Hope you find the right place. Would be glad if it were {school}.
perspective|conversational|{parentHandle} your decision, your timeline. congrats on the offer
agreement|expressive|{parentHandle} An offer from {school}! Happy for you.
question|measured|{parentHandle} What matters most to you in finding the right program?
welcome|conversational|{parentHandle} I know which colors I'd like to see you wearing
perspective|expressive|{parentHandle} Take it in. You don't have to decide in the replies.
agreement|measured|{parentHandle} Glad you have that opportunity with {school}.
question|conversational|{parentHandle} are you enjoying the process or ready for the decision to be over?
welcome|expressive|{parentHandle} Could get used to seeing your name next to {school}.
perspective|measured|{parentHandle} The offer is good news. The fit is yours to decide.
agreement|conversational|{parentHandle} another choice on the table. happy for you
question|expressive|{parentHandle} What's the biggest thing you're weighing right now?
welcome|measured|{parentHandle} Congratulations. Would welcome you at {school} if that is your choice.
perspective|conversational|{parentHandle} enjoy this part before everybody starts telling you where to go
agreement|expressive|{parentHandle} That's a good update to see. Congrats!
question|measured|{parentHandle} Do you have a sense of what will decide the right fit?
welcome|conversational|{parentHandle} {school} would look right next to your name, just saying
perspective|expressive|{parentHandle} Everybody has an opinion. Make the choice for yourself.
agreement|measured|{parentHandle} Wishing you well as you consider the {school} offer.
question|conversational|{parentHandle} what part of picking a program are you most excited about?
welcome|expressive|{parentHandle} I'd be happy to welcome you here.
perspective|measured|{parentHandle} A good opportunity to consider. No rush from me on a decision.
agreement|conversational|{parentHandle} saw the offer from {school}. congrats
welcome|expressive|{parentHandle} You already know which school I'd pick for you. Your call though.
`);
add('cycle.player-reset',`
reflection|measured|Appreciate everyone who followed our season.
team|conversational|Going to miss game weeks with this group for a bit
reflection|expressive|The season went fast. Still taking it all in.
gratitude|measured|Thank you to the people who kept showing up for us.
team|conversational|Teammates make the long days worth it
reflection|expressive|A lot of feelings when a season ends. Football is hard to put down.
gratitude|measured|Grateful for another season with this team.
team|conversational|Already missing the routine with the guys
reflection|expressive|Still thinking through the season. Not much else to say yet.
gratitude|measured|The support this season mattered. Thank you.
team|conversational|Some days were better than others. glad I had this group with me
reflection|expressive|Feels strange not having the next game to think about.
gratitude|measured|To everyone who supported our team, I appreciate you.
team|conversational|Football gives you people you don't forget
reflection|expressive|Season ends and my mind is still on football.
gratitude|measured|Thankful I got to be part of this season.
team|conversational|Hard to explain how much the people around you mean until a season is over
reflection|expressive|Don't have a perfect season-ending caption. Just a lot to appreciate.
gratitude|measured|I will remember the people who supported us through this year.
team|conversational|Time to breathe a little. going to miss being around the group every game week
reflection|expressive|You spend all year waiting for it and then the season is over.
gratitude|measured|Appreciate the time and energy people gave to our team.
team|conversational|The teammates are what stay with you
reflection|expressive|Still not used to the season being done.
gratitude|measured|Thank you for following along with us this year.
team|conversational|A season is a lot of small moments with the same people. thankful for that
reflection|expressive|Can't fit a whole season into one post.
gratitude|measured|Taking a moment to appreciate everyone around this team.
team|conversational|Whatever the day looked like, I was glad to be with the guys
reflection|expressive|Football season never feels long enough when it ends.
`);
add('reply.fan.player_good',`
credit|excited|{parentHandle} Give {playerName} his part of the credit. That's the production I'm here to talk about.
context|skeptical|{parentHandle} I like {playerName}'s line. I still want the team result kept beside it.
humor|wry|{parentHandle} Conveniently leaving this here for my next argument about {playerName}.
question|optimistic|{parentHandle} Can {playerName} build on that in the next game?
perspective|rival|{parentHandle} I can admit {playerName} played well without changing who I root for.
credit|excited|{parentHandle} Happy to see {playerName} get a post of his own for this.
context|skeptical|{parentHandle} Individual production is worth discussing. It doesn't answer every question about {school}.
humor|wry|{parentHandle} I will be using this {playerName} post in conversations for an unreasonable amount of time.
question|optimistic|{parentHandle} What would a repeat of that look like for {school}?
perspective|rival|{parentHandle} Not my team, but {playerName} deserves that mention.
credit|excited|{parentHandle} More attention on what the report actually shows about {playerName}, please.
context|skeptical|{parentHandle} Good to have a specific line to discuss rather than just calling {playerName} good.
humor|wry|{parentHandle} My extremely biased {playerName} agenda appreciates the material.
question|optimistic|{parentHandle} Is {playerName} the part of this game you keep coming back to?
perspective|rival|{parentHandle} Rooting against {school} doesn't mean pretending that wasn't good work from {playerName}.
credit|excited|{parentHandle} This is the {playerName} conversation I wanted to see.
context|skeptical|{parentHandle} Credit for the production, without turning one update into a whole-season verdict.
humor|wry|{parentHandle} Saving the {playerName} evidence for completely calm and reasonable future use.
question|optimistic|{parentHandle} How much can {school} keep getting from {playerName} like that?
perspective|rival|{parentHandle} Reluctant credit to {playerName}. I remain reluctant.
credit|excited|{parentHandle} There's a reason I wanted {playerName} included in the recap.
context|skeptical|{parentHandle} The player line and the team story deserve separate attention here.
humor|wry|{parentHandle} A helpful contribution to my ongoing inability to be normal about {school}.
question|optimistic|{parentHandle} What part of {playerName}'s performance stood out most to you?
perspective|rival|{parentHandle} Wrong school for my taste. Still good production from {playerName}.
credit|excited|{parentHandle} Let {playerName} have this bit of the spotlight.
context|skeptical|{parentHandle} This is useful evidence on {playerName}. I'd rather stick to it than jump to awards predictions.
humor|wry|{parentHandle} Please respect my decision to bring up {playerName} at every opportunity.
question|optimistic|{parentHandle} Does this change what you want to see from {school} next week?
perspective|rival|{parentHandle} Even I can give {playerName} that one.
`);
add('buzz.watcher-player-Unpredictable',`
spectator|expressive|{home} and {away} in the {eventLabel}. I have no business being this invested.
question|conversational|Trying to pick a side for {home}–{away}. who you got in the {eventLabel}?
humor|expressive|The {eventLabel} has {home} and {away}. My neutral-fan status may not last.
spectator|measured|I want to watch {home} against {away} in the {eventLabel} without making a prediction.
question|conversational|{home} or {away} in the {eventLabel}? I keep changing my mind
humor|expressive|Ready to have unnecessarily strong opinions about {home}–{away} in the {eventLabel}.
spectator|conversational|{eventLabel}: {home} vs {away}. just here to watch some football
question|expressive|Am I the only one having trouble picking {home} or {away} for the {eventLabel}?
humor|conversational|My plan for {home}–{away} is to be confidently wrong about the {eventLabel}
spectator|measured|Looking forward to the {eventLabel} matchup between {home} and {away}.
question|expressive|Who is more confident in their {eventLabel} pick than I am? {home} or {away}?
humor|conversational|{home} against {away}. the {eventLabel} may be bad for my screen time
spectator|expressive|I don't need a pick to care about {home}–{away} in the {eventLabel}.
question|conversational|Watching the {eventLabel} for {home}, {away}, or just the game?
humor|expressive|The {eventLabel} is {home} vs {away}. I reserve the right to change sides repeatedly.
spectator|measured|A spectator for this one: {home}, {away}, {eventLabel}.
question|conversational|Need somebody to talk me into a {eventLabel} pick. {home} or {away}?
humor|expressive|Going into {home}–{away} with a very scientific plan: vibes. {eventLabel}.
spectator|conversational|On my watch list: the {eventLabel}, {home} against {away}
question|expressive|Anybody else watching {home}–{away} in the {eventLabel} without rooting for either?
humor|conversational|I can already tell I'm going to take {home}–{away} too seriously. {eventLabel}
spectator|measured|The {eventLabel} gets my attention with {home} and {away} matched up.
question|expressive|What's your pick for the {eventLabel}? Mine for {home}–{away} is still loading.
humor|conversational|Please hold me to none of my {home}–{away} opinions. excited for the {eventLabel}
spectator|expressive|{home}. {away}. {eventLabel}. That's enough to get me watching.
question|conversational|Who are the neutral fans backing in the {eventLabel}: {home} or {away}?
humor|expressive|Trying to watch {home}–{away} normally. No promises. {eventLabel}.
spectator|measured|Just football to watch for me: {home} against {away}, in the {eventLabel}.
question|conversational|Anyone actually settled on a {home}–{away} pick for the {eventLabel} yet?
humor|expressive|My contribution to {eventLabel} analysis: I'm excited to watch {home} and {away}.
`);
add('history.coach-counter',`
milestone|wire|Career milestone: {coachName}, {ordinal} {achievement}. School: {school}.
career|local|At {school}, {coachName}'s career ledger now reaches a {ordinal} {achievement}.
scope|cautious|This is a career total: {coachName} has recorded his {ordinal} {achievement}.
milestone|wire|{ordinal} career {achievement} recorded for {school} coach {coachName}.
career|evidence|Add another milestone to {coachName}'s career record — his {ordinal} {achievement}.
scope|local|A career mark to note alongside the {school} news: {coachName}, {ordinal} {achievement}.
milestone|wire|Milestone entry — {coachName}: {ordinal} career {achievement}.
career|development|The career count for {coachName} reaches a {ordinal} {achievement} while he is at {school}.
scope|cautious|The achievement covers {coachName}'s career, not only his time at {school}: {ordinal} {achievement}.
milestone|wire|{school}'s {coachName} hits career {achievement} number {ordinal}.
career|local|One more line in the career record for {coachName}: his {ordinal} {achievement}.
scope|evidence|For the career record, {school} coach {coachName} now has a {ordinal} {achievement}.
milestone|wire|Recorded today: {coachName}'s {ordinal} career {achievement}.
career|local|{coachName} reaches a career marker at {school}, recording his {ordinal} {achievement}.
scope|cautious|A career milestone, without a school-record claim: {coachName}, {ordinal} {achievement}.
milestone|wire|Career {achievement} milestone\n{coachName}\n{ordinal}
career|development|A new marker on {coachName}'s career timeline: {ordinal} {achievement}.
scope|evidence|{school} has a coach with a new career milestone: {coachName}'s {ordinal} {achievement}.
milestone|wire|{coachName}: career {achievement}, {ordinal} milestone reached.
career|local|The latest career note on {school} coach {coachName} is his {ordinal} {achievement}.
scope|cautious|Count this across the coaching career: {coachName}'s {ordinal} {achievement}.
milestone|wire|A {ordinal} career {achievement} is on record for {coachName}.
career|development|For {coachName}, the career ledger adds a {ordinal} {achievement} at {school}.
scope|evidence|The milestone is career-wide. {school}'s {coachName} records his {ordinal} {achievement}.
milestone|wire|Career marker reached — {ordinal} {achievement} for {coachName}.
career|local|Following the {school} staff? {coachName} has his {ordinal} career {achievement}.
scope|cautious|There is a career total to report for {coachName}: {ordinal} {achievement}.
milestone|wire|{coachName} records career {achievement} milestone {ordinal}.
career|development|The newest career milestone for {school} coach {coachName} is a {ordinal} {achievement}.
scope|evidence|A place in the career log for {coachName}'s {ordinal} {achievement}.
`);
// New alternatives extend rather than replace existing coverage. Conditionally
// richer perspectives are eligible only when the captured slot exists.
const context=[
 ['beat.performance','outcome','evidence','{resultLine} The individual evidence from {playerName} against {opponent}: {statLine}.'],
 ['roster.commit','evaluation','development','{school} has a commitment from {playerName}. {recruitRankDetail} The future role is still to be earned.'],
 ['team.upcoming','season','announcement','At {recordContext}, the {nickname} turn to {opponent}. {hashtag}'],
 ['late.rivalry-team','season','pride','A {recordContext} record heading into the rivalry. {nickname} vs. {opponent}. {hashtag}'],
 ['reply.fan.player_good','agreement','excited','{parentHandle} "{parentDetail}" is the part worth highlighting. Glad that made the recap.'],
 ['reply.fan.player_good','contrast','skeptical','{parentHandle} I saw "{parentDetail}" too. Useful detail, though one update is still one update.'],
 ['reply.fan.upset','contrast','skeptical','{parentHandle} "{parentDetail}" is confirmed. The new poll position people are guessing at is not.'],
 ['reply.player.offer','perspective','measured','{parentHandle} Saw the {school} offer. Congratulations, and take the time you need.']
];
for(const [family,angle,voice,body] of context)families[family].push({id:`${family}.cot104.context.${families[family].length}`,angle,voices:[voice],body,when:[...body.matchAll(/\{(recordContext|parentDetail)\}/g)].map(m=>m[1])});
// Secondary families dominate the larger native Conference/National windows.
// Add genuine alternative fact emphasis there as well as in the For You top 20.
add('team.poll_up',`
movement|announcement|{pollName} update: {nickname}, No. {rank}. Previously No. {previousRank}.
pride|pride|Up to No. {rank} for your {nickname}. {pollName}, from No. {previousRank}. {hashtag}
movement|announcement|A climb in the {pollName}: No. {previousRank} to No. {rank}.
pride|pride|Proud of the {nickname}. Our new {pollName} position is No. {rank}, up from No. {previousRank}.
movement|announcement|New ranking\n{pollName}: {rank}\nPrevious: {previousRank}
pride|pride|The {nickname} move higher. No. {rank} in the {pollName}, after No. {previousRank}. {hashtag}
movement|announcement|Now listed at No. {rank} in the {pollName}. The {nickname} were No. {previousRank}.
pride|pride|Carry those {nickname} colors proudly. {pollName}: No. {previousRank} → No. {rank}.
movement|announcement|Ranking movement for {school}: {previousRank} → {rank} in the {pollName}.
pride|pride|A step up for our program. The {pollName} puts the {nickname} No. {rank}, previously No. {previousRank}.
movement|announcement|The updated {pollName} has {school} at No. {rank}. Last place: No. {previousRank}.
pride|pride|A higher spot beside the {nickname} name: No. {rank}. Up from No. {previousRank} in the {pollName}.
movement|announcement|From {previousRank} to {rank}: {school}'s change in the {pollName}.
pride|pride|Your {nickname} have climbed in the {pollName}. The new number is {rank}, replacing {previousRank}.
movement|announcement|Poll card — {school}, No. {rank}. Change from No. {previousRank}, {pollName}.
pride|pride|The colors are the same. The poll spot is higher. {nickname}: {previousRank} → {rank}, {pollName}.
movement|announcement|{school} moves up in the {pollName}, to No. {rank} from No. {previousRank}.
pride|pride|Another ranking update to share with the {nickname} family: No. {rank} in the {pollName}, up from No. {previousRank}.
`);
add('beat.poll_same',`
position|wire|No movement for {school}: No. {rank}, {pollName}.
snapshot|evidence|The current {pollName} lists {school} at No. {rank}, unchanged.
continuity|local|A familiar spot for {school}. The {pollName} keeps the program No. {rank}.
position|wire|Ranking status — {school}, No. {rank} in the {pollName}. Unchanged.
snapshot|cautious|No new poll position to report for {school}. It remains No. {rank} in the {pollName}.
continuity|race|The next ranking discussion starts from the same place for {school}: No. {rank}, {pollName}.
position|wire|{pollName}: {school}, {rank}. No change from the previous list.
snapshot|evidence|On the updated list, {school}'s number is still {rank}. Source: {pollName}.
continuity|local|The {school} poll entry stays put. No. {rank} in the {pollName}.
position|wire|Still ranked {rank}: {school}, {pollName}.
snapshot|cautious|The update changes no position for {school}. {pollName}, No. {rank}.
continuity|race|For the ranking watch, carry forward No. {rank} beside {school}. The {pollName} position is unchanged.
position|wire|Poll entry\n{school}\nNo. {rank}, {pollName}\nUnchanged
snapshot|evidence|{school} appears at the same No. {rank} position in the {pollName}.
continuity|local|No rise or fall this time for {school}, still No. {rank} in the {pollName}.
position|wire|Unchanged at {rank}. That's {school}'s position in the {pollName}.
snapshot|cautious|There is a new poll, with the same {school} placement: No. {rank} in the {pollName}.
continuity|race|The {pollName} leaves {school}'s starting point for the next update at No. {rank}.
`);
add('portal.unsigned-local',`
status|wire|Still uncommitted: {playerName}, No. {nationalRecruitRank} nationally.
position|development|A high school {position} remains on the open board: {playerName}, national No. {nationalRecruitRank}.
uncertainty|cautious|No school choice recorded for {playerName}. The prospect is ranked No. {nationalRecruitRank} nationally.
status|wire|Open recruitment — {playerName}. National recruit rank: {nationalRecruitRank}.
evaluation|development|{starDetail}{playerName} has a national No. {nationalRecruitRank} ranking and remains uncommitted.
uncertainty|evidence|The rank is known, the destination is not: {playerName}, national No. {nationalRecruitRank}.
status|wire|{playerName} is available on the high school recruiting board, ranked No. {nationalRecruitRank} nationally.
position|local|At {position}, an unsigned name to track is {playerName}, No. {nationalRecruitRank} nationally.
uncertainty|cautious|Keep {playerName} in the uncommitted column. His national ranking is No. {nationalRecruitRank}.
status|wire|Recruitment status: uncommitted. Prospect: {playerName}. National rank: {nationalRecruitRank}.
evaluation|development|A {starDetail}{position} is still weighing a school choice: {playerName}, national No. {nationalRecruitRank}.
uncertainty|evidence|This is an open recruitment, without a destination to announce. {playerName} stands No. {nationalRecruitRank} nationally.
status|wire|No commitment yet from national No. {nationalRecruitRank} prospect {playerName}.
position|local|The high school {position} board still has {playerName} uncommitted. National rank: {nationalRecruitRank}.
uncertainty|cautious|A school decision would be new information on {playerName}. For now he is uncommitted, national No. {nationalRecruitRank}.
status|wire|Uncommitted board\n{playerName}\nNational No. {nationalRecruitRank}
evaluation|development|Looking at remaining high school prospects: {playerName}, {starDetail}{position}, national No. {nationalRecruitRank}.
uncertainty|evidence|The current status for {playerName} remains open. National ranking: {nationalRecruitRank}.
`);
add('cycle.signed-spotlight',`
signing|announcement|Signed-class spotlight: {playerName}, {school}. {tagHandle}
pride|pride|Proud to have {tagHandle} in our signed class. {nickname} family, meet {playerName}.
signing|announcement|One name in the {school} signed class: {playerName}.
pride|pride|A {nickname} signing we are happy to share: {tagHandle}. {hashtag}
signing|announcement|Class introduction — {playerName}, signed with {school}.
pride|pride|Our signed class includes {playerName}. Good to have you with the {nickname}, {tagHandle}.
signing|announcement|Get to know the signed class: {tagHandle}, {playerName}.
pride|pride|The {school} colors have a signed-class name in {playerName}. {hashtag}
signing|announcement|On the signed board for {school}: {playerName}. {tagHandle}
pride|pride|A welcome from the {nickname} family to one of our signees, {tagHandle}.
signing|announcement|Signed player profile\n{playerName}\n{school}
pride|pride|Happy to introduce {playerName} as part of the {nickname} signed class.
signing|announcement|From our list of signees: {playerName}. {tagHandle}
pride|pride|Show {tagHandle} some school pride. {playerName} is signed with {school}.
signing|announcement|{playerName} has a place in the {school} signed class.
pride|pride|Meet a signee wearing our name next: {playerName}. Proud to have {tagHandle} with the {nickname}.
signing|announcement|The {school} signed-class introduction turns to {playerName}.
pride|pride|A signed-class shout for {tagHandle}. Welcome to {school}, {playerName}.
`);
add('cycle.outlook',`
record|wire|Season complete for {school}: {record}. {expectationNote}
transition|local|The games are finished for {school}. After {record}, the focus turns to the roster and recruiting.
expectations|evidence|Match the record to the expectations: {school}, {record}. {expectationNote}
record|wire|Final season ledger — {school}, {record}.
transition|development|With a {record} season finished, {school}'s next questions concern retention and the incoming class.
expectations|cautious|{expectationNote} That is the context for {school}'s completed {record} season.
record|local|The last game is behind {school}. Its season record is {record}.
transition|schedule|A completed schedule, a {record} record, and roster decisions ahead for {school}.
expectations|evidence|The season review begins with {record} for {school}. {expectationNote}
record|wire|{school}\nSeason finished\nRecord: {record}
transition|development|What changes on the roster is the next {school} story. The completed season reads {record}.
expectations|cautious|No need to predict next season to assess this one. {school}: {record}. {expectationNote}
record|local|Put {record} in the final record column for {school}.
transition|schedule|From completed games to offseason choices: {school} finishes {record}.
expectations|evidence|For the {school} review, keep the baseline beside the outcome. {record}. {expectationNote}
record|wire|Finished on the field: {school}, {record}.
transition|development|The {school} offseason starts from a completed {record} schedule. Retention and recruiting are the next watch points.
expectations|cautious|The record is settled for {school}, at {record}. {expectationNote}
`);
add('cycle.portal-needs',`
roster|wire|{school} roster note: {count} {seniorLabel} at {position}.
need|development|How does {school} replenish {position}? There are {count} {seniorLabel} in the group, with eligibility still to assess.
uncertainty|cautious|Senior status is not the same as a confirmed departure. {school} has {count} {seniorLabel} at {position}.
roster|local|A position group to follow at {school}: {position}, with {count} {seniorLabel}.
need|development|The portal is one possible route at {position} for {school}. Its roster includes {count} {seniorLabel} there.
uncertainty|evidence|Before declaring a vacancy, check eligibility. The known count at {school} is {count} {seniorLabel} at {position}.
roster|wire|Position planning\n{school}: {position}\n{count} {seniorLabel}
need|local|Recruiting and retention both matter to {school}'s {position} group, which has {count} {seniorLabel}.
uncertainty|cautious|That group may need attention, but departures are separate news: {school}, {position}, {count} {seniorLabel}.
roster|wire|At {position}, {school} lists {count} {seniorLabel}.
need|development|One offseason question for {school} is continuity at {position}, with {count} {seniorLabel} currently listed.
uncertainty|evidence|A senior count gives a planning question, not a promised transfer. {school}: {count} {seniorLabel}, {position}.
roster|local|The {school} position watch includes {position}: {count} {seniorLabel}.
need|development|For the {school} recruiting board, consider the {position} group and its {count} {seniorLabel}. Remaining eligibility matters.
uncertainty|cautious|No portal addition is implied by this count. {school} has {count} {seniorLabel} at {position}.
roster|wire|Roster count to note for {school}: {position}, {count} {seniorLabel}.
need|local|Who returns and who is added will shape {school}'s {position} group. It currently includes {count} {seniorLabel}.
uncertainty|evidence|The known fact is the senior count at {position}: {count} {seniorLabel} for {school}. The roster outcome is still ahead.
`);
add('beat.team_stats',`
offense|wire|{school} against {opponent}: {passingYards} passing yards and {rushingYards} rushing yards.
possession|evidence|{firstDowns} first downs, {giveaways} {giveawayLabel}. Those are two parts of {school}'s box score vs. {opponent}.
balance|film|The yardage split for {school} vs. {opponent}: {passingYards} passing, {rushingYards} rushing.
offense|local|From {school}'s game against {opponent}, the offensive totals read {passingYards} passing yards and {rushingYards} rushing yards.
possession|cautious|The box score records {giveaways} {giveawayLabel} for {school} against {opponent}. It also shows {firstDowns} first downs.
balance|film|Through the air: {passingYards}. On the ground: {rushingYards}. {school}'s yards against {opponent}.
offense|wire|Game totals — {school} vs. {opponent}: pass {passingYards}, rush {rushingYards}, first downs {firstDowns}.
possession|evidence|Keep the {school} possession numbers together: {firstDowns} first downs and {giveaways} {giveawayLabel} against {opponent}.
balance|local|Two ways to move the ball: {school} had {rushingYards} rushing yards alongside {passingYards} passing yards vs. {opponent}.
offense|wire|Passing yards: {passingYards}. Rushing yards: {rushingYards}. Team: {school}. Opponent: {opponent}.
possession|cautious|No need to guess at possession details from the score. {school} recorded {firstDowns} first downs and {giveaways} {giveawayLabel} vs. {opponent}.
balance|film|{rushingYards} on the ground, {passingYards} through the air for {school} against {opponent}.
offense|local|The offensive sheet from {opponent} lists {passingYards} passing yards and {rushingYards} rushing yards for {school}.
possession|evidence|A ball-security number to log: {school}, {giveaways} {giveawayLabel} against {opponent}.
balance|film|Read {school}'s yardage in two columns: pass {passingYards}, rush {rushingYards}. Opponent: {opponent}.
offense|wire|For the {school} recap: {passingYards} passing, {rushingYards} rushing, {firstDowns} first downs, {giveaways} {giveawayLabel} vs. {opponent}.
possession|local|Against {opponent}, {school} finished with {firstDowns} first downs. The giveaway total was {giveaways}.
balance|evidence|The {opponent} game left {school} with a passing/rushing yardage split of {passingYards}/{rushingYards}.
`);
add('beat.watch',`
player|wire|Watch-list name for {school} vs. {opponent}: {playerName}.{watchDetail}
matchup|film|One player to follow in the {opponent} matchup is {school}'s {playerName}.{watchDetail}
context|cautious|A preview focus, not a performance prediction: {playerName} for {school} against {opponent}.{watchDetail}
player|local|From the {school} roster, keep an eye on {playerName} against {opponent}.{watchDetail}
matchup|schedule|The {school}–{opponent} preview includes a player watch on {playerName}.{watchDetail}
context|development|A name to evaluate as the game unfolds: {school}'s {playerName} vs. {opponent}.{watchDetail}
player|wire|Player watch\n{playerName}, {school}\nOpponent: {opponent}{watchDetail}
matchup|film|For the matchup with {opponent}, my {school} player focus is {playerName}.{watchDetail}
context|cautious|The game will provide the answer. Before it, {playerName} is a {school} player to watch vs. {opponent}.{watchDetail}
player|local|Tracking {school} this week? Put {playerName} on the watch list for {opponent}.{watchDetail}
matchup|schedule|A player thread in the {opponent} preview: {playerName} of {school}.{watchDetail}
context|development|The individual watch for {school} turns to {playerName} against {opponent}.{watchDetail}
player|wire|Preview note — {playerName}, {school}, vs. {opponent}.{watchDetail}
matchup|film|When {school} meets {opponent}, {playerName} is one name worth following.{watchDetail}
context|cautious|No stat projection attached. Just a watch-list spot for {school}'s {playerName} vs. {opponent}.{watchDetail}
player|local|A {school} name for your {opponent} game notes: {playerName}.{watchDetail}
matchup|schedule|The player angle to follow into {school}'s game with {opponent}: {playerName}.{watchDetail}
context|development|Before the box score exists, there is a player to evaluate: {playerName}, {school}, against {opponent}.{watchDetail}
`);
add('ending.expectation-review',`
record|wire|Final record — {school}: {record}. {expectationNote}
expectations|evidence|The expectation check for {school} starts with a {record} finish. {expectationNote}
review|local|Looking back at {school}, the completed season reads {record}. {expectationNote}
record|wire|{record}. That is the finished record for {school}. {expectationNote}
expectations|cautious|Keep the outcome beside the baseline: {school} finishes {record}. {expectationNote}
review|schedule|A completed schedule to review at {school}: {record}. {expectationNote}
record|local|The season ledger closes on {record} for {school}. {expectationNote}
expectations|evidence|How the season matched expectations is the question for {school}. The record is {record}. {expectationNote}
review|cautious|One season's review, with the next still ahead: {school}, {record}. {expectationNote}
record|wire|Season closed\n{school}\n{record}\n{expectationNote}
expectations|evidence|For {school}, assess the {record} result against this context: {expectationNote}
review|local|The final {school} review has a {record} record at its center. {expectationNote}
record|wire|At the end of the schedule, {school} stands {record}. {expectationNote}
expectations|cautious|A record needs context. {school}: {record}. {expectationNote}
review|schedule|The games have their final count for {school}, at {record}. {expectationNote}
record|local|Write {record} beside {school} in the season summary. {expectationNote}
expectations|evidence|That {record} finish is the evidence for the {school} season review. {expectationNote}
review|cautious|No next-season forecast here. {school}'s completed year is {record}. {expectationNote}
`);
add('draft.pick',`
selection|wire|NFL Draft, pick {draftPick}: {playerName} to the {nflTeam}. Round {draftRound}, {school} {position}.
destination|wire|The {nflTeam} select {playerName} from {school}. No. {draftPick}, Round {draftRound}.
round|wire|Round {draftRound} selection — {position} {playerName}, {school}, to the {nflTeam} at No. {draftPick}.
selection|wire|Selected at {draftPick}: {school}'s {playerName}. The {nflTeam} make the Round {draftRound} pick.
destination|wire|{playerName}'s NFL destination is the {nflTeam}, who select the {school} {position} at No. {draftPick}.
round|wire|In Round {draftRound}, the {nflTeam} use pick {draftPick} on {playerName} of {school}.
selection|wire|Draft selection confirmed\n{draftPick}: {playerName}\n{nflTeam}\n{school}, {position}
destination|wire|Off to the {nflTeam}: {position} {playerName} from {school}, selected No. {draftPick} in Round {draftRound}.
round|wire|No. {draftPick} overall, Round {draftRound}. The selection is {playerName} ({school}) for the {nflTeam}.
selection|wire|{position} {playerName} is drafted from {school} by the {nflTeam}. Overall pick: {draftPick}.
destination|wire|The {nflTeam} add {school} {position} {playerName} with the {draftPick} overall pick.
round|wire|Draft board, Round {draftRound}: the {nflTeam} take {playerName}, {school}, No. {draftPick}.
selection|wire|Pick recorded: {draftPick}. Player: {playerName}. Team: {nflTeam}. School: {school}.
destination|wire|For {playerName} of {school}, the draft call is the {nflTeam}, at pick {draftPick} in Round {draftRound}.
round|wire|A Round {draftRound} pick for {school}'s {playerName}: No. {draftPick} to the {nflTeam}.
selection|wire|The pick at {draftPick} is {playerName}, {position}, {school}. Selected by the {nflTeam}.
destination|wire|{nflTeam} selection — {playerName} ({school}), {position}, overall No. {draftPick}.
round|wire|Round {draftRound}, selection {roundPick} in the round: {playerName} to the {nflTeam}. Overall No. {draftPick}, from {school}.
`);
add('beat.heisman',`
race|race|Heisman position update: {playerName} of {school} {rankChange}.
player|development|A Heisman watch note on {school}'s {playerName}: he {rankChange}.
scope|cautious|Current race position, not an award result: {playerName} {rankChange} in the Heisman watch.
race|wire|Heisman board — {playerName}, {school}: {rankChange}.
player|local|For the {school} Heisman watch, {playerName} {rankChange}.
scope|evidence|The verified change on the Heisman board: {playerName} {rankChange}.
race|race|The Heisman update has {school}'s {playerName} listed as follows: {rankChange}.
player|development|Tracking {playerName}'s Heisman candidacy at {school}: {rankChange}.
scope|cautious|No voting explanation to invent here. The Heisman update is that {playerName} {rankChange}.
race|wire|Heisman watch\n{playerName}\n{school}\n{rankChange}
player|local|{playerName} has a place in the Heisman discussion for {school}; the current update is {rankChange}.
scope|evidence|Record the position before judging it: {playerName}, Heisman race, {rankChange}.
race|race|For the next Heisman conversation, the board shows {playerName} {rankChange}.
player|development|The individual award watch at {school} includes {playerName}, who {rankChange} in the Heisman race.
scope|cautious|A race update can change again. At present, {playerName} {rankChange} on the Heisman board.
race|wire|On the current Heisman list: {school}'s {playerName}, {rankChange}.
player|local|Keep {playerName}'s latest Heisman entry beside the {school} notes: {rankChange}.
scope|evidence|Heisman status to report for {playerName}: {rankChange}. School: {school}.
`);
add('roster.destination',`
move|wire|Transfer commitment: {position} {playerName}, from {formerSchool} to {school}.
origin|local|After {formerSchool}, {playerName} has chosen {school} as his transfer destination.
status|cautious|A commitment rather than just interest: transfer {playerName} chooses {school} from {formerSchool}.
move|wire|{school} gets a commitment from transfer {position} {playerName}, previously at {formerSchool}.
origin|development|A change of school ahead for {playerName}: {formerSchool} to {school}, at {position}.
status|evidence|The destination is confirmed as a commitment. {playerName}: {school}, after {formerSchool}.
move|wire|Portal decision\n{playerName}, {position}\n{formerSchool} → {school}
origin|local|From the {formerSchool} roster, {playerName} commits to {school}.
status|cautious|The school choice is {school}. How {position} {playerName} fits there comes later; he transfers from {formerSchool}.
move|wire|Next school for {playerName}: {school}. Previous program: {formerSchool}.
origin|development|Transfer {position} {playerName}'s path now runs from {formerSchool} to a commitment at {school}.
status|evidence|No longer only a leading-school listing: {playerName} has committed to {school}, coming from {formerSchool}.
move|wire|{playerName} picks {school} in the portal. The {position} was at {formerSchool}.
origin|local|A {formerSchool} transfer has a destination: {position} {playerName} to {school}.
status|cautious|Report the move without a playing-time forecast. {playerName} commits to {school} from {formerSchool}.
move|wire|Transfer board update — {playerName}, {position}, committed to {school} from {formerSchool}.
origin|development|The next program for former {formerSchool} player {playerName} is {school}.
status|evidence|What is settled is the school decision. Transfer {position} {playerName} chooses {school} after {formerSchool}.
`);
add('team.win',`
result|announcement|Final: {nickname} win {score} against {opponent}.
pride|pride|A win for everyone behind the {nickname}. {score}. {hashtag}
result|announcement|Result recorded — {nickname}, {score} winners.
pride|pride|Your {nickname}, on the winning side of {score}.
result|announcement|The final score is {score}. The win belongs to the {nickname}. {hashtag}
pride|pride|Proud to share this one with the {nickname} family. A {score} win.
result|announcement|{opponent} matchup complete. {nickname} win, {score}.
pride|pride|Our colors. Our team. A {score} {nickname} win. {hashtag}
result|announcement|In the win column: {nickname}, {score} over {opponent}.
pride|pride|Backed by the {nickname} family. Finished with a {score} win.
result|announcement|Scoreboard\n{nickname}: winners\n{score}
pride|pride|Take a moment for a {nickname} victory. {score}. {hashtag}
result|announcement|Game finished with the {nickname} ahead, {score}.
pride|pride|A good result to share in our colors: {nickname} win {score}.
result|announcement|Winning final — {nickname} {score}, against {opponent}.
pride|pride|For everyone following the {nickname}, here's your {score} victory. {hashtag}
result|announcement|The {nickname} take the {opponent} game, {score}.
pride|pride|Win posted. {score} for the {nickname} family.
`);
add('narrative.game',`
turning-point|wire|{winner} defeats {loser}, {score}. {turningPoint}
performance|development|{detail} The team result: {winner} over {loser}, {score}.
path|film|How the {winner} win unfolded: {turningPoint} Final against {loser}: {score}.
result|local|The final is {score} for {winner} against {loser}. From the game: {detail}
turning-point|evidence|A verified piece of the {winner}–{loser} recap: {turningPoint} {winner} wins {score}.
performance|cautious|The player detail and the result belong together. {detail} {winner} beats {loser}, {score}.
path|film|{turningPoint} That is part of the route to {winner}'s {score} win over {loser}.
result|wire|Recap card — {winner}, {score}, over {loser}. {detail}
turning-point|local|Beyond the {score} final for {winner}: {turningPoint}
performance|development|An individual note from {winner}'s {score} victory over {loser}: {detail}
path|evidence|Use the game evidence for the {winner} recap. {turningPoint} The final was {score} over {loser}.
result|wire|Winner: {winner}. Final: {score}. Opponent: {loser}. {detail}
turning-point|film|The path through this game matters as well as the score. {turningPoint} {winner} over {loser}, {score}.
performance|local|From the {loser} game, one {winner} player note: {detail} The result was a {score} win.
path|cautious|The captured sequence says this: {turningPoint} It ended {winner} {score} over {loser}.
result|wire|{winner} wins; {loser} falls. Score: {score}. {detail}
turning-point|evidence|Put this beside the {winner}–{loser} final: {turningPoint} Final score, {score}.
performance|development|The box score adds an individual angle to {winner}'s win over {loser}. {detail} Final: {score}.
`);
add('rank.shared',`
position|wire,announcement|{pollName}, No. {rank}: {school}. {rankMovement}
movement|local,pride|The ranking update for {school} puts the program at No. {rank}. {rankMovement} {pollName}.
scope|cautious,evidence|A published position, separate from a game prediction: {school}, No. {rank} in the {pollName}. {rankMovement}
position|wire,announcement|Current poll card — {school}, {rank}, {pollName}. {rankMovement}
movement|local,pride|For everyone tracking {school}'s ranking: No. {rank} in the {pollName}. {rankMovement}
scope|race,evidence|The {pollName} provides this point in the race: {school}, No. {rank}. {rankMovement}
position|wire,announcement|{school}\n{pollName}\nNo. {rank}\n{rankMovement}
movement|local,pride|A ranking note beside the {school} name: {rankMovement} Current spot, No. {rank} in the {pollName}.
scope|cautious,evidence|Keep this tied to the published list: {school} is No. {rank} in the {pollName}. {rankMovement}
position|wire,announcement|Poll position recorded: {rank}. Program: {school}. Poll: {pollName}. {rankMovement}
movement|local,pride|{rankMovement} That's the change to follow for {school}, now No. {rank} in the {pollName}.
scope|race,evidence|Before projecting the next list, the current {pollName} entry is {school} at No. {rank}. {rankMovement}
position|wire,announcement|The number beside {school} is {rank} on the {pollName} list. {rankMovement}
movement|local,pride|Follow the {school} poll trail: {rankMovement} The {pollName} placement is No. {rank}.
scope|cautious,evidence|The poll gives a position to report, without explaining the voters: {school}, {rank}, {pollName}. {rankMovement}
position|wire,announcement|Ranking sheet\nProgram: {school}\nPlace: {rank}\n{rankMovement}\n{pollName}
movement|local,pride|On the ranking side of the {school} update, {rankMovement} No. {rank}, {pollName}.
scope|race,evidence|The next poll conversation begins with {school} No. {rank}. {rankMovement} Source: {pollName}.
position|wire,announcement|Listed at {rank} in the {pollName}: {school}. {rankMovement}
movement|local,pride|What changed on the {school} poll entry? {rankMovement} It reads No. {rank}, {pollName}.
scope|cautious,evidence|No reason to invent a voting cause. The {pollName} has {school} No. {rank}. {rankMovement}
position|wire,announcement|A confirmed poll entry: {school}, {pollName}, No. {rank}. {rankMovement}
movement|local,pride|The {school} ranking note starts with this: {rankMovement} New listing: No. {rank}, {pollName}.
scope|race,evidence|For the rankings watch, log No. {rank} for {school} in the {pollName}. {rankMovement}
position|wire,announcement|No. {rank} is {school}'s current {pollName} place. {rankMovement}
movement|local,pride|Here is the {school} ranking update in full: {pollName}, No. {rank}. {rankMovement}
scope|cautious,evidence|Separate the ranking from the reasons people assign to it. {school} sits No. {rank} in the {pollName}. {rankMovement}
position|wire,announcement|From the current {pollName} list: {school}, {rank}. {rankMovement}
movement|local,pride|The ranking line for {school} has an update to carry forward. No. {rank}, {pollName}. {rankMovement}
scope|race,evidence|A poll snapshot for the next discussion: {school}, {rank}, {pollName}. {rankMovement}
`);
// These are shared factual rank frames, not unique perspectives per alias ID.
// Global structure memory recognizes the same frame across every family.
for(const family of ['team.poll_up','beat.poll_same','beat.poll_down','beat.poll_entry'])families[family]=[...(families[family]??[]),...families['rank.shared'].map((t,i)=>({...t,id:`${family}.cot104.rank.${i+1}`,when:['rankMovement']}))];
delete families['rank.shared'];
add('history.team-counter',`
milestone|wire|Program milestone for {school}: {ordinal} {achievement}.
history|local|The {school} history ledger adds a {ordinal} {achievement}.
scope|evidence|A program total to record: {school}, {ordinal} {achievement}.
milestone|wire|{ordinal} program {achievement} reached by {school}.
history|local|A new marker on the {school} timeline: its {ordinal} {achievement}.
scope|cautious|This count is for the program: {school}'s {ordinal} {achievement}.
milestone|wire|History entry — {school}, {ordinal} {achievement}.
history|local|Add this to the {school} record: {ordinal} {achievement} in program history.
scope|evidence|The milestone is a cumulative one for {school}, recording its {ordinal} {achievement}.
milestone|wire|{school}\nProgram milestone\n{ordinal} {achievement}
history|local|Another line in {school}'s program story: a {ordinal} {achievement}.
scope|cautious|Keep the program count separate from a season total. {school}: {ordinal} {achievement}.
milestone|wire|Milestone logged: {school} has its {ordinal} {achievement}.
history|local|For those following {school}'s history, this is the program's {ordinal} {achievement}.
scope|evidence|The historical record for {school} reaches a {ordinal} {achievement}.
milestone|wire|A {ordinal} {achievement} is recorded in {school} history.
history|local|A program-history note on {school}: {ordinal} {achievement} achieved.
scope|cautious|Across its program history, {school} has now reached a {ordinal} {achievement}.
`);
add('official.performance.tag',`
production|announcement|Player spotlight: {playerHandle}. {statLine}.
pride|pride|Proud of this contribution from {playerHandle}: {statLine}. {hashtag}
production|announcement|On the stat sheet\n{playerHandle}\n{statLine}
pride|pride|Give {playerHandle} his part of the spotlight. {statLine}.
production|announcement|Recorded for {playerHandle}: {statLine}.
pride|pride|Our player. His production. {playerHandle}: {statLine}. {hashtag}
production|announcement|Performance entry — {playerHandle}, {statLine}.
pride|pride|A contribution worth sharing with our fans: {playerHandle}, {statLine}.
production|announcement|The individual line for {playerHandle} reads {statLine}.
pride|pride|Put some appreciation behind {playerHandle}. {statLine}. {hashtag}
production|announcement|{statLine}. That's the performance report for {playerHandle}.
pride|pride|Happy to put the spotlight on {playerHandle} for this: {statLine}.
production|announcement|Player report\nName: {playerHandle}\nLine: {statLine}
pride|pride|A name for our fans to celebrate: {playerHandle}, with {statLine}.
production|announcement|From the performance sheet: {statLine} for {playerHandle}.
pride|pride|The work shows on this line for {playerHandle}: {statLine}. {hashtag}
production|announcement|Production noted: {playerHandle} — {statLine}.
pride|pride|This part of the recap belongs to {playerHandle}. {statLine}.
`);
add('cycle.recruit-fit',`
signing|wire|Signed with {school}: {starDetail}{playerName}.
class|development|An incoming-class name at {school} is {playerName}, already signed.
status|cautious|A confirmed signee, with an on-field role still ahead: {playerName}, {school}.
signing|wire|{playerName} is on the {school} signed list.
class|local|For those tracking {school}'s incoming players, {playerName} is signed.
status|evidence|The recruitment has a signed outcome: {playerName} to {school}.
signing|wire|Signee profile — {playerName}, {school}.
class|development|The next {school} roster has a signed-class name to follow in {playerName}.
status|cautious|Signing confirms the school. The future role for {playerName} at {school} is a separate question.
signing|wire|On record as signed: {playerName}, {school}.
class|local|One piece of the {school} class is settled. {playerName} has signed.
status|evidence|This is more than an interest listing: {playerName} is a {school} signee.
signing|wire|{school}\nSigned prospect\n{starDetail}{playerName}
class|development|Watch how {playerName} fits into the coming {school} roster. His signing is confirmed.
status|cautious|No starting-job projection here. {playerName} is a confirmed part of {school}'s signed class.
signing|wire|Signed-player update for {school}: {playerName}.
class|local|The incoming class at {school} includes a signed {playerName}.
status|evidence|For the class record, {playerName} is signed with {school}.
`);
const prospectReference=/(?:\{starDescription\})?(?:\{prospectType\})?(?:\{position\} )?\{playerName\}/;
families['roster.major']=families['roster.commit'].filter(t=>!t.fallback&&!/\{(?:starDescription|prospectType|position)\}/.test(t.body.replace(prospectReference,''))).map((t,i)=>({...t,id:`roster.major.cot104.shared.${i+1}`,body:t.body.replace(prospectReference,'5-star {prospectType}{position} {playerName}'),when:[...(t.when??[]),'fiveStarKnown']}));
add('conversation.player-report',`
perspective|measured|{parentHandle} I appreciate the attention. I still think about it as part of what our team is doing.
question|conversational|{parentHandle} what part are you watching for next from me?
perspective|expressive|{parentHandle} I see the post. More interested in the team than a headline about me.
context|measured|{parentHandle} The numbers are there to assess. I want to keep the team in the discussion too.
question|conversational|{parentHandle} fair to look at the numbers. what would you want to see next?
perspective|expressive|{parentHandle} Appreciate you paying attention. I'd rather let the football do most of the talking.
context|measured|{parentHandle} That is one part of the picture. I have my own work to keep doing.
question|conversational|{parentHandle} everybody sees a different part of it. what stood out to you?
perspective|expressive|{parentHandle} Not much of a headline writer myself. I'll stick to playing.
context|measured|{parentHandle} I read it. Trying to keep an even view of my own game.
question|conversational|{parentHandle} interested in what you want to see from the team alongside that
perspective|expressive|{parentHandle} The attention is appreciated. The team comes first for me.
`);
add('injury.report',`
availability|wire|Injury update: {playerName}, {school}.{injuryDetail} {timetable}
player|local|The {school} availability report includes {playerName}.{injuryDetail} {timetable}
timetable|cautious|{timetable} That is the current injury outlook for {school}'s {playerName}.{injuryDetail}
availability|wire|{school} injury report — {playerName}.{injuryDetail} {timetable}
player|local|An availability note on {playerName} at {school}.{injuryDetail} {timetable}
timetable|cautious|For anyone following {playerName}'s availability: {timetable}{injuryDetail}
availability|wire|Player status update\n{playerName}, {school}{injuryDetail}\n{timetable}
player|local|There is an injury update for {school}'s {playerName}.{injuryDetail} {timetable}
timetable|cautious|The current return estimate concerns {playerName} of {school}. {timetable}{injuryDetail}
availability|wire|Listed on the injury report: {school}, {playerName}.{injuryDetail} {timetable}
player|local|The player in this {school} injury update is {playerName}.{injuryDetail} {timetable}
timetable|cautious|{playerName}'s return outlook is an estimate. {timetable}{injuryDetail}
availability|wire|Availability report filed for {playerName}.{injuryDetail} {timetable}
player|local|Keep {playerName} on the {school} availability watch.{injuryDetail} {timetable}
timetable|cautious|For {playerName}, the injury report carries this outlook: {timetable}{injuryDetail}
availability|wire|From the injury report — {playerName} ({school}).{injuryDetail} {timetable}
player|local|A {school} player to follow for availability is {playerName}.{injuryDetail} {timetable}
timetable|cautious|{timetable} The report is on {playerName} at {school}.{injuryDetail}
availability|wire|Injury status: {school}'s {playerName}.{injuryDetail} {timetable}
player|local|For the {school} notes, an injury update on {playerName}.{injuryDetail} {timetable}
timetable|cautious|The return projection for {playerName}: {timetable}{injuryDetail}
availability|wire|{playerName} appears in the {school} injury update.{injuryDetail} {timetable}
player|local|Following {school}'s personnel? Here is the injury update for {playerName}.{injuryDetail} {timetable}
timetable|cautious|With {playerName} on the injury report, this is the present outlook: {timetable}{injuryDetail}
availability|wire|Injury note — player {playerName}, program {school}.{injuryDetail} {timetable}
player|local|The latest availability detail for {school} concerns {playerName}.{injuryDetail} {timetable}
timetable|cautious|An injury estimate to monitor for {playerName}: {timetable}{injuryDetail}
availability|wire|An injury is reported for {school}'s {playerName}.{injuryDetail} {timetable}
player|local|On the {school} player-status sheet, an injury note for {playerName}.{injuryDetail} {timetable}
timetable|cautious|The injury update for {playerName} includes this return outlook. {timetable}{injuryDetail}
`);
add('cycle.fan-future',`
reflection|optimistic|The {school} season is over and I'm not ready to stop talking football.
question|skeptical|Before I make a prediction for {school}, can we see how the roster looks?
humor|wry|{school} football is done. My ability to log off remains questionable.
reflection|excited|Already looking forward to the next time {school} plays.
question|optimistic|What do you most want to see from {school} next season?
humor|wry|No more {school} games for a while. Apparently I'm supposed to develop another interest.
reflection|optimistic|I can enjoy following {school} without having next year's record picked out.
question|skeptical|What's the one {school} question you want answered before getting optimistic again?
humor|wry|The {school} offseason has begun. I will be acting like every small update is enormous.
reflection|excited|The games are done, but I'm still here for {school}.
question|optimistic|Who else is already thinking about the next {school} season?
humor|wry|My {school} offseason plan is to check for football news and pretend I'm being casual about it.
reflection|optimistic|A little time to reflect on {school}. Plenty to look forward to without rushing to a prediction.
question|skeptical|How different does the {school} roster need to look before your outlook changes?
humor|wry|I would like to skip the part of the calendar without {school} games.
reflection|excited|I'll miss game weeks with the {school} crowd. Already want the next one.
question|optimistic|What is your first offseason watch point for {school}?
humor|wry|The {school} season ended. My group chat is choosing not to acknowledge that.
reflection|optimistic|Following {school} next year will be easier than predicting it right now.
question|skeptical|Can we let the {school} offseason develop before settling the next season in the replies?
humor|wry|Currently between {school} games and searching for a reasonable way to handle that. No luck yet.
reflection|excited|Football being done doesn't make me less of a {school} fan.
question|optimistic|What are you most curious to learn about {school} before the next opener?
humor|wry|The offseason is a long time to spend having {school} opinions without any games to test them.
reflection|optimistic|Taking the {school} offseason one update at a time. The next team will have its own story.
question|skeptical|Which {school} question actually needs answering first: retention, recruiting, or the returning group?
humor|wry|I have reached the stage of the {school} offseason where a schedule is acceptable reading material.
reflection|excited|The {school} games are finished for now. Being a fan isn't seasonal for me.
question|optimistic|Anyone else more curious than confident about the next {school} team?
humor|wry|Every offseason I plan to be normal about {school}. An ambitious annual project.
`);
append('team.win',`
result|announcement|The {opponent} game goes in the {nickname} win column. Final: {score}.
pride|pride|For everyone in our corner — a {score} win for the {nickname}.
result|announcement|The result against {opponent} is a {score} {nickname} victory.
pride|pride|The {nickname} name belongs beside the winning score. {score}. {hashtag}
result|announcement|{score}. Game complete. The {nickname} defeat {opponent}.
pride|pride|Proud to bring a {score} win to the {nickname} family.
result|announcement|Winner on the final sheet: {nickname}. Score against {opponent}: {score}.
pride|pride|A victory in our colors. {nickname}, {score}. {hashtag}
result|announcement|Result of the {opponent} matchup\n{nickname} win\n{score}
pride|pride|Good news for every {nickname} fan: a {score} victory.
result|announcement|Final whistle, winning side: {nickname}. {score} over {opponent}.
pride|pride|A result to celebrate together. The {nickname} win {score}.
result|announcement|Against {opponent}: {nickname}, victorious at {score}.
pride|pride|Thank you for backing the {nickname}. Sharing a {score} win with you.
result|announcement|The {nickname} finish the matchup with a {score} win over {opponent}.
pride|pride|Take this one in, {nickname} fans. {score}, a victory.
result|announcement|A win is posted for the {nickname}: {score} against {opponent}.
pride|pride|The final comes with a {nickname} celebration. {score}. {hashtag}
result|announcement|Final result listed: {nickname} beat {opponent}, {score}.
pride|pride|A {score} win, and a good moment for our {nickname} family.
result|announcement|Completed matchup — {nickname} over {opponent}. Winning score: {score}.
pride|pride|The {nickname} close this game on the winning side. {score}.
result|announcement|Victory confirmed: {nickname}, {score}, opponent {opponent}.
pride|pride|A win to share with everyone who follows the {nickname}. {score}. {hashtag}
`);
append('portal.unsigned-local',`
status|wire|National No. {nationalRecruitRank} {playerName} has no commitment recorded.
evaluation|development|An open school decision remains for {starDetail}{playerName}, nationally ranked {nationalRecruitRank}.
uncertainty|cautious|The school remains undecided for {playerName}. His current national recruit rank is {nationalRecruitRank}.
status|wire|Recruiting board: {playerName}, uncommitted, national No. {nationalRecruitRank}.
evaluation|development|A remaining high school prospect to evaluate: {position} {playerName}, No. {nationalRecruitRank} nationally.
uncertainty|evidence|An uncommitted status still sits beside {playerName}'s name. National ranking: {nationalRecruitRank}.
`);
append('cycle.signed-spotlight',`
signing|announcement|The signed-class sheet for {school} carries {playerName}'s name. {tagHandle}
pride|pride|One of our own signees to introduce: {playerName}. Proud to have {tagHandle} with the {nickname}.
signing|announcement|Introducing {tagHandle}, part of {school}'s signed class.
pride|pride|Our colors have a place in {playerName}'s next chapter. Signed with {school}. {tagHandle}
signing|announcement|Signee on the {school} list: {playerName}. {tagHandle}
pride|pride|A {nickname} welcome for one of our signees, {playerName}. {hashtag}
`);
add('player.performance.Entertainer',`
win|measured|Happy with my part in that one. Happier about the team win.
team|conversational|Everybody likes a good stat line. I like seeing {school} win
humor|expressive|Got the win. I will be smiling about that for a while.
production|measured|{statLine}. Good to put that alongside a win.
team|conversational|The team got what we came for. that's the part I'm enjoying
humor|expressive|A win makes the postgame conversation a lot more fun.
win|measured|A good result for {school}. Enjoyed being part of it.
team|conversational|My numbers are one part. proud of what the group got done
humor|expressive|I enjoy this sport considerably more when we win.
production|measured|Glad I could contribute. {statLine}, with a team win.
team|conversational|Love representing {school}. especially on the winning side
humor|expressive|Today's favorite number is the one in the win column.
win|measured|The result makes this one worth enjoying. Appreciate the support.
team|conversational|I'll take the win with this group over a solo headline
humor|expressive|Winning is a very convincing argument for having a good day.
production|measured|I will take {statLine} as part of a win. Good day for our team.
team|conversational|Plenty of credit to go around for this one. glad to be part of it
humor|expressive|Much easier to come up with a caption after a win.
win|measured|Proud to be on the winning side with {school}.
team|conversational|The {school} win is the first thing on my mind. the personal stuff comes after
humor|expressive|Not pretending to be neutral about a {school} win. Feels good.
production|measured|My contribution was {statLine}. The group got the result.
team|conversational|Glad we could give the {school} people a win to enjoy
humor|expressive|A win and a reason to keep talking football. Works for me.
win|measured|A result to appreciate with my teammates. Enjoying this one.
team|conversational|Being part of a {school} win never gets old to me
humor|expressive|I recommend being on the winning side. Excellent experience.
production|measured|{statLine} is nice to look back on. The win matters more to me.
team|conversational|Good to contribute. better to share the result with this group
humor|expressive|We won, so I will be accepting no criticism of my good mood.
`);
add('history.player-career',`
milestone|wire|Career milestone: {playerName} reaches {threshold} {metric}. Current total: {total}.
player|local|For {school}'s {playerName}, the career count now stands at {total} {metric}, reaching the {threshold} marker.
scope|evidence|This is a career count: {playerName}, {total} {metric}, with the {threshold} threshold reached.
milestone|wire|{threshold} career {metric} reached by {playerName}. The total is {total}.
player|development|An individual milestone at {school}: {playerName} reaches {threshold} career {metric}, now at {total}.
scope|cautious|Keep this separate from a season total. {playerName} has {total} career {metric}, reaching {threshold}.
milestone|wire|Career ledger\n{playerName}\nMarker: {threshold} {metric}\nTotal: {total}
player|local|{playerName} has a career milestone to note for {school}: {threshold} {metric}, with {total} in all.
scope|evidence|Across his career, {playerName}'s {metric} total is {total}. That reaches the {threshold} marker.
milestone|wire|Milestone update — {playerName}, {threshold} career {metric}. Total now {total}.
player|development|The career story for {playerName} adds a {threshold} {metric} milestone. His total: {total}.
scope|cautious|The figure is career-wide: {playerName} has {total} {metric}, meeting the {threshold} milestone.
milestone|wire|Reached: {threshold} career {metric}. Player: {playerName}. Total: {total}.
player|local|A {school} name on the career milestone sheet: {playerName}, {threshold} {metric}, total {total}.
scope|evidence|The milestone is {threshold}; the actual career total is {total}. {metric} for {playerName}.
milestone|wire|{playerName} crosses the {threshold} career {metric} threshold, with a total of {total}.
player|development|Another career marker for {playerName}: {threshold} {metric} reached. The count stands at {total}.
scope|cautious|Read this as a career mark for {playerName}: {total} {metric}, reaching {threshold}.
milestone|wire|Career marker logged: {playerName}, {threshold} {metric}. Current count {total}.
player|local|The {school} career note on {playerName} is a {threshold} {metric} milestone, with {total} accumulated.
scope|evidence|A threshold and its total, kept distinct: {playerName} reaches {threshold} career {metric}; total {total}.
milestone|wire|{threshold} is the career {metric} milestone reached by {playerName}. He is at {total}.
player|development|For the player-history watch, {playerName} has {total} career {metric}, enough to reach {threshold}.
scope|cautious|There is no season-only claim in this figure: {playerName}, {total} career {metric}, {threshold} milestone.
milestone|wire|{playerName}\nCareer {metric}: {total}\nMilestone reached: {threshold}
player|local|The {school} report includes {playerName} reaching a career {metric} milestone of {threshold}. Total: {total}.
scope|evidence|The career record shows {total} {metric} for {playerName}. The {threshold} milestone has been reached.
milestone|wire|Career {metric} milestone for {playerName}: {threshold}, with {total} now recorded.
player|development|An individual career checkpoint: {playerName} reaches {threshold} {metric}. The running total is {total}.
scope|cautious|For {playerName}, {threshold} is the milestone and {total} is the career {metric} count.
`);
add('preseason.honor.beat',`
honor|wire|Preseason honor for {school}'s {playerName}: {award}.
player|local|{playerName} represents {school} on the preseason honors list: {award}.
scope|cautious|The preseason designation for {playerName} is {award}. School: {school}.
honor|wire|Honors board — {playerName}, {school}, {award}.
player|development|An individual to follow at {school}: {playerName}, named {award}.
scope|evidence|For the preseason record, {playerName} has the {award} designation.
honor|wire|{award}: {playerName} of {school}.
player|local|A {school} name among the preseason honorees is {playerName}: {award}.
scope|cautious|A preseason honor to note as the games approach: {playerName}, {award}, {school}.
honor|wire|Preseason list\n{playerName}\n{school}\n{award}
player|development|The preseason attention includes {school}'s {playerName}, selected as {award}.
scope|evidence|The designation is {award}; the player is {playerName}, from {school}.
honor|wire|Named in the preseason honors: {playerName}, {award}.
player|local|For those following {school}, {playerName} has earned a preseason listing as {award}.
scope|cautious|Before the season's results, a preseason honor for {playerName}: {award}.
honor|wire|An honor to log for {school}: {playerName} is {award}.
player|development|{playerName}'s name belongs on the {school} preseason watch after the {award} selection.
scope|evidence|On the preseason honors sheet, the entry reads {playerName}, {school}, {award}.
`);
// Current routing uses personality-specific announcements; make the factual
// decision frames usable there too. These shared bodies are not counted as
// new structures per alias. New personal copy must match the account cadence.
for(const personality of ['Leader','Intense','Entertainer','TeamPlayer','Unpredictable','Neutral']){
 const allowed=personality==='Leader'?['decision','gratitude','brevity']:personality==='TeamPlayer'?['gratitude','decision','pride']:personality==='Intense'?['decision','pride','brevity']:null;
 const family='roster.announce.'+personality;
 families[family]=families['roster.announce'].filter(t=>!allowed||allowed.includes(t.angle)).map((t,i)=>({...t,id:`${family}.cot104.announcement.${i+1}`,style:t.voices[0],when:['personal'+t.voices[0]]}));
}
families['narrative.game'].push(
 {id:'narrative.game.cot104.factual-fallback.1',body:'{winner} defeats {loser}, {score}.',angle:'result',voices:['wire','local','cautious'],fallback:true},
 {id:'narrative.game.cot104.factual-fallback.2',body:'Final score: {score}. {winner} beats {loser}.',angle:'result',voices:['wire','local','cautious'],fallback:true});
families['injury.report'].push(
 {id:'injury.report.cot104.factual-fallback.1',body:'Injury report: {school}\'s {playerName}.',angle:'availability',voices:['wire','local','cautious'],fallback:true},
 {id:'injury.report.cot104.factual-fallback.2',body:'Availability update for {playerName} at {school}.',angle:'availability',voices:['wire','local','cautious'],fallback:true});
for(const lines of Object.values(families))for(const t of lines){
 t.body=t.body.replace('Last place: No.','Previous position: No.').replace('the {draftPick} overall pick','the No. {draftPick} overall pick');
 t.body=t.body.replace('The captured sequence says this:','The game unfolded this way:').replace('A verified piece of the','One part of the').replace('The performance evidence is specific:','The production is clear:')
  .replace('No reason to invent a voting cause. The','The').replace('No voting explanation to invent here. The','The').replace('The verified change on the','The latest change on the')
  .replace('recorded top three','current top three').replace('recorded leaders','current leaders').replace('recorded production','production');
 t.body=t.body.replace('Recorded today:','Career update:').replace("Today's award note",'The award note')
  .replace('There is a new poll, with the same','The current poll retains the same')
  .replace('current return estimate concerns','current availability update concerns').replace("{playerName}'s return outlook is an estimate.",'Current availability information for {playerName}.')
  .replace('The return projection for','Availability information for').replace('An injury estimate to monitor','An injury update to monitor');
 t.body=t.body.replace('The next poll is still ahead.','That result alone does not establish a new poll position.')
  .replace('Poll implications can wait for publication.','A game result and a poll position are separate facts.')
  .replace('No poll movement to report yet. There is a result:','The result itself is confirmed:')
  .replace('The new poll position people are guessing at is not.','A ranking change needs its own published evidence.')
  .replace('weighing a school choice','uncommitted');
 if(t.id.startsWith('reply.fan.player_good.')&&t.voices.includes('rival'))t.when=['rivalPerspective'];
 if(t.id.startsWith('conversation.player-report.'))t.style=t.voices[0];
 if(t.id.startsWith('narrative.game.'))t.when=[...['detail','turningPoint'].filter(k=>t.body.includes('{'+k+'}'))];
 if(t.id.startsWith('narrative.game.')&&t.angle==='performance')t.when.push('individualDetail');
 if(t.id.startsWith('injury.report.')&&t.body.includes('{timetable}'))t.when=['timetable'];
 if(t.body.includes('{recruitRankDetail}'))t.when=[...(t.when??[]),'recruitRankDetail'];
}
const output=new URL('../../Content/Runtime/perspectives-pack.json',import.meta.url);
for(const [family,lines] of Object.entries(families))for(const t of lines){
 if(['cycle.team-welcome','cycle.signed-spotlight'].includes(family)&&!t.body.includes('{tagHandle}'))t.body+=' {tagHandle}';
 t.id=perspectiveId(family,t);
}
fs.writeFileSync(output,JSON.stringify({format:1,revision:'cot-0.1.4-perspectives-1',publicationPolicy:{structuralMemoryEditions:4},families},null,2)+'\n');
console.log(`${Object.keys(families).length} families; ${Object.values(families).flat().length} authored alternatives`);
