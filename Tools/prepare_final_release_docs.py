from pathlib import Path
root=Path('Packaging/Standalone')
features={
'BeyondTheField':'''# Beyond the Field by Ace

An optional suite for offline Dynasty. No other COT module is required.

## Social Media

- Follow results, polls, recruiting, transfers, awards, records, coaching, conference changes and supported draft developments
- Persistent reporters, analysts, official teams, fans, players and recruits have different voices and relevant replies
- Browse For You, National, conference, team and topic filters, plus published past editions
- National displays up to 300 cards; routine team coverage has a 15-root-post budget, with major-news protections and separate replies/recruit-stat treatment
- Rivalry reactions, casual chatter and seasonal content use supported context; unchanged preseason expectations do not repeat every spring week
- Dynasty Extensions spring exhibitions stay separate from competitive results and season statistics
- Recruit game/film posts use persistent generated records when native records are unavailable; they do not change native ratings, development traits or game results
- GPI (@GPIndex) covers qualifying mod grades, position leaders and weekly player/team performances
- Confirmed signing destinations use the game's committed-player membership rather than a stale interest leader
- Refreshes keep the same accounts and published words; updates do not rewrite old publications
- Existing public editions can be recovered; genuinely missing competitive weeks require available completed-game facts, without inventing old recruiting actions

## Recruiting and Notes

- Recruiting Fog, Roster Fog and Team Fog each offer Off, A+/A/A-, A, S, Descriptive and Blind, with independent choices
- Off restores the domain's native rating presentation; enabled scales retain the supported concealment rules
- Interest Display works independently: Default keeps native interest, Recruit Feedback uses broad relative standings and grouped school order, and Rank Only hides the bars
- Recruiting Blind hides interest, exact school ranks, bars and cutoff clues regardless of the interest preference
- Previously scouted, off-board and uncommitted recruits show a scouting-progress icon without changing their names
- Recruit Notes provide compact football observations, previous-season backgrounds for supported non-transfer prospects and optional athletic estimates
- Observations appear as scouting allows; Blind can retain Notes while concealing rating-derived measurements
- New reports use pipeline-based reliability, so strong relationships improve information without making every report perfect
- Previously obtained assessments and backgrounds stay stored; updates may clean their presentation without changing statistics or accuracy categories
- New high-school production varies with prospect quality, position, workload and efficiency; qualified ATH prospects can have coherent secondary roles
- Active transfers receive football observations rather than invented high-school background notes
- Optional Athletic Measurables replace revealed physical ratings with estimated dash, lifts, cone, shuttle and vertical results, including with Recruiting Fog off
- Measurables retain deterministic uncertainty and scouting gates; Default leaves the normal display, and Recruiting Blind conceals them
- Compact transfer labels retain year and redshirt information, such as TR RSFR

## Ratings, money and Player Analytics

- Optional rating colors retain native colors when switched off; descriptive grades are capitalized and fitted to supported rating circles
- Supported lists, live cards, full cards, depth charts, position changes, hub comparisons and Dynasty Extensions weekly reports/drill selectors follow their appropriate fog domain
- Full-card Ratings buckets retain selection feedback, formatted attributes and matching bars without briefly revealing numbers
- Optional dollar labels use readable $K/$M amounts, including supported edited NIL and recruiting displays; editing values and sorting remain numeric
- Player Analytics shows a selected team's completed-week and season GPI grades, snaps and full player cards
- Week 0 appears in Week 1; Season excludes unfinished weeks and matches the weekly grade after exactly one graded game
- GPI estimates use available tracked statistics, position-specific comparisons and bounded uncertainty; these are not official film grades
- The slow National Analytics list and nonworking Player Stats shortcut are omitted
- Default choices enable Social Media and A+/- fog; colors and dollar formatting start off; Measurables starts at Default

## Data and compatibility

- Settings, accounts, published history, Notes and verified runtime files use %LOCALAPPDATA%\\Ace\\DynastyCOT\\BeyondTheField across MMC copies
- Legacy data is copied without deleting originals; existing AppData preferences win and conflicts are archived
- Copy one BeyondTheField.dll into Plugins, or let COT install that same DLL; no separate runtime folder is arranged by the user
- Use one provider per module; COT can back up and retire recognized legacy plugins
- Main-menu and Play Now ratings remain native; unknown fingerprints or hook owners are rejected
- Targeted AIO and Dynasty Extensions paths are supported, not a promise for every overhaul/game version
- Multi-coach public-history recovery is improved, but active non-owner selection and school-private Notes ownership are not fully supported yet; the confirmed scope is the native Owner-school perspective
''',
'GamedayAtmosphere':'''# Gameday Atmosphere by Ace

Optional stadium atmosphere for supported offline Dynasty and basic offline Play Now. Other COT modules are not required.

## Attendance and crowd movement

- Dynasty turnout considers available program demand, season performance, opponent appeal, venue, weather and supported game context
- Poor seasons can leave noticeably emptier stands; special events help without always erasing weak demand
- Supported low-demand fixtures have attendance caps, with guarded opener, rivalry, major-game, senior-night or night-game exceptions as appropriate
- Fans arrive after kickoff, take temporary halftime seat breaks and leave at different rates as the game develops
- Tune arrival timing, kickoff fullness, break strength, departure strength, Fan Patience and Beat the Traffic
- Winning visiting supporters can still leave during a blowout, more slowly than the losing home crowd
- Replay clocks do not drive live crowd movement
- Saved fixture plans and departures preserve continuity; the crowd-regeneration setting applies only when explicitly chosen

## Visiting supporters

- Separate regional travel, national support and home ticket demand shape the visiting contingent
- Verified prestige gives established national programs more long-distance support; current results affect excitement without instantly erasing that following
- Great-circle city distances use weaker state fallbacks when necessary; unknown geography stays neutral, and FCS regions use supported regional estimates
- New eligible non-neutral plans start between 3% and 35% visitors; larger shares require weak home demand and a qualifying strong visitor
- The protected late-game visitor floor represents a small family/friends contingent without refilling departed seats
- A tapered increase helps low ordinary visiting shares without similarly inflating already healthy support
- A visiting-dominated main section keeps irregular edges and home pockets, plus scattered parties of 1-8 weighted toward 2-4
- Supported neutral Dynasty fixtures, including verified championship launches, use dynamic fanbase/prestige splits bounded to 30/70 in either direction while preserving EA turnout
- Visiting Fan Strength ranges from 0-200%, with 100% as the normal model; changes affect new plans, not old saved crowds

## Spring games

- Recognize verified Dynasty Extensions spring fixtures, including supported TeamBuilder identities, without treating the practice stand-in as an ordinary FCS opponent
- Use the actual school's colors throughout the ordinary crowd, retaining protected special groups
- Estimate exhibition turnout from actual-school audience/venue inputs, stable engagement and modest variation, with time/weather/roof effects and a 60% ceiling
- Preserve intentionally empty stadiums, native empty seats, culling and the corrected occupied-seat baseline
- A permanent post-halftime wave, temporary breaks and gradual second-half departures are score independent
- Spring reactions favor successful plays and restrained disappointment after meaningful starting-offense failures; audible boo acceptance remains pending

## Sound, shake and replays

- Crowd volume follows modeled supporters present; generally medium reactions are the minimum except genuinely sparse crowds
- Opening energy, play-selection buildup, pass anticipation, skill moves, big hits, sacks, takeaways, returns, stops and scores have guarded reactions
- Distinct verified events can coordinate complementary sounds with separate delivery tracking and bounded expiry; generic reactions do not automatically suppress meaningful current events
- Background roar softens during large home deficits, settles more gently during large home leads and quiets after visiting touchdowns
- Supported crowd/band/commentary routes retain their protections; no claim that every selected sound is audible in every mix
- Camera shake requests accompanying sound; tune play-selection, important-down, live-play, touchdown, celebration, camera-specific, visitor and cannon effects
- Dynamic Camera Effects must be enabled for added shake
- Separate controls cover extra ordinary-play insets, defensive highlights and full-screen non-touchdown big plays
- Full-screen replay yardage defaults to 12 and can be changed in-game; the supported Broadcast Universe handoff enables that route without promising every native clip
- Touchdown presentation remains native; no experimental guarantee of touchdown replays is included
- Show crowd fullness in the pause menu; 2020 Mode empties stands and quiets stadium layers

## Installation and limits

- Copy GamedayAtmosphere.dll into Plugins, or use COT to install the same DLL; matching support prepares automatically in AppData without an injector EXE or manual COT-Support folder
- Data/settings/history/logs use %LOCALAPPDATA%\\Ace\\DynastyCOT\\GamedayAtmosphere across MMC copies; originals are preserved during legacy migration
- Bands, recognized military/special groups, native empty seats and culling retain their protections
- With arrivals enabled, fresh offline Play Now starts at 75% of its available crowd and then fills; its visiting appearance remains native and it does not use Dynasty travel/history
- Practice and online atmosphere are not supported; exact fingerprints, scope, ownership and integrity checks remain enforced
- New attendance/shares/layout rules apply to new plans; an existing saved fixture can retain its old plan deliberately
''',
'ExtraSettings':'''# Extra Settings by Ace

Optional controls for offline Dynasty. Other COT modules are not required.

- Set offensive and defensive playcall cooldowns independently, controlling how soon a play can repeat
- Set offensive and defensive playcall limits independently, controlling the number of uses; zero disables the restriction
- Choose offensive play clock On, Off At Playcall, Off or 60 Seconds
- Choose defensive clock 3, 5, 15, 20, 30, 45 or 60 seconds, or Off
- Clock choices belong to each verified Dynasty; new Dynasties start with offense On and defense 15 seconds
- Find the controls in Dynasty League Settings > Game Settings; start a fresh game to check changed gameplay settings
- A verified persistence helper saves supported choices after MMC closes and exits with its matching game session
- Settings, diagnostics and verified runtime files use %LOCALAPPDATA%\\Ace\\DynastyCOT\\ExtraSettings across MMC copies
- Legacy data is copied without deleting originals; explicit AppData preferences take precedence and conflicts are archived
- Copy ExtraSettings.dll into Plugins, or let COT install that same DLL; runtime support prepares automatically
- Supported controller-selection/menu bridges coexist with the current companion modules without granting control of unsupported modes
- Offline scope, exact fingerprints, hook ownership and helper hash verification remain enforced
- Do not also load old clock/playcall/settings providers; COT handles recognized legacy copies with backups
- This does not add clock settings to online play or Play Now
''',
'ExpandedPractice':'''# Expanded Practice by Ace

Optional drive and scrimmage practice for supported offline Dynasty and main-menu Practice. Other COT modules are not required.

- Choose Default Practice, Drive Practice or Scrimmage in the native pause-menu Mode control
- Drive Practice carries downs, distance, field position and score between plays and returns to play selection
- Scrimmage supports possession changes and scoring/turnover flow, including returns before the next possession
- Choose supported starting positions, switch sides and return to Default Practice
- Session statistics track supported practice results without adding them to Dynasty season records
- Supported first-down markers, drive resets and opponent/defensive selection policies remain included
- The verified Dynasty Extensions pregame bridge avoids the confirmed controller-hook conflict while preserving hook ownership
- Settings/logs/diagnostics and verified runtime files use %LOCALAPPDATA%\\Ace\\DynastyCOT\\ExpandedPractice across MMC copies
- Legacy data is copied without deleting originals; existing AppData preferences win and conflicts are archived
- Copy ExpandedPractice.dll into Plugins, or let COT install that same DLL; support prepares automatically
- Session ownership prevents duplicate practice helpers; payload hashes are verified before startup
- Native fingerprints and offline eligibility remain enforced; unsupported owners are rejected
- No arbitrary online modes or replacement of normal competitive simulation is included
- Current companion modules share the supported practice/controller contracts; older modified versions may conflict
'''
}
notes={
'BeyondTheField':('1.0.23','1.0.7',[
'Choose separate grade scales for recruiting, rosters and teams, including Off and Blind',
'Use optional Athletic Measurables and interest-display choices independently of Recruiting Fog',
'Recruit Notes load more reliably and offer shorter, more varied observations, coherent ATH secondary roles and improved new prospect backgrounds',
'Previously saved Notes keep their data while technical labels are hidden; compact transfer labels retain year and redshirt status',
'Confirmed signing news now follows the actual signing school, including recruits whose interest list still names another leader',
'Fans and official accounts have more personality, clearer rivalry references and cleaner grammar; unchanged preseason stories and milestone repetition are reduced',
'Social Media reuses unchanged work and handles available historical editions more efficiently without increasing weekly post budgets',
'GPI Season matches the weekly grade when a player has only one graded game',
'Fog covers additional depth-chart, hub and supported Dynasty Extensions report/drill displays; money formatting and navigation fixes remain',
'Settings and history move safely to shared AppData, helping installs in protected folders'
]),
'GamedayAtmosphere':('2.1.35','2.1.11',[
'Visiting support better separates national following, travel and home ticket demand, with a modest lift for low ordinary visiting shares',
'Main visiting sections have a clearer visiting core, softer edges and small scattered groups',
'Poor home seasons matter more to turnout; supported neutral and championship crowds get dynamic supporter splits while keeping EA attendance',
'Dynasty Extensions spring games use the actual school, its colors and a dedicated exhibition turnout model, including supported TeamBuilder fixtures',
'Spring crowds keep native empty seats, take halftime breaks and gradually thin without reacting to the score',
'Background roar settles during big leads and quiets after visiting touchdowns; event reactions have more focused coordination',
'Replay yardage returns to settings with a 12-yard default, plus the supported Broadcast Universe big-play handoff',
'Crowd preparation reuses calculations and avoids repeated work while preserving saved crowds',
'One plugin DLL installs everything needed; settings and history use shared AppData rather than requiring a writable Plugins folder'
]),
'ExtraSettings':('0.2.11','0.2.10',[
'Settings, diagnostics and verified runtime files now use shared AppData, so a protected MMC folder does not stop normal saving',
'Different MMC copies share your existing choices; legacy files are preserved and explicit AppData preferences take priority',
'Keeps per-Dynasty clocks, separate offensive/defensive playcall controls and existing compatibility checks'
]),
'ExpandedPractice':('0.8.23','0.8.21',[
'Avoids the confirmed Dynasty Extensions pregame controller-hook conflict on the supported layout',
'Settings, diagnostics and verified runtime files now use shared AppData, including when MMC is installed in a protected folder',
'Keeps Default, Drive and Scrimmage modes, session statistics and existing native safety checks'
])}
for name,body in features.items():
 d=root/name;d.mkdir(parents=True,exist_ok=True);(d/'FEATURES.md').write_text(body,encoding='utf8')
 version,prior,bullets=notes[name]
 readme=f'''{name} {version} by Ace

Close MMC and the game. Back up your current {name}.dll, then copy this ZIP's {name}.dll into the versioned MMC folder's Plugins folder.
Replace the previous copy; do not load duplicate or legacy versions. COT can handle recognized old plugins for you.
Support files prepare automatically. Keep %LOCALAPPDATA%\\Ace\\DynastyCOT and all old data folders.
To roll back, close MMC/game and restore your backed-up DLL. Do not delete settings or history.
See FEATURES.md for features and limits. This is a private release-preparation candidate, not a published update.
'''
 (d/'README.txt').write_text(readme,encoding='utf8')
 limitations={'BeyondTheField':'Full multi-coach active-school selection and private Notes ownership remain unsupported outside the verified Owner-school perspective. Saved backgrounds are not rerolled. Installed-menu acceptance is still required.', 'GamedayAtmosphere':'Existing fixture history deliberately keeps old plans. Audible spring boos remain pending listening confirmation. Touchdown replays remain native and are not guaranteed. New crowd visuals and live performance still need acceptance.', 'ExtraSettings':'Offline Dynasty only. Check persistence after a clean restart; other game layouts still require their own verification.', 'ExpandedPractice':'Only the exact supported Dynasty Extensions/controller layout is covered. Check both practice entry routes after a clean restart.'}[name]
 text=f'# {name} {version}\n\nChanges since public {prior}\n\n'+'\n'.join('- '+s.rstrip('.') for s in bullets)+'\n\n'+limitations+'\n\nInstall or update: close MMC/game, back up the previous DLL and replace it in Plugins. Keep settings and history. Restore that DLL to roll back.\n'
 (d/'RELEASE_NOTES.md').write_text(text,encoding='utf8')
 discord=f'**{name} {version}**\nChanges since {prior}\n\n'+'\n'.join('- '+s.rstrip('.') for s in bullets)+'\n\n'+limitations+'\n\nClose MMC/game before replacing the DLL. Keep settings/history.\n'
 assert len(discord)<2000,(name,len(discord))
 (d/'DISCORD.txt').write_text(discord,encoding='utf8')
# Required redistributed notices remain with the ZIP as well as embedded support.
parts=[('Node.js and bundled dependencies',Path('Vendor/Node/LICENSE-v24.18.0.txt')),('nlohmann/json',Path('Modules/DynastyEnhancer/Vendor/json.LICENSE.txt')),('Player Analytics contributions',Path('Modules/DynastyEnhancer/Source/Analytics.NOTICES.txt')),('Ace Toolkit reference implementation',Path('Modules/DynastyEnhancer/LICENSES/Aces-CFB-Toolkit-MIT.txt'))]
(root/'BeyondTheField/LICENSES.txt').write_text('Beyond the Field redistribution notices\nNode is bundled by the standalone distribution; its license follows.\n\n'+'\n\n'.join(title+'\n'+p.read_text(encoding='utf8') for title,p in parts),encoding='utf8')
print('Prepared four code-reviewed feature lists, release notes and Discord posts')
