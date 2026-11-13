# Gameday Atmosphere by Ace

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
- Data/settings/history/logs use %LOCALAPPDATA%\Ace\DynastyCOT\GamedayAtmosphere across MMC copies; originals are preserved during legacy migration
- Bands, recognized military/special groups, native empty seats and culling retain their protections
- With arrivals enabled, fresh offline Play Now starts at 75% of its available crowd and then fills; its visiting appearance remains native and it does not use Dynasty travel/history
- Practice and online atmosphere are not supported; exact fingerprints, scope, ownership and integrity checks remain enforced
- New attendance/shares/layout rules apply to new plans; an existing saved fixture can retain its old plan deliberately
