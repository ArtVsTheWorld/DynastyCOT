"""One bounded, authored RC016 expansion. No captured game/user data."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
families={}
def add(family,texts,voices=None,angle=None):
    lines=texts.strip().split('\n')
    for i,text in enumerate(lines):
        text=text.strip()
        if not text: continue
        item={'id':family+'.rc016.'+hashlib.sha256(text.encode()).hexdigest()[:16],'body':text,'angle':angle or family.rsplit('.',1)[-1]}
        if voices:item['voices']=voices
        families.setdefault(family,[]).append(item)

# Each topic: twelve fans, six players/recruits, three analysts, three local/official.
casual={
'uniforms':'''A good helmet stripe has done more for my mood than most preseason predictions.
White jerseys on the road. School colors at home. I am willing to be boring about this.
Some of these alternate uniforms need an alternate opinion.
Give me a clean jersey and numbers I can read from the cheap seats.
My uniform rankings have absolutely no relationship to my football knowledge. Neither is improving.
The best throwback is usually the one nobody tried to redesign.
I can support my school and admit the old logo was better.
Chrome helmets are going to start a family argument before kickoff again.
School colors should be visible somewhere on the school uniform. Bold stance, apparently.
Nothing wrong with a simple uniform. The game can provide the unnecessary drama.
A jersey reveal is the one time everybody agrees our season is undefeated.
Somebody loves the uniform you hate. College football is generous like that.
Clean numbers. Clean look. That's my vote.
Still picking school colors over everything.
Good helmets look better with grass on them.
Everybody has uniform opinions until laundry day.
I like a throwback. Doesn't need a speech.
Would wear the old logo tomorrow.
Uniform taste and team quality are separate rankings. It is possible to enjoy one without projecting the other.
Legible numbers are an underrated design choice. Especially when the player is moving.
The strongest visual identity is one you recognize before reading the school name.
School colors mean something to the people wearing them.
The uniform debate is part of the routine here. It rarely ends in consensus.
Logos change. The attachment to them tends to last.''',
'traditions':'''A stadium tradition doesn't have to make sense to outsiders. That's half the appeal.
The band is part of the game. I will not be taking questions.
You can tell me the tradition is silly. I already know. I'm doing it anyway.
Every school has one ritual that sounds made up until the whole section joins in.
Visiting fans judging our traditions should remember we can see theirs too.
The fight song works on me every time. Terrible news for anyone sitting nearby.
I respect a tradition that survives several coaching searches.
College football needs more marching band and fewer opinions about how much marching band is too much.
Explaining a pregame ritual out loud is how you discover how strange this sport is.
Our mascot could lose every argument and I'd still defend it.
No neutral opinion about a school bell, a cannon, or a live animal has ever survived kickoff.
One day I'll hear our fight song and act normal. Probably not soon.
Band starts up and the day feels different.
Same song. Still gets me.
Traditions hit harder when you're part of them.
The little routines matter.
I get why people come back for this.
Pregame feels like home.
Tradition is a real part of the appeal of this sport, even when it cannot explain a single play.
A school's identity is larger than its latest record. The familiar rituals help carry it through losing seasons.
The interesting traditions are often the ones a visitor needs explained.
Some things belong to everyone who has worn these colors.
The rituals around the game help connect one class to the next.
School pride does not need a new slogan every season.''',
'tailgating':'''Tailgate rankings should include how quickly somebody offers you a plate.
The person who remembers extra chairs is the real five-star recruit.
A folding table and a suspicious amount of food. Excellent Saturday architecture.
My tailgate contribution is optimism. People keep requesting ice instead.
Nobody has ever overestimated how much ice a tailgate needs.
There is always one person protecting the grill like a defensive coordinator on fourth down.
If the parking-lot argument reaches strength of schedule, refill your plate. This will take a while.
The most convincing preseason pitch is still somebody's barbecue.
Bringing a rival to the tailgate is fine. Letting them control the music is reckless.
A good breakfast tailgate deserves its own ranking system.
The final cooler is always heavier than the person carrying it expected.
Every tailgate has a coach. Usually the person standing nearest the food.
Save a plate for after.
Football weekends smell different.
Family, food, football. Easy list.
Appreciate everybody who makes the trip.
The people outside the stadium make it special too.
Got to respect an early setup.
The tailgate is one reason college football travels so well. Visitors can find a welcome before the rivalry starts.
The best home atmosphere starts well before the gates open.
There is more to a football weekend than the final score. The parking lots are a useful reminder.
Game weekends bring a lot of people home.
The familiar faces outside the stadium are part of the season.
A football Saturday takes more than the people on the field.''',
'campus':'''Football weather and a walk across campus. I can build a whole personality around that.
The walk to the stadium is undefeated in the offseason imagination rankings.
Every campus has a shortcut that saves thirty seconds and requires an entire explanation.
Campus parking is the one opponent nobody has solved.
Autumn on campus does a lot of work for the recruiting brochure.
The stadium looks different when you pass it on a normal weekday.
Buying another school sweatshirt is apparently my long-term financial plan.
I want the campus tour that includes where everybody actually eats.
Being back on campus immediately restores my confidence that I could still make an 8 a.m. class. Incorrectly.
College football makes you sentimental about sidewalks. Ridiculous sport.
There is no reasonable limit to how long somebody will talk about their old dorm.
The best campus building is whichever one gets you out of the rain first.
Class first. Then football.
Campus finally feels familiar.
Long day. Good place to be.
Finding a routine matters.
The walk back gives you time to think.
Appreciate the people who check in.
For a recruit, the school still has to work on an ordinary Tuesday. The stadium is only part of the choice.
Campus fit is difficult to reduce to a ranking. Players have lives beyond the depth chart.
A visit is useful because a campus is more specific than a logo.
The connection to this place extends beyond Saturdays.
An ordinary day on campus is part of the story too.
The school is bigger than its football schedule.''',
'conference':'''Conference pride is a complicated way to admit you watched your rival's game.
I support conference strength right up until a particular school benefits from it.
The conference argument always ends with somebody opening a spreadsheet nobody asked for.
Rooting for a league is fine. Rooting for our rival requires a written explanation.
Every conference has the best road trip according to somebody who has only taken one.
Conference debates would improve if people admitted which games they actually watched.
Geography still belongs somewhere in a conference conversation. Possibly near the start.
I can dislike a school and still admit the road environment is good. Painful growth.
The league logo does not play defense. This keeps getting overlooked.
One strong team isn't a whole conference. One bad Saturday isn't one either.
Conference pride gets very selective around rivalry time.
My conference is difficult. Your conference is overrated. Glad we settled that scientifically.
Every league game counts.
Conference football feels personal.
Different stadium. Same job.
Learn the opponent. Respect the work.
Can't borrow anybody else's wins.
The league schedule tells you plenty.
Conference comparisons need more than the best team from each league. The middle matters too.
One head-to-head result is evidence, not an entire league ranking.
Travel and geography shape a conference schedule even when they never appear in the standings.
League games connect familiar schools and unfamiliar challenges.
Conference weekends give fans plenty to compare.
Rivalry and conference pride rarely fit in the same sentence comfortably.''',
'rivalries':'''Rivalry records are important until somebody uses the version I don't like.
Our rival could change its logo to a blank square and I would dislike the square.
The best part of a rivalry is having a completely unreasonable reason to watch another game.
No, I have not moved on from that rivalry argument. I have organized it.
Rivalry week turns people with mortgages into comment-section detectives.
The polite version of my rivalry opinion is also impolite.
We disagree about everything except how much this game matters.
Some rivalry jokes have been passed down longer than the family furniture.
Nobody remembers a rival's record more accurately than somebody claiming not to care.
An annual grudge is a perfectly reasonable item to put on a calendar.
There is no neutral way to pronounce a rival's school name in this house.
I respect the rivalry. That is the full extent of today's generosity.
Some games need fewer words.
Everybody knows what this one means.
Rivalry respect. Competitive mindset.
The week feels different.
You hear about these games early.
Let the football speak.
Rivalry stakes do not disappear because the teams have different records.
Familiar opponents create familiar assumptions. It is still worth looking at the current teams.
The history explains the attention. It does not decide the next result.
Some dates carry their own significance.
School pride is especially visible when the familiar opponent comes around.
Rivalries connect the current team to generations of fans.''',
'arguments':'''Punting can be correct and still make me personally unhappy.
Running the ball is an identity until it's third and long. Then everybody has ideas.
The backup quarterback has a remarkable approval rating before his first snap.
A good fullback block deserves a replay and a respectful silence.
The best stadium seat is the one where nobody starts explaining what you just watched.
Calling every fourth-down decision obvious after the result is a wonderful hobby.
I enjoy a defensive game until it's my offense being stopped.
The most popular offensive system is the one somebody else is currently winning with.
A long snapper deserves to stay anonymous for all the right reasons.
Time of possession arguments should at least mention what happened with the possession.
Every football argument eventually becomes a debate about a game from childhood.
I trust the person who admits they don't know the coverage more than the person yelling about effort.
Blocking is football too.
Special teams people know.
Film makes simple opinions complicated.
One snap at a time is practical advice.
Everybody has a role.
Good football isn't always loud.
Result and decision are different questions. A failed conversion does not automatically make the decision poor.
An offensive identity is useful when it fits the roster. It is less useful as a substitute for evaluating the roster.
Small samples create confident arguments. That does not make the samples larger.
Every unit contributes to the final result.
The quiet assignments matter as much as the highlight.
Football gives people plenty to discuss between games.''',
'optimism':'''Preseason optimism is a renewable resource and I refuse to conserve it.
I can be realistic tomorrow. Today I am looking at the schedule with dangerous confidence.
Supporting a school means knowing exactly why this might go wrong and showing up anyway.
The offseason is when every depth-chart question becomes a promising opportunity.
My expectations are reasonable. My reaction to the first good drive will not be.
Cautious optimism is still optimism. I'm counting it.
I have learned from past seasons. I have chosen to ignore the lessons selectively.
A quiet week without football drama would be nice. I would immediately get suspicious.
The same logo can cause hope and stress at the same time. Efficient branding.
Somebody has to believe before there's evidence. Apparently that's my assignment again.
You can care a lot and still laugh at how much you care.
Not every season needs a grand prediction. Sometimes you just want the next game.
Ready for the work.
Small progress still counts.
Keep the people close.
Another chance to get better.
Good days don't happen alone.
Looking forward to what's next.
Expectations should leave room for uncertainty. That is especially true before the first meaningful sample.
A developing team can improve without following a straight line.
Optimism and evidence can coexist, provided the first does not get mistaken for the second.
A new week gives the team a new task.
There is room to appreciate progress while expecting more.
The people following this program care deeply. That part is clear.'''
}
for topic,block in casual.items():
    lines=block.splitlines();assert len(lines)==24,(topic,len(lines))
    for role,subset,voices in [('fan',lines[:12],['excited','skeptical','wry','optimistic','rival']),('personal',lines[12:18],['measured','conversational','expressive']),('analyst',lines[18:21],['wire','race','film','evidence','schedule','development']),('local',lines[21:],['wire','local','cautious','announcement','pride'])]:
        family='casual.'+topic+'.'+role
        add(family,'\n'.join(subset),voices,topic)
        for i,item in enumerate(families[family]):item['voices']=[voices[i%len(voices)]]

seasonal={
'halloween':'''The scariest costume is somebody confidently explaining the playoff picture.
Halloween candy rankings and football rankings deserve equally unreasonable arguments.
No haunted house can compete with watching your team protect a small lead.
A pumpkin in school colors is festive. A rival logo on it is a warning.
October 31. Time to pretend the football stress is part of the costume.
The candy bowl has more depth than some of our message-board arguments.
Halloween is a good reminder that orange is not automatically a neutral color.
Nothing frightening about a normal football weekend. Except all of it.
Happy Halloween to everyone wearing these colors.
Wishing our school community a happy and safe Halloween.
October 31 brings a little extra school spirit.
Enjoy Halloween, wherever you are following the team.
Happy Halloween. Save some candy.
Football and Halloween. Good combination.
October went fast.
Happy Halloween to everybody back home.''',
'rivalry':'''Week 13. Time for the annual family argument with a football schedule.
Rivalry Week: when old scores become current events again.
I hope our rival has a pleasant week with one very specific exception.
Week 13 brings out the historian in people who cannot remember their passwords.
The rivalry jokes are recycled. The dislike is fresh.
Rivalry Week is not the time to ask this family to be normal.
Some people prepare for Week 13 by studying matchups. I prepare by remembering grievances.
Rivalry Week. Nobody is neutral and everybody has receipts.
Week 13 is here. School pride comes with the territory.
Rivalry Week connects this team with everyone who has worn the colors.
Familiar opponents. A meaningful week for our fans.
Our school community knows what Rivalry Week means.
Week 13. Focus stays on the work.
Rivalry Week has a different feel.
The people back home know what week it is.
Keep the preparation right this week.''',
'thanksgiving':'''Thankful for family, food, and a remote nobody is hiding.
Thanksgiving football arguments count as conversation. My family has ruled on this.
Pass the potatoes before explaining why the conference is overrated.
The fourth Thursday in November deserves a plate and a full battery in the remote.
Thankful for the people who understand that kickoff affects dinner logistics.
Thanksgiving means somebody is making a playoff argument while holding a serving spoon.
Good food, familiar people, unreasonable football opinions. Happy Thanksgiving.
I can disagree with your rankings and still pass you the rolls.
Happy Thanksgiving to our school community and everyone following the program.
Thankful for the people who support this team throughout the year.
Wishing our fans and their families a happy Thanksgiving.
Grateful for the community behind these colors. Happy Thanksgiving.
Thankful for my people.
Happy Thanksgiving to everybody back home.
Family means a lot. Happy Thanksgiving.
Grateful for another year with the people I love.''',
'christmas':'''Merry Christmas. May the football arguments wait until after breakfast.
Somebody got school gear again. It was me. I am delighted.
Christmas wish list: peace, good health, and fewer preventable penalties.
The school sweatshirt under the tree never gets old.
December 25. An excellent day to let somebody else hold the remote.
Merry Christmas to everyone whose holiday decorations include a team logo.
Nothing says family like explaining the bowl schedule for the third time.
Football can wait long enough to thank the people making today happen.
Merry Christmas to our fans, families, and school community.
Wishing everyone following the program a peaceful Christmas.
Thank you for sharing the year with this team. Merry Christmas.
Warm Christmas wishes from our school community to yours.
Merry Christmas to my family and everybody back home.
Hope everybody gets some time with their people.
Merry Christmas. Appreciate you all.
Family time matters. Merry Christmas.''',
'newyear':'''New year. Same school. Several carefully revised football opinions.
My resolution is to react reasonably to football. A useful annual joke.
Happy New Year to the people who understand why the remote is important.
December becomes January and the football arguments somehow keep their eligibility.
New calendar, familiar colors. Happy New Year.
Here's to another year of caring far too much about a sport.
Some resolutions are difficult. Supporting this school is easy.
Happy New Year. May your snacks last longer than your confident predictions.
Happy New Year to everyone who supports the program.
Thank you for being part of our school community. Welcome to a new year.
Wishing our fans and families a healthy and happy New Year.
A new calendar year begins with appreciation for the people behind this team.
Happy New Year. Grateful for my people.
New year. Keep growing.
Wishing everybody a good year ahead.
Another year to appreciate the opportunity.'''
}
for holiday,block in seasonal.items():
    lines=block.splitlines();assert len(lines)==16
    for role,subset in [('fan',lines[:8]),('official',lines[8:12]),('personal',lines[12:])]:add('calendar.'+holiday+'.'+role,'\n'.join(subset),angle=holiday)

# Each reference has one concise verified factual clause; surrounding comments are opinions.
# Provenance is shipped as text metadata, not scraped source text.
historical=[
('boise','https://collegefootballplayoff.com/sports/2021/9/7/bcs-history','Boise State beat Oklahoma 43–42 in the 2007 Fiesta Bowl',[
"Boise State 43, Oklahoma 42. The 2007 Fiesta Bowl remains an excellent argument for staying up.",
"The 2007 Fiesta Bowl ended Boise State 43, Oklahoma 42. I still enjoy saying that score.",
"Boise State's 43–42 win over Oklahoma in the 2007 Fiesta Bowl is why reputation cannot settle a matchup.",
"From the archive: Boise State defeated Oklahoma 43–42 in the 2007 Fiesta Bowl."]),
('texas','https://collegefootballplayoff.com/sports/2021/9/7/bcs-history','Texas beat USC 41–38 in the 2006 Rose Bowl',[
"Texas 41, USC 38 in the 2006 Rose Bowl. Some scores come with their own volume control.",
"The 2006 Rose Bowl finished Texas 41, USC 38. A reasonable use of an entire evening.",
"Texas's 41–38 victory over USC in the 2006 Rose Bowl still belongs in any great-game discussion.",
"Archive note: Texas beat USC 41–38 in the Rose Bowl played in January 2006."]),
('ohio','https://collegefootballplayoff.com/sports/2021/9/7/bcs-history','Ohio State beat Miami31–24 in two overtimes in2003FiestaBowl',[
"Ohio State 31, Miami 24 after two overtimes in the 2003 Fiesta Bowl. Nobody needed a calm ending, apparently.",
"The 2003 Fiesta Bowl took two overtimes: Ohio State 31, Miami 24. Some nights demand extra snacks.",
"Ohio State's 31–24, double-overtime win over Miami in the 2003 Fiesta Bowl is a reminder that finals do not need style points.",
"From the archive: Ohio State defeated Miami 31–24 in two overtimes in the 2003 Fiesta Bowl."]),
('nebraska','https://espnpressroom.com/press-release/espn-celebrates-college-footballs-150th-birthday-by-unveiling-sports-150-greatest-games-amplifying-season-long-cfb150-initiative/','Nebraska beat Oklahoma35–31 onNovember25,1971',[
"Nebraska 35, Oklahoma 31 in 1971. The old highlights still hold up their end of the argument.",
"The 1971 Nebraska–Oklahoma game finished 35–31 for Nebraska. An excellent reason to listen to the older fans.",
"Nebraska's 35–31 win over Oklahoma in 1971 is a useful reminder that great football predates our favorite era.",
"Archive score: Nebraska 35, Oklahoma 31, November 25, 1971."]),
('miami','https://espnpressroom.com/press-release/espn-celebrates-college-footballs-150th-birthday-by-unveiling-sports-150-greatest-games-amplifying-season-long-cfb150-initiative/','Miami beat Nebraska31–30 in1984OrangeBowl',[
"Miami 31, Nebraska 30 in the 1984 Orange Bowl. One point can occupy a very large part of football history.",
"The 1984 Orange Bowl ended Miami 31, Nebraska 30. The old bowl arguments have staying power.",
"Miami's 31–30 win over Nebraska in the 1984 Orange Bowl remains a case study in how thin the final margin can be.",
"From the archive: Miami defeated Nebraska 31–30 in the 1984 Orange Bowl."]),
('bc','https://espnpressroom.com/press-release/espn-celebrates-college-footballs-150th-birthday-by-unveiling-sports-150-greatest-games-amplifying-season-long-cfb150-initiative/','BostonCollege beat Miami47–45 onNovember23,1984',[
"Boston College 47, Miami 45 in 1984. Defense did not get the last word that day.",
"The 1984 Boston College–Miami game finished 47–45 for BC. That's enough scoring for a family argument.",
"Boston College's 47–45 win over Miami in 1984 shows why the final score can survive far beyond a season.",
"Archive score: Boston College 47, Miami 45, November 23, 1984."]),
('lsu','https://lsusports.net/news/2020/01/13/lsu-football-national-championship-game-recap','LSU won2019nationaltitle42–25 overClemson onJanuary13,2020',[
"LSU closed its 2019 season by beating Clemson 42–25 for the title. Some highlight playlists do not require skipping.",
"The 2019 LSU title team finished with a 42–25 win over Clemson. A pretty convincing closing argument.",
"LSU's 42–25 title-game win over Clemson capped its 2019 season. It remains a useful benchmark for an offense discussion.",
"Archive note: LSU won the 2019 national title with a 42–25 victory over Clemson in January 2020."]),
('georgia','https://georgiadogs.com/news/2022/1/10/football-bulldogs-capture-national-title-with-33-18-win-over-alabama','Georgia beat Alabama33–18 for2021seasonnationaltitle January10,2022',[
"Georgia 33, Alabama 18 for the 2021 season's title. Some scoreboards deserve to become wall art.",
"Georgia finished its 2021 season with a 33–18 title-game win over Alabama. A good night to remember.",
"Georgia's 33–18 victory over Alabama for the 2021 national title is a reminder that a season gets its own ending.",
"From the archive: Georgia defeated Alabama 33–18 in January 2022 to win the 2021 season's national title."]),
('tennessee','https://utsports.com/news/2018/8/14/football-1998-season-story-needs-title','Tennessee beat FloridaState23–16 January4,1999 tocomplete1998title season',[
"Tennessee's 1998 season ended with a 23–16 title-game win over Florida State. Orange looks good in an old highlight.",
"The January 1999 Fiesta Bowl finished Tennessee 23, Florida State 16. Vols fans have good reason to save that one.",
"Tennessee's 23–16 victory over Florida State completed its 1998 national-title season. The calendar-year distinction matters.",
"Archive note: Tennessee beat Florida State 23–16 in January 1999 to cap its 1998 national championship season."]),
('appstate','https://broncosports.com/news/2025/9/24/football-the-power-of-victory','AppalachianState beat Michigan34–32 in2007',[
"Appalachian State 34, Michigan 32 in 2007. Yes, people are still going to bring it up.",
"The 2007 Appalachian State–Michigan score was 34–32 for App State. College football has a very long memory.",
"Appalachian State's 34–32 win over Michigan in 2007 is a fair reminder to evaluate the opponent on the field.",
"From the archive: Appalachian State defeated Michigan 34–32 in September 2007."]),
('cal','https://espnpressroom.com/press-release/the-band-is-on-the-field-espn-e60-relives-memorable-cal-stanford-play-on-40th-anniversary/','Cal usedfive laterals onwinning kickoffreturn vsStanford in1982',[
"The 1982 Cal–Stanford finish involved five laterals and the band. This sport is allowed to be that strange.",
"Cal's five-lateral finish against Stanford in 1982 remains a strong case for keeping the band off the field until it's over.",
"The five laterals in Cal's winning 1982 kickoff return against Stanford are a reminder that possession rules matter until the play ends.",
"Archive note: Cal's winning kickoff return against Stanford in 1982 included five laterals."]),
('ringo','https://collegefootballplayoff.com/sports/2022/1/20/ncg-recap-2022','KeleeRingo returnedinterception79yards in2022titlegame',[
"Kelee Ringo's 79-yard interception return in the January 2022 title game. Georgia fans have a very reasonable favorite replay.",
"The Ringo interception return went 79 yards in the 2022 championship game. A long way to carry a fanbase's nerves.",
"Kelee Ringo's 79-yard title-game interception return in January 2022 is an example of one defensive play becoming the lasting image.",
"From the archive: Kelee Ringo returned an interception 79 yards in the January 2022 national championship game."])
]
for name,url,fact,lines in historical:
    for role,subset in [('fan',lines[:2]),('analyst',lines[2:3]),('beat',lines[3:])]:add('archive.'+name+'.'+role,'\n'.join(subset),angle='historical-'+name)

context={
'world.upset':'''Unranked {winner} beat {loserLabel}, {score}. The ranking did not survive the matchup.
{loserLabel} had the number beside its name. {winner} got the {score} win.
The result is {winner} over {loserLabel}, {score}. That deserves a closer look than the logos.
A ranked opponent beaten: {winner} takes down {loserLabel}, {score}.
{score} for {winner} against {loserLabel}. Reputation was not enough for the other side.
{winner} leaves with a {score} victory over {loserLabel}. The winning team earned the attention.
One result, very different reactions: {winner} beat {loserLabel}, {score}.
{loserLabel} lost to {winner}, {score}. The winner gets the credit; the ranked team gets the questions.''',
'reply.fan.upset':'''{loserLabel} on the other side. {winner} on the right side of {score}. I'm saving this one.
Nobody can tell us {loser} didn't count after looking at that ranking all week.
{winner} beat {loserLabel}. Going to enjoy typing that for a while.
Respectfully, the {loserLabel} part makes the {score} part even better.
We get to be unreasonable about beating {loserLabel} for at least the rest of today.
The scoreboard says {score}. I would like the people who only discussed the logos to read it.
That ranking next to {loser} makes a very nice souvenir.
{winner} won. Against {loserLabel}. Let us have the noise.''',
'reply.fan.upset-loss':'''Losing to {winner}, {score}, with that ranking beside our name. There isn't a comforting way to arrange it.
Credit {winner}. Still expect more from {loser} than this result.
{score} against {winner}. Please spare me the argument about which program is bigger.
The ranking was {loser}'s. The win was {winner}'s. That is the problem.
I'm not calling {winner} lucky because I don't like the final score.
We can congratulate {winner} and be furious about {loser} losing. Both fit.
{loserLabel} losing this game is going to follow us. It should.
{winner} earned it. {loser} has questions to answer on the field.''',
'context.upset-rival':'''Our rival {loser} just lost to {winner}, {score}. A perfectly normal thing to mention repeatedly.
{winner} beat {loserLabel}. As a rival, I am handling this information with no dignity.
I respect what {winner} did. I also deeply enjoy who they did it to.
{score} against {loser}. The rivalry group chat has received the update.
{winner} handed our rival a loss. Excellent public service.
No analysis from me. Just the {winner}–{loser} result and a smile.
{loserLabel} lost. To {winner}. The jokes are going to write themselves.
Congratulations to {winner}. Our rivalry takes care of the rest.''',
'context.upset-expectation':'''The program-strength gap favored {loser}. The {score} result favored {winner}. That is a meaningful upset, not just a ranking change.
{winner} beat a ranked {loser} despite the difference in program strength. The winner deserves its own story.
This wasn't simply two nearby rankings trading places: {winner} beat the stronger-profile {loserLabel}, {score}.
The ranking and program profiles both pointed toward {loser}. {winner} won anyway.
{loserLabel} had advantages on paper. {winner} has the verified {score} result.
The gap between these programs helps explain the reaction to {winner}'s {score} win. It does not diminish the win.
Judge {winner}'s performance by the opponent it actually beat: {loserLabel}. The program gap adds context.
{winner} over {loserLabel}, {score}, deserves attention beyond a routine recap because the program profiles were so different.''',
'reply.fan.beat_win':'''I'll take the {score} against {opponent}. Plenty of room to discuss the rest tomorrow.
{opponent} is the opponent we beat. No need to turn it into somebody else's game.
That {score} looks pretty good from this side.
Winning against {opponent} makes the week easier to start.
The report can be calm. My reaction to beating {opponent} will take longer.
{opponent} made us earn a result. Happy to have it.
The final score is enough for me today: {score}.
Good to read a recap of a win over {opponent}. Simple pleasure.''',
'reply.fan.beat_loss':'''The {score} against {opponent} is the part I keep coming back to.
Credit {opponent}. We still have to live with our side of the result.
Reading it again did not improve the {score}.
That's a loss to {opponent}. Dressing it up won't change the week.
I can accept the report. Enjoying the result is a separate issue.
{opponent} got the result. We need a better answer next time.
The facts are fair. That doesn't make {score} less frustrating.
No argument with the recap. Plenty of frustration with the football.''',
'reply.fan.analyst_high':'''That {school} detail is useful. I'd rather see the evidence than another slogan.
Good to see what the {school} numbers actually support.
That's a specific reason to feel better about {school}. I'll take specific.
The detail matters more than calling {school} improved without explaining why.
I like the {school} argument. Want to see it hold up over more games.
Useful context for {school}. It doesn't have to become a prediction.
That's worth watching with {school}; no need to declare the whole season solved.
I can be encouraged by that and still want a bigger sample.''',
'reply.fan.analyst_low':'''The {school} concern is fair. I'd like the football to make the next version less painful.
That is more useful than saying {school} just needs to want it more.
The numbers give {school} something specific to answer.
Not the {school} detail I wanted to read. Still a real detail.
Fair concern about {school}. How it changes matters more than defending the old result.
I'd argue with the {school} take if I had a better explanation for the evidence.
That belongs in the {school} discussion. It isn't fixed by pretending it didn't happen.
Specific problem. Now {school} needs a specific improvement.''',
'reply.fan.commit':'''{playerName} choosing {school} is news I can use.
Welcome to {school}, {playerName}. Glad that decision went our way.
The {position} room has another name to follow now.
Happy to see {playerName} in the {school} class.
That's a {position} addition. Curious to follow the development from here.
{playerName} made the choice. Now we get to follow the next part.
Good news for the class. Welcome, {playerName}.
No prediction needed today. Just glad {playerName} picked {school}.''',
'reply.fan.award':'''{playerName} earned the {award} recognition. Good to see the name there.
That {award} goes nicely with the work {playerName} put on the field.
An award for {playerName}. I'll gladly pass that report along.
Glad the {award} voters noticed {playerName} too.
{playerName} gets the recognition. {school} fans get to enjoy it.
That's the {award}. Worth taking a moment for {playerName}.
Well deserved recognition for {playerName}.
The {award} belongs to {playerName}. Good news to open the feed with.''',
'conversation.player-report':'''Appreciate the attention. The next job still needs doing.
I read it. Back to the work now.
Good or bad, the film gives me something to learn.
Appreciate everybody watching. Still have things to improve.
The report is out there. My focus is what I can do next.
Keep learning from it.
There's always another detail to clean up.
One week doesn't finish the work.'''
}
for family,block in context.items():assert len(block.splitlines())==8;add(family,block,angle='context-'+family.rsplit('.',1)[-1])

# Simulated results are visibly labelled, never passed off as captured native HS stats.
recruit={
'qb':'''Modeled HS game: {passYards} passing yards, {passTD} passing TD. Working on the decisions between the highlights.
{passYards} through the air in the modeled HS game. {passTD} passing TD. Plenty to learn from the rest.
Modeled HS game line: {passYards} pass yards and {passTD} TD. Film work next.
Putting the modeled HS line here: {passYards} passing yards, {passTD} TD. Keep improving.
Modeled HS film: {passYards} passing yards. Looking at the reads, not just the throws that worked.
The modeled HS tape has {passTD} passing TD. Want to clean up the decisions around them.
Modeled HS film is up: {passYards} through the air. Watching the feet on every drop.
{passYards} passing yards in the modeled HS game. The throws between the highlights are on the tape too.''',
'runner':'''Modeled HS game: {rushYards} rushing yards, {rushTD} rushing TD. Appreciate the people blocking.
{rushYards} on the ground in the modeled HS game. {rushTD} rushing TD. Keep the legs going.
Modeled HS line: {rushYards} rush yards, {rushTD} TD. Not getting those alone.
Putting the modeled HS game here: {rushYards} rushing yards and {rushTD} TD.
Modeled HS film: {rushYards} rushing yards. Looking at the cuts that didn't need the extra step.
The modeled HS tape is about the reads too. {rushYards} on the ground this game.
{rushTD} rushing TD in the modeled HS game. The blocking is on the tape with them.
Modeled HS film up. {rushYards} rushing yards. Protect the ball and find the right lane.''',
'receiver':'''Modeled HS game: {catches} catches, {receiveYards} yards, {receiveTD} receiving TD.
{catches} for {receiveYards} in the modeled HS game. {receiveTD} TD. Keep the routes right.
Modeled HS line: {receiveYards} receiving yards on {catches} catches. Appreciate the throws.
Posting the modeled HS game: {catches} catches, {receiveYards} yards and {receiveTD} TD.
Modeled HS film: {catches} catches. Watching the routes where the ball went somewhere else too.
{receiveYards} receiving yards in the modeled HS game. The releases are what I'm looking at first.
Modeled HS tape up. {catches} catches, but every route still has a job.
{catches} catches in the modeled HS game. Working on the little space at the top of the route.''',
'line':'''Modeled HS game: {blocks} credited blocks. Keep the feet moving.
{blocks} credited blocks on the modeled HS line. Work starts before the contact.
Modeled HS game posted. {blocks} credited blocks. Appreciate the guys next to me.
The modeled HS line has {blocks} credited blocks. Still finding details to clean up.
Modeled HS film: {blocks} credited blocks. Looking at hand placement first.
{blocks} credited blocks in the modeled HS game. The footwork is all on the tape.
Modeled HS tape up. {blocks} credited blocks. Finish the assignment before hunting a highlight.
The modeled HS game gave me {blocks} credited blocks to review. Pad level still matters.''',
'defense':'''Modeled HS game: {tackles} tackles, {sacks} sacks. Keep finding the ball.
{tackles} tackles in the modeled HS game. {sacks} sacks. Every rep has a job.
Modeled HS line: {tackles} tackles and {sacks} sacks. More work on the tape.
Putting the modeled HS game here: {tackles} tackles, {sacks} sacks.
Modeled HS film: {tackles} tackles. Eyes and angles are the first things I'm checking.
{sacks} sacks in the modeled HS game. Watching the reps that didn't get there too.
Modeled HS tape is up. {tackles} tackles. Got to keep the pursuit right.
The modeled HS line says {tackles} tackles. The film says more than the count.''',
'specialist':'''Modeled HS game: {kickMade}/{kickAttempts} field goals. The routine matters.
{kickMade} of {kickAttempts} field goals in the modeled HS game. Back to the steps.
Modeled HS line: {kickMade}/{kickAttempts} on field goals. Every attempt gets reviewed.
Posting the modeled HS game: {kickMade} field goals on {kickAttempts} attempts.
Modeled HS film: {punts} punts averaging {puntAverage} yards. Looking at contact and the approach.
The modeled HS tape has {punts} punts. Same routine on every one.
{puntAverage} yards per punt in the modeled HS game. Watching the plant foot before anything else.
Modeled HS film up. {punts} punts. The operation is part of it too.'''
}
for group,block in recruit.items():assert len(block.splitlines())==8;add('recruit.game.'+group,block,angle='modeled-hs-'+group)
for i,item in enumerate(families['recruit.game.specialist']):item['when']=['isKicker' if i<4 else 'isPunter']
grouped={}
for family,lines in families.items():
    target=family
    if family.startswith('casual.') or family.startswith('calendar.'):
        target,role=family.rsplit('.',1)
        for item in lines:item['when']=['role'+role.title()]
    elif family.startswith('archive.'):
        _,moment,role=family.split('.');target='archive.reference'
        for item in lines:item['when']=['archive_'+moment,'role'+role.title()]
    grouped.setdefault(target,[]).extend(lines)
families=grouped
assert sum(map(len,families.values()))==464
pack={'format':1,'revision':'cot-0.1.6-beyond-field-1','boundedExpansion':{'casual':192,'historical':48,'seasonal':80,'contextual':96,'recruit':48},'families':families}
target=ROOT/'Content/Runtime/beyond-field-pack.json';target.write_text(json.dumps(pack,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
provenance={'format':1,'description':'Authored occasional archive references; opinions are not new Dynasty events. All referenced events precede 2026.','references':[{'key':name,'source':url,'verifiedFact':fact} for name,url,fact,_ in historical]}
(ROOT/'Content/Runtime/archive-provenance.json').write_text(json.dumps(provenance,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps({'alternatives':sum(map(len,families.values())),'families':len(families),'pack':str(target)}))
