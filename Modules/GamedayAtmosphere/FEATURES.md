# Dynamic Gameday Atmosphere — full feature list

By Ace · **Update 4 private development — 2.1.9** · College Football 27 for Windows · MMC

This list describes the working code at the default settings. Visiting-fan appearances, academy sections and save/load appearance restoration still need live verification. Automatic celebration filtering and presentation-skip protection are not enabled yet. Most effects have their own switch. A crowd sound request does not guarantee EA will play it, and a replay request does not guarantee a clip exists.

Three numbers matter:

- **Expected peak:** how many fans are expected to attend, compared with the stadium capacity.
- **Kickoff share:** how many of those expected fans are seated at the start.
- **Current fullness:** how many seats are actually filled now, after arrivals, breaks and departures.

For example, a 60% expected peak with a 55% kickoff share means about 33% of stadium seats are filled at kickoff. Shake qualification and the crowd mix follow actual seated fans.

## Attendance: the crowd for this game

### Visiting supporters and protected sections

Visitor targets describe the people attending, not empty seats. For example, 5,000 visiting fans among 20,000 spectators is 25% away, regardless of stadium capacity. Bands are excluded from supporter counts, and existing EA visitors count toward the visiting total.

| Matchup | Visiting share of spectators |
| --- | --- |
| Distant opponent with limited traveling support | Usually 1–3% |
| Ordinary home game | Usually 3–10% |
| Strong traveling support or a nearby opponent | Often 10–20% |
| Large fanbase visiting a less-supported host | Often 20–35% |
| Exceptional takeover | 35–70% at launch |

These ranges guide a continuous formula, rather than assigning schools to fixed categories. Relative fanbase demand, native school-city distance, same-state opponents and rivalry shape the target. Neither stadium capacity nor a hardcoded big-school list sets the visitor percentage. Later departures can push the remaining visiting share above 70%.

About 70% of the assigned visiting fans sit together near a corner/end zone. The other 30% spread through the stands in small parties, usually two to four people in nearby seats on the same row. This changes where visitors sit, without adding fans or increasing their overall share. Sparse seating can leave some people without a nearby companion. When an original native occupied-seat snapshot is available during outfit generation, only those spectators enter the assignment budget. The first generation can precede that snapshot; the private build logs this case, and its exact sparse-crowd appearance ratio still needs a live check. Current supporter percentages always use the measured remaining spectators. Existing saved fixture plans retain their established targets and outfit seeds.

Each side has its own permanent departure count. The losing side leaves more readily; changing the lead does not bring departed fans back. A visiting crowd can become the majority after home fans leave. Reaction strength follows the supporters who remain. Neutral sites keep EA's appearance balance and their separate shake slider.

A verified fixture's target and clothing-selection seed are saved under the plugin's History folder. Matching uses native dynasty identity, both program rows, teams, season, week and stadium. Reloading the same fixture reuses that plan without forcing a crowd rebuild. Changing stadiums, seasons, program slots or dynasties does not reuse it. Saved departure fractions remain separate from clothing identity.

Resumed attendance also uses that fixture's own recorded game ID. The same teams, score and clock in another dynasty are insufficient. This ownership link works with visiting-fan appearances off and at neutral sites too. A new link is not saved before its resumable crowd checkpoint reaches disk. Older unlinked histories use an estimate; a later verified checkpoint establishes the link. Loading-time rank or weather refreshes do not discard a fixture-owned crowd history, while team, stadium, game type, quarter length, score and clock checks remain.

Band musicians are excluded from normal crowd thinning and supporter counts. Military-tagged spectators and student sections belonging to verified military schools stay through arrivals, halftime and departures. Army, Navy and Air Force's checked sections use the student tag with school uniforms. Those spectators still count as fans. Ordinary students and standing-only spectators are not automatically protected. Protected groups keep their native uniforms. 2020 Mode deliberately empties these groups too.

Protected spectators are reserved within one total crowd budget across the verified seating layouts. A 10,000-person target with 2,000 required cadets means 8,000 ordinary spectators, rather than adding those cadets on top. Arrivals, halftime absences and permanent departures share this accounting. Bands remain excluded from fan counts. If only required spectators remain, they form a headcount minimum; this does not add another percentage floor or fill native empty seats. 2020 Mode still empties everyone.

Each side keeps its existing departure pressure. Layout rounding is stable, so more permanent departures cannot return a fan to another section. Duplicate renderers of the same seating layout do not create another crowd budget. Save/replay history marks which accounting was used: older clips retain their old look, and an older Midgame Save finishes its current temporary-absence interval under the old rules before switching. New recorded targets use the last applied crowd budget rather than a newer target waiting for a safe seat update. Missing or conflicting layout data preserves the earlier guarded behavior.

Appearance selectors are changed only while EA generates the outfits, then restored. DGA keeps its own supporter record for departures and reactions. It does not recolor clothes every frame or leave a visitor plan in EA's source selectors.

Protected rivalry kickoff ranges and their faster Q1 arrival timing also apply when the richer Dynasty launch read is unavailable. This does not authorize a replacement peak-attendance calculation or invent missing school data.

### Attendance Profile

Settings → Gameday Atmosphere → Attendance offers two choices, in this order:

| Choice | Expected peak | Kickoff crowd |
| --- | --- | --- |
| Fuller Crowds | Adds 8 capacity percentage points to a fresh regular-season Dynasty target | Adds 7 points to the share of expected fans seated at kickoff |
| Default | The normal mod formula below | The normal arrival balance below |

**Default stays selected unless you change it.** For example, a 60% peak and 55% seated share become 68% and 62% with Fuller Crowds: about 42% of stadium seats filled at kickoff rather than 33%. Existing caps, weather deductions, school arrival groups and arrival timing still apply. A sellout is not guaranteed. The ordinary 70% kickoff ceiling and exceptional 80% ceiling remain.

Choose before launching a fresh game. Existing crowd plans and Midgame Saves keep their recorded kickoff plan; changing the setting does not refill a game in progress. Postseason peak attendance is unchanged. Play Now, RTG and launches without verified revised-attendance inputs keep their existing attendance behavior. Fan Patience is a separate setting for departures.

### Expected turnout

Fresh regular-season Dynasty games use a revised expected peak attendance, calculated during launch before seating is generated. It does not change attendance stored for simulated season games. Already loaded games, Midgame Saves, postseason, practice and unsupported inputs keep the game's existing peak rather than recalculating it. Play Now does not use the revised peak formula. Missing or invalid required inputs leave EA's result in place.

DGA first estimates a number of interested fans, then divides that number by the stadium capacity. A program attracting 25,000 people can nearly fill a 30,000-seat stadium while leaving many seats empty in a 65,000-seat stadium. Moving into a larger venue does not create more fans.

| Audience influence | Weight |
| --- | ---: |
| Team Prestige | 40% |
| Stadium Atmosphere | 30% |
| Previous three completed seasons | 20% |
| AP/media-poll rank | 10% |

Established programs start from a fixed attendance foundation: an equal blend of their reported 2025 average and five-year average. That observed audience is no longer increased or reduced by an initial comparison against similarly graded schools. Custom programs and schools without a reference still start from comparable programs, chosen by prestige and atmosphere, not stadium size. These are estimates of likely attendees, not a count of everyone who supports the school.

The foundation is saved once for that dynasty and program. Current support is compared with its reference metrics; improving prestige, atmosphere and results can draw more people, while decline can reduce interest. Relaunching a game or advancing another year does not itself award growth or repeatedly multiply the last audience. The current metrics determine the change from the same foundation each time.

Recent results weight the newest completed season 50%, then 30%, then 20%. Missing or unplayed seasons are ignored and the available weights are shared among the remaining inputs. For TeamBuilder, results from before the dynasty started are excluded. After Year 0 finishes, its actual result becomes eligible. The reference peer history is an expectation for later comparison, never a fabricated TeamBuilder record.

Foundations are stored under the plugin's own `History/audience-foundations.txt`, separate from its Midgame Save crowd timeline. Native dynasty ID and program record identity separate different dynasties. Loading an older save cannot import a future first-observed peer estimate. Bad required bindings or an unreadable foundation file preserve EA turnout and report the reason; no game save is edited. The core path passed live fresh-launch, arrival, restart and separate-dynasty checks in 2.1.3. The 2.1.4 correction and migration need their own live check.

Older format-1 foundations are backed up as `History/audience-foundations.pre-2.1.4.txt` before being upgraded to format 2. Established-school fan estimates receive their old-to-new calibration ratio once. Frozen grade/history/AP references, dynasty identities and TeamBuilder/peer estimates stay intact. Reloading format 2 does not repeat the correction. An unreadable file or failed original-file backup leaves that file intact and preserves EA turnout instead. An older plugin cannot read the new format; its original foundation file remains in the backup if a deliberate rollback is needed.

Program Tradition, Brand Exposure and Conference Prestige no longer receive separate weights in peak audience demand. The existing school-support standing is retained for the unchanged kickoff-share rules. Team Prestige already includes multiple school grades; toughest-place rank is not counted again.

| Game circumstance | Effect on the audience for this game |
| --- | --- |
| Current season performance | Existing short-term form adjustment; the old ±6-point strength is normalized to the audience, not the current stadium size |
| Opponent and rivalry appeal | Existing matchup strengths, with the combined ordinary adjustment bounded at −15/+12 on a fixed 75-point reference |
| Convincing breakout season | Temporary interest lift up to 15%, based on verified results and AP support; no stadium-percentage guarantee or permanent growth bonus |
| Home opener | 10–25% relative boost |
| Stadium-local night kickoff, 7 PM or later | 5–15% relative boost |
| Senior day / verified last regular-season home game | 10% relative boost |
| Same-state schools | 5–10% relative boost; half the extra local bonus if already rivals |
| Two service academies | 5–15% relative boost |
| Combined special-game boosts | Same multiplication and 40% combined cap |
| Known light / medium / heavy precipitation | Reduces expected spectators by 5% / 9% / 14% outdoors |
| Game-to-game variation | −3% to +3% of expected spectators |

Boosts and variation remain fixed by matchup, stadium, week and season; reloading does not reroll them. Rain, variation and matchup effects act on spectators so a larger stadium does not magnify them. Fuller Crowds remains an explicit user override adding 8 capacity points, rather than being recorded as permanent fanbase growth. The protected showcase events below still override ordinary peak demand.

Weak opponents matter less for programs whose attendance data shows consistently strong turnout. A smooth retention score uses the lower of the 2025 and five-year averages relative to the frozen reported 2025 capacity, rising from no protection at 85% to full protection at 100%. At full protection, an ordinary weak-FBS penalty falls from about 8% to 2% of audience; an FCS penalty falls from about 13.3% to 3.3%. Current form, positive opponent appeal and rivalry bonuses keep their strengths. Protection fades as the dynasty's audience falls below its reference. Moving to a different stadium cannot increase the score. TeamBuilder and unmapped programs keep their normal opponent penalties and never inherit the replaced school's protection. Neutral venues apply each audience's own protection to the other team, then average the appeal.

The audience model does not manufacture fans to meet the previous 25% regular-season peak floor. Attendance cannot exceed the venue's capacity. Existing departure floors still limit removals from a crowd that is already present; they do not refill a naturally sparse game. Roofed venues avoid the rain deduction. An uncertain dynamic forecast is not treated as confirmed rain. Live weather still affects departures.

Night detection continues to use stadium-local kickoff time. Regular neutral games average both audiences and form, apply the existing shared matchup policy and omit the home-opener benefit. Postseason retains EA turnout. Arrival shares, school arrival groups, deadlines and showcase kickoff ranges are unchanged.

### Showcase rivalries

These 24 native rivalry events always receive a **100% expected peak** on a supported fresh regular-season Dynasty launch, regardless of school grades, records, rankings, ordinary weather deductions or Attendance Profile:

| Matchup | Stadium fullness at kickoff |
| --- | --- |
| Florida–Georgia | 70–80% |
| UCLA–USC | 70–80% |
| Florida State–Miami | 70–80% |
| Notre Dame–USC | 70–80% |
| Oklahoma–Texas | 80–90% |
| Army–Navy | 90–95% |
| Alabama–Auburn | 70–80% |
| Michigan–Ohio State | 85–95% |
| Texas–Texas A&M — Lone Star Showdown | 70–80% |
| Florida–Florida State — Sunshine Showdown | 70–80% |
| Clemson–South Carolina — Palmetto Bowl | 70–80% |
| Ole Miss–Mississippi State — Egg Bowl | 70–80% |
| BYU–Utah — Holy War | 70–80% |
| Oklahoma–Oklahoma State — Bedlam | 70–80% |
| Michigan–Michigan State | 70–80% |
| Alabama–Tennessee | 70–80% |
| Oregon–Oregon State | 70–80% |
| Washington–Washington State — Apple Cup | 70–80% |
| Minnesota–Wisconsin — Paul Bunyan’s Axe | 70–80% |
| Pittsburgh–West Virginia — Backyard Brawl | 70–80% |
| Georgia–Georgia Tech — Clean, Old-Fashioned Hate | 70–80% |
| Iowa–Iowa State — Cy-Hawk | 70–80% |
| Kansas–Kansas State — Sunflower Showdown | 70–80% |
| Kentucky–Louisville — Governor’s Cup | 70–80% |

The exact starting percentage is fixed by the rivalry event, week and season. Reloading does not reroll it. Remaining fans arrive by 30% of the way through Q1, at normal hidden breaks. These rules replace the ordinary arrival-group and weather opening deductions for these events. Arrivals off starts with the full expected crowd. Normal halftime breaks and departures still apply afterward; the rule does not keep refilling empty seats.

Recognition uses **the game's rivalry-event identifier**, rather than a hard-coded pair of team IDs. A TeamBuilder replacement that retains that native rivalry event retains the attendance rule. A custom matchup with no matching event is not guessed from team slots, names or stadium. Already loaded games and Midgame Saves keep their recorded attendance; 2020 Mode still shows empty stands. Other modes and postseason keep their existing attendance paths.

### Kickoff and first-quarter arrivals

Peak turnout and kickoff crowd are separate. If a stadium expects to fill 50% of its seats and 55% of those fans start seated, kickoff fullness is about 27.5%.

| Game | Seated at kickoff, as a share of expected fans | Arrival deadline |
| --- | --- | --- |
| Ordinary regular season | 50–60% baseline, adjusted for school support | Halfway through Q1 |
| Late-arriving home schools | 45% baseline | 75% of the way through Q1 |
| Big regular-season game | Modest opening bonuses; usually no more than 70% | 30% of the way through Q1, unless the late-school rule applies |
| Bowl / non-championship CFP | 70–80% | 20% of the way through Q1 |
| Conference / national championship | 80–90% | 20% of the way through Q1 |

Late-arriving homes are **UCLA, USC, Virginia, Miami (FL), Georgia Tech, Maryland and Utah**. Neutral games use normal arrival timing. Smaller home venues receive up to 8 extra kickoff percentage points for easier access, tapering to zero in the largest venues. Same-state opponents add 2 points, night adds 5, a home opener adds 1, a qualifying big game adds up to 3, and two Top 10 teams add 1. These additions share the normal 70% ceiling.

Exceptional regular-season games can open at 75–80% of expected peak. This requires at least 95% expected capacity, top-fifth school support, a night kickoff or home opener, and either two Top 10 teams or rivals both in the Top 25. Late-arriving home schools do not use this exception. **80% is the usual regular-season kickoff cap.** The named showcase rivalries below have their own protected bands.

Only weather can take the kickoff share below 40%, with a hard 35% minimum. Outdoor light/medium/heavy precipitation takes 3/5/8 kickoff points. Postseason openings stay within their protected bands.

A big regular-season game means **two Top 25 teams, or rivals with either team Top 25 or both Top 50**. A rivalry between two struggling teams can still increase attendance without getting big-game atmosphere shortcuts. The same classification is used for arrivals, regular-season patience, halftime protection and big-game reaction context.

Timings scale with quarter length. For 12-minute quarters, ordinary arrivals finish at 6:00 remaining, big games at 8:24, late-arriving schools at 3:00 and postseason at 9:36. Seat changes wait for a suitable break; a clock threshold does not make fans appear during live action. Turning arrivals off starts with the expected crowd, not an automatic sellout.

Logs distinguish the arrival curve reaching its target from visible catch-up finishing. A completion notice waits until the paced temporary gaps have settled and the visibility update succeeded. The remaining-arrivals readout excludes permanent departures.

### Seating patterns

- Upper and more distant rows generally clear earlier. Midfield sidelines and useful viewing elevation keep fans longer; end zones tend to thin sooner.
- Small row-oriented group gaps and stragglers break up uniform thinning. The full-looking selection at 90% retained crowd and above is preserved.
- The pattern stays fixed for the game. It does not reshuffle at each update, on a camera turn or when watching a replay.
- Arrivals, departures and halftime breaks share the pattern. Replays use their recorded attendance; leaving replay restores the live crowd.
- Only seats populated in the game's generated baseline can fill. Permanent departures do not return. Freshly loaded crowd renderers inherit the correct attendance, including during postgame shots.
- Unverified seating layouts retain the simpler thinning fallback. Small sideline groups are excluded from the stadium fullness measurement.

### Permanent departures

Score, time, rankings, matchup, weather and the way the game developed determine patience. Winning home crowds generally stay longer. Fans leave in smaller steps at safe opportunities, rather than vanishing on a scoring play.

| General halftime situation | Permanent departure target, share of expected crowd |
| --- | ---: |
| Close big game | 1–3% |
| Close ordinary game | 3–5% |
| 17–24-point margin | 10–20% |
| 25–34-point margin | 20–30% |
| 35+ margin | 30–40% |
| Postseason | 0–5% |

Margins between these bands blend gradually. These are targets, not guaranteed cuts. Home wins, weather, protection, floors and available update windows can change the result. Home deficits above 16 can begin thinning before halftime. Moderate deficits stay gentler in Q3; substantial Q4 blowouts can catch up when too many fans remain for the score.

An underdog competing with a Top 25 opponent gets more patience, especially when leading a Top 10 team late. Recovering at least 14 points after trailing by 21 can pause new departures while the comeback is credible. A fading rally or too little time can end that protection. Possession, field position and remaining timeouts help when known. Neither an upset nor a comeback refills departed seats.

In a regular-season home night game, a home lead of 21+ points adds earlier traffic pressure from the second half of Q3. It builds with both the lead and elapsed time through Q4. A 33-point night home win can aim toward roughly a third of the crowd remaining, rather than protecting the winning crowd as strongly. This is a gradual target, not an immediate cut or an extra deduction every time someone scores. The existing crowd floors, quarter caps, comeback protection and safe seating windows remain in charge. Neutral and postseason games do not get this extra home-night rule.

After halftime, regular-season night games add up to 20% departure pressure. Low scoring pace can add up to 15%; a projected 72+ combined points can reduce pressure by up to 10%. These changes build through Q3, reach full strength in Q4 and are reduced to a quarter of that effect in one-score games. Postseason keeps its own rules. These are pressure adjustments, not repeated percentages removed from the remaining crowd.

### Halftime breaks and returns

About 20–30% of fans still present can temporarily leave their seats for the concourse. Each game has a fixed, slightly different timetable: departures begin in the final 20–30% of Q2; most return in the first 15–25% of Q3, with a smaller later wave finishing 25–35% into Q3. Close big games and packed close games hold most of that exodus until halftime.

In ordinary home games down 18+, some seat-breakers may not return. This only applies when more than 80% of the original crowd is still at the stadium and excludes big games, neutral venues and postseason. About 20–25% of the seat-breakers can become permanent exits as the deficit grows; floors and quarter limits still apply. It reduces later returns rather than removing another visible group.

Permanent and temporary absences are tracked separately. Only temporary seats can return. Matching Midgame Saves keep the original timetable.

### Late-game exits

- A small permanent group can leave at the Q3/Q4 break: roughly 1 point of the original crowd in a close home loss, 2 at a 14-point deficit, 5 at 21 and 7 at 28+. Winning and postseason crowds get more patience.
- Home deficits of 17+ can accelerate traffic departures in the last third of Q4. Deficits of 28+ can start gently in the last half. With 12-minute quarters, these begin at 4:00 and 6:00 remaining.
- Less time, few timeouts and opponent possession increase urgency. Home possession near a score or a recent home score buys patience. Unknown possession/timeout information uses a patient fallback.
- Severe late home losses can share up to 10 extra departure points beyond the ordinary quarter cap. This is an allowance, not an automatic cut. Neutral venues and postseason do not use this home-loss exception.
- In outdoor rain/snow, a home lead above 21 weakens winning-fan protection during the last half of Q4, reaching full severity at 35+. It shares a limited extra allowance instead of stacking a separate score penalty.
- A late costly home interception or lost fumble can send up to 10 extra points home when recovery looks unlikely. Neutral and postseason games are excluded. A return touchdown uses the larger score/turnover group, not both.
- A final-two-minute opponent score can send 5 extra points home if the home team trails by 9+ with at most one timeout. PATs cannot repeat it. Neutral and postseason games are excluded.
- Q4 has a separate 2:00 checkpoint when quarters are longer than two minutes. Ordinary departures stop in overtime.

### Floors, caps and timing

| Minimum retained share of expected crowd | Dry | Rain / snow |
| --- | ---: | ---: |
| Ordinary game | 25% | 15% |
| CFP / conference championship | 60% | 50% |
| National championship | 80% | 70% |

A winning Top 25 home team also has a 35% floor; higher postseason floors win. Permanent exits additionally respect a **15% actual-capacity floor** when the mapped baseline or validated launch peak is available. These rules stop further departures; they do not refill already empty seats, force sparse opening arrivals upward or override original empty seats. Temporary breaks also respect the applicable retained-crowd floors.

Default permanent-departure limits are 45 original-crowd points per ordinary quarter, 20 in CFP/conference championships and 10 in the national championship, plus the narrow late-blowout allowances described above. A blocked allowance is not saved up to hit all at once later.

Attendance decisions follow game time even while the field is visible. Actual seat changes wait for play selection, breaks or verified offscreen simulation. Ordinary steps start at 4 points and can grow to 16 after a longer wait; already-earned permanent exits can catch up within the applicable limits. Pausing does not accumulate departures. Normal/Fast Super Sim follow simulated score and time; Slow Sim uses the visible-field update rules.

## Soundscape: crowd sound and reactions

### The occupied-seat mix

Sound uses measured fullness across the verified seating layout and the number of occupied mapped positions. It does not use just the current camera view, a percentage of original attendance or real-world advertised capacity. Duplicate renderers are counted once; missing measurements keep native sound until ready.

The native crowd mixer handles one synchronized mix for the Crowd bus and its seven children. Chants follow the background; there is no separate chant cut. Commentary, players, referees and field sounds keep their normal direct volume. The old crowd-excitement limiter and graph-volume reductions are removed.

| Example at default mix strength | Background / chants | Collective reactions | Individual detail |
| --- | ---: | ---: | ---: |
| Smallest reference layout, 25% full | 50% of vanilla signal | About 62.5% | About 95.8% |
| Smallest reference layout, 70% full | 100% | 100% | 100% |
| Smallest reference layout, packed | 115% | 115% | 115% |
| Largest reference layout, 30% full | 100% | 100% | 100% |
| Dense medium layout, 94% full / about 27,500 occupied positions | About 150% (+3.5 dB) | About 124% | About 124% |
| Largest reference layout, packed | About 158.5% (+4 dB) | 130% | 130% |

Nonempty crowds never request less than 40% background/chant signal, 55% collective reaction signal or 95% individual-detail signal. The curve is smooth. Exactly zero attendance mutes crowd sound. These are audio-signal ratios, not measured perceived-loudness percentages. Individual-detail audibility depends on native routing and source material.

The vanilla balance point runs from about 70% fullness in the smallest layout to 30% in the largest. Above it, crowds get fuller than vanilla. Background Beds, Chants and StudentSection gain an additional smooth lift above 80% fullness and 20,000 occupied positions, reaching +3.5 dB around 92% / 27,000 occupied positions. Above 45,000 occupied positions, the background ceiling eases toward +4 dB at 61,888. Reaction and individual-detail gains retain the +30% maximum. Sparse crowds keep their native big-play peaks rather than having excitement flattened to the background level. Crowd Mix Strength scales how much of this profile is used; 0 keeps native levels for nonempty stands.

### Which crowds get extra layers?

Simple earned cheers or disappointment can work below the layered-atmosphere requirement. Fuller effects—roars, shake and larger celebrations—need the right situation and enough occupied seats.

| Situation | Small reference layout, 6,060 positions | Large reference layout, 61,888 positions |
| --- | ---: | ---: |
| Ordinary packed crowd | 98% full | 60% full |
| Big game / opening drive / close critical down / goal-line tension | 90% | 55% |
| Overtime / late winning drive | 85% | 50% |

Requirements blend between sizes and stop at those endpoints. Regular-season night games lower admission requirements by 10% relative to daytime, never below 50% measured fullness. This does not change the separate packed-house label. Enough people must actually be seated; qualification never adds fans.

### Before the snap, opening and closing scenes

- Eligible play selection gets a roar and anticipation swell, usually without an extra cheer. Home and visitor offensive possessions can get buildup.
- Special pre-snap cheers are reserved for calls inside the opponent's 10, the first home offensive call, or the first call of a late drive that can win. Neutral sites allow either offense.
- Big games can begin from 50% actual fullness in the intro and early Q1. The usual requirement returns gradually through Q1.
- A game expecting at least 90% peak capacity can get gentler first-possession energy from 55% fullness, or its lower normal gate, only for a big game, rivalry or venue with at least 13,000 stored seat positions. Smaller ordinary games keep their normal requirement. This ends with that possession and does not give the stronger hostile opening surge.
- The opening-drive status distinguishes meeting the normal stadium-size/night requirement from using the special opening discount. A night game meeting its normal requirement does not mean the special discount was granted.
- A qualifying opening kickoff can carry its cheer, roar and swell through the kick, return and whistle. At a home venue, visitors opening on offense can get a continuous hostile roar/swell through their first series of downs. Their first down or punt ends that special atmosphere; scores, takeaways, possession changes and field-goal attempts also end it. Timeouts and penalty scenes alone do not end the series. Each verified incompletion or stop for two yards or less can request the largest generic cheer once per play; first-down conversions are excluded and stronger scoring/takeaway reactions retain ownership. Neutral games keep each team's first Q1 offensive surge once.
- Sparse fresh openings can receive a modest sound-only welcome. Resuming, attaching late or simulating does not invent an opener.
- Ordinary sequences run about 40 real seconds including their fade. Snapping bridges sound into the play; shake fades instead of cutting off instantly. Ordinary layers weaken across 16–24-point score margins and stop starting at 24+.
- Consecutive useful home gains and conversions build drive excitement. Failures cool it. It halves over one eighth of a quarter of game time; scores, possession changes, halftime and simulation clear it. It strengthens an existing qualified situation rather than bypassing crowd requirements. Neutral games support either offense.
- Qualified home wins, or either winner at a neutral site, can retain closing atmosphere until the stats/highlights menu, with a two-minute safety limit. Earned winning eruptions take priority. Intro atmosphere has a five-minute safety limit.

UCLA has a 14% soft demand discount and USC a 10% discount at their regular home games, applied before the capacity clamp. These make ordinary games harder to fill without setting a maximum below capacity. Strong seasons, desirable matchups and event boosts can still sell out; the protected rivalry-event attendance policies still apply. Neutral and postseason turnout are unaffected. USC and Virginia (UVA) now use the same late-arrival schedule as UCLA, Miami, Georgia Tech, Maryland and Utah.

### Upset Watch and late drives

From Q2 onward, a credible home underdog bid against a Top 25 visitor can qualify for extra sound at a lower threshold, never below 50% actual fullness. Rank gap, score and game stage shape the effect; either underdog can qualify at a neutral site. A deficit above 14 ends the route, with stricter late-game treatment. This does not change packed-house status or ordinary pre-snap camera rules.

In the final three minutes of Q4, a home offense tied or down by at most eight can build winning-drive anticipation inside the opponent's 40. The normal late-drive fullness requirement applies. During the final two minutes, any one-score game gets sustained late-game anticipation for either offense anywhere on the field, including a team protecting a narrow lead. Simulated and clock-management plays do not create it. Crowd requirements still apply. Eligible overtime keeps tension through plays, kicks and two-point tries instead of going quiet between them.

Distinct live-skill, result and eruption cheers can overlap vanilla voices and each other. An independent sound slot preserves a skill that coincides with a long gain on the same update. One trigger is still consumed once; stale, wrong-team and own-fumble reports do not become extra sounds. Native voice limits can still prevent audible playback. Continuous roars share their existing priority-controlled channel rather than stacking duplicate loops.

### Live plays, skills and defensive results

- **Runs, scrambles and short catch-and-runs:** buildup starts at 11 total yards, targets its peak at 15, and requests a strong cheer with the roar at 15 by default. A fast native swell accompanies the buildup. The advanced gain threshold remains adjustable. Strength rises toward the distance target over up to about 0.8 seconds, so a late first sample does not jump straight to the peak. A tackle fades from the movement actually reached. A short catch needs actual running afterward. Backward cuts do not restart the reaction.
- **Long catches:** 20 yards at the catch can earn an immediate response. An immediate tackle gets a short impact; a running receiver keeps the live-play reaction. A short official-result confirmation window covers delayed completion data.
- **Pass anticipation:** either offense's qualifying airborne pass can get a fast swell. No pre-catch cheer, shake or separate volume bonus is added.
- **Kick returns:** swell and shake build from 25 return yards, reaching the full cheer and roar at 35. Punt returns keep their 30-to-40-yard buildup. Return distance starts where the ball is caught, rather than at the kicking line of scrimmage.
- **Interceptions:** fresh native proof can request an immediate strong takeaway cheer and sustained roar before the possession label catches up. Verified defensive fumble recoveries use the same return treatment. Own-team pickups do not qualify.
- **Forced fumbles:** fresh force proof can react on any down. A delayed-proof fallback requires opposing contact, carrier possession loss and new native fumble confirmation. Defensive recovery can supply one missed impact. Own-team pickups never get a takeaway shake or revive an old gain.
- **Defensive losses:** a confirmed loss of at least five yards, including a sack, can shake without waiting for a sack label. Shorter sacks retain sound and replay classification. Carried flags from prior plays, released passes, kneels and spikes cannot invent sacks.
- **Third/fourth-down stops:** verified knockout, breakup or hit-stick evidence can earn one confirmed stop impact. Contact and official-result callbacks cannot stack the same impact. A routine third-down incompletion alone does not earn shake in regulation. Overtime can use a generic confirmed stop.
- **Goal-line stands:** the dedicated stronger impact needs fourth down inside the five. The special high-leverage stop applies in Q4. Turnover on downs has its own paired cheer/impact.
- **Fourth-down conversions:** can react when the live ball reaches the first-down line, with one official-result fallback if missed. Later long gains or scores can still upgrade the reaction without repeating the same conversion impact.
- **Skills:** successful jukes/spins cheer without shake; ordinary broken tackles use sound/swell only. Confirmed trucks add a quick impact; stiff arms and hurdles add gentler accents. Trucks request the strongest tested big-hit “ooh” selector; successful hurdles, stiff arms, jukes and spins request stronger ballcarrier cheers. Fullness and occupied seats can still reduce their size. Independent shake-worthy moves can gently reinforce a running gain without restarting its timer. Hit sticks get immediate sound, but need a confirmed third/fourth-down stop or a loss of yards for truck-strength shake. Attempts alone do not count. Accents have a short spacing limit and a maximum of three per play.
- **Live showboating:** a fresh verified home-carrier running celebration builds slowly; visitor showboating gets boos. Both need layered atmosphere. Cached tags and post-whistle celebrations do not trigger it.
- **Kicks and tries:** close made field goals, opponent misses and blocked field goals get appropriate reactions. A game-saving block can use a touchdown-style profile. Overtime includes made/failed two-point tries and field goals; failed home results use disappointment. Safeties use a shorter touchdown-style response.

At home venues, home fans cheer home successes and react negatively to visitor successes. Visiting supporters get their own opposite response. Shake and sound size follow each side's remaining supporters: a small visiting group stays restrained, while a large visiting crowd can become more noticeable. Away Team Reaction Shake is now an On/Off switch and defaults to On. Off removes visitor-success shake while keeping the sounds. Both sides still need the normal atmosphere admission and verified play proof. Neutral sites retain their shared-crowd settings. Own-fumble pickups do not become takeaways. Away takeaways clear old positive home movement, and clock plays, stale data, pauses and recorded replays do not start new live reactions.

### Reaction variety and overlapping cheers

Positive gains cannot also trigger a deep-incompletion response. A measured loss cannot become a first down because an old flag remained set. Routine results that settle more than 2.5 seconds after the whistle do not start new late reactions; scoring updates remain eligible.

If the native event-specific cheer queue drops a request, DGA makes one short, same-size generic cheer or disappointment attempt. Existing game voices keep playing. Registration is logged separately from audible playback.

The palette contains **80 listening-tested native selectors across 27 usable families**: positive and negative ballcarrier actions, catches, field goals, fumbles, incompletions, interceptions, sacks, receiving touchdowns, rushing touchdowns and yardage gains; positive big hits; generic cheers, disappointment, surprise, boos, applause-style cheers and anticipation. Negative big-hit IDs 13–15 and negative catch ID 23 are excluded after failed listening checks.

Admitted eruptions always request the largest generic cheer and native roar variants. Pass-flight buildup uses the strongest native swell variant; crowd admission and the occupied-seat mixer still control its context and volume. Higher IDs within a family are generally stronger. Selection considers the play's importance, occupied seats and fullness; ordinary contexts favor medium and larger variants, while important skills, takeaways and peak gains request the strongest usable family member. Sparse crowds still cap the final size, so a small crowd does not receive every large-stadium recording. The remaining native variants in the auditioned families are used as variety for existing triggers, not as new play detectors.

DGA cheers may overlap native cheers. Matching native sounds and an already busy crowd do not deliberately suppress an intended DGA cheer. Same-event and ownership checks still prevent repeated callback dispatch. A discarded request can get one brief same-play recovery; a major play can have one delayed second accent. A new snap, pause or replay cancels stale work. EA still decides voice allocation; a sent request is not proof that a sound was heard.

One event can move from a live gain to a confirmed result to a score without restarting the same effect. A stronger score replaces the prior reaction. Ordinary successful-play responses can hold about 5–8 seconds and fade over about 7–9; continuous runs/returns use their whistle fade instead. Native scene interruptions may resume the remaining roar/swell on its original timer, without replaying the cheer.

### Major celebrations and postgame carry

Qualifying home moments, or either team's at a neutral site, can erupt for:

- The game's first touchdown within its first two plays, including an opening kickoff return, when tracked from the start.
- A last-gasp touchdown pass snapped in the last 15 seconds of Q2/Q4, with at least 30 air yards and a catch within five yards of the goal line.
- A go-ahead score in the final 30 seconds of Q4, or a verified winning score, including kicks and two-point tries.
- An overtime interception, defensive fumble recovery or winner.
- A game-sealing takeaway protecting a one-score lead in Q4's final minute, when remaining timeouts support that conclusion. More opponent timeouts require less time remaining; unknown timeouts are not guessed.

A 60+ yard touchdown in a one-score game receives the normal touchdown profile with added excitement; long distance alone does not cause an eruption. All celebrations retain their crowd and ownership requirements. There is no extra bus-volume multiplier stacked on the occupied-seat mix.

Ordinary eruptions last about 45 seconds; overtime winners/takeaways can last about 60. Winning eruptions can continue through postgame scenes until the stats/highlights menu. Other earned final-play reactions can finish their original fade into postgame too: ordinary effects stay bounded by their remaining fade and score effects by 11 seconds after final. Nothing restarts its cheer because the game ended. Pauses, interactive replay, ownership changes and final menus clean up the effects.

### 2020 Mode

Disabled by default. This option empties the stands and mutes collective crowd beds, chants, reactions, student-section sound, swells and scripted crowd audio. Both bands, drum cadence and the PA announcer are muted. Quiet individual Sweeteners remain at −3 dB as sideline flavor; they are existing fan samples, not newly identified player dialogue.

Outdoor ambience is lifted by 6 dB and the existing reverb return by 3 dB during unpaused field scenes for a more open sound. This uses the game's existing reverb, not a new room effect. Commentary, field, player and referee direct gains stay unchanged. Cannons and other stadium/music groups remain separate. If required crowd subgroups cannot be verified, a complete crowd mute is the fallback.

The normal attendance plan and history continue underneath. Switching off restores the current normal target and retires the owned audio changes. It does not rewrite the game's scheduled attendance or save. Fullness displays zero while active, and crowd-driven shake is suppressed. The mode works independently of the normal Attendance/Soundscape switches in supported stadium contexts. Its quiet individual-detail balance still needs a final live check on this release build.

### Steady background roar

A separate low-priority sustained roar can fill the gaps between play reactions once the stadium is more than 60% full with over 10,000 occupied mapped positions. It blends up with both numbers, uses a larger native size around the dense-crowd tiers below, and works between snaps and during live play. It does not fire repeated cheer samples or create shake.

This reuses the verified native sustained-crowd channel. A play reaction takes over that same voice, then the steady background can return afterward. That limits extra voices and avoids layering multiple copies of the same roar. Pauses, interactive replay, simulation, disabled extra crowd energy and 2020 Mode stop it. The mix stays under the existing crowd-bus gain limits. This new steady-roar balance still needs a live listening check.

### Fuller dense-crowd layers

Already-qualified atmosphere gets stronger roar support around 80% fullness and 20,000 occupied mapped positions. A second tier around 85% and 45,000 strengthens very large crowds further. Both blend in gradually. The first tier adds up to 20% to DGA layer intensity; the second adds another 12 percentage points, within the native intensity limit. These are layer-intensity changes. At the default 100% mix setting, the background mixer uses the separate +3.5 to +4 dB dense-crowd ceiling described above; reaction buses retain their +30% ceiling. Crowd Volume Adjustment ranges from 0–100%, default 100%, and blends this mix toward vanilla at lower settings. The qualified hostile opening series uses its separate temporary background balance until the series ends. Dense crowds can sustain a supporting swell at lower excitement levels, while ordinary plays keep restrained cheer sizes. Major moments can use the strongest tested variants once at least 24,000 positions are occupied at 80% fullness. No new play triggers or extra cheer requests are added.

## Broadcast: shake and replays

At a qualified home-stadium opener where the visitors take the first offensive snap, defensive beds/chants rise to +6 dB and swells to +3.5 dB at the default mix strength. The roar starts quickly with immediate supporting swell. This stays active through the visitors' first series, ending on their first down or punt, or another drive-ending event. It respects the mix/boost controls and does not add a positive lineup cheer for the visiting offense. The gentler opening kickoff stays separate.

Each positive defensive result during that actual hostile opening series can request the largest generic cheer once per snap: verified incompletions and stops allowing two yards or less, including losses. It promotes an existing official-result cheer when present, otherwise requests one cheer of its own. First downs/conversions and conceded scores do not qualify; takeaways, home scores and safeties retain their stronger existing reactions. Native sounds and earlier live accents keep playing. This adds no shake, supporting-roar size or volume bonus, does not recur after that opening series, and does not apply to neutral sites, simulation or resumed games.

Once touchdown or eruption shake has a verified matching crowd voice, brief native voice handoffs no longer fade that shake out and back in across live celebration/lineup camera states. The original strength and expiry stay unchanged. Muting, pause, recorded replay, stale context, a different game or a new snap still stop it. This fixes a reproduced interruption path; its live visual effect still needs checking.

### Strength follows the fans in the seats

Camera sliders default to 100%, which now uses the same calibrated screen-motion baseline for Standard, Zoom, All-22, Wide and Broadcast. Each slider independently adjusts that result from 0-200%; existing saved values stay unchanged. Actual zoom is balanced automatically, including unknown/player-lock views. The older 110/130/75/100/75 test calibration was normalized once by its original upgrade and is not repeated by this build.

Fullness and situation decide whether shake qualifies. Once admitted, occupied mapped positions determine strength on a smooth curve: about 0.80× at 6,060 occupied positions, 0.87× at 15,000, 0.99× at 30,000, 1.15× at 50,000 and 1.25× at 61,888. This is a straight-line increase. It continues smoothly below 6,060 and caps at 1.25×. Unknown layouts and zero occupancy add no crowd motion.

The same stadium strengthens as fans arrive and softens as they leave. Neutral-Site Crowd Shake (%) defaults to **75% of normal for either team**. Set it from 0 (no added crowd shake) to 100 (normal home-stadium strength). It affects both teams together and does not use the away-team switch. Qualifying overtime adds one 10% lift within the existing limits. Cannons keep their independent physical-blast setting.

| Effect | Default profile before crowd/view scaling |
| --- | --- |
| Ordinary play selection | 2.5–5% |
| Before a punt | 1.25–2.5% |
| Special lineup cheer | 4–6% |
| Close defensive third/fourth down | 7.5–10%; home offense 5.625–7.5% |
| Successful truck / confirmed big hit on a stop or loss | 7.5–9%; quick 0.1-second rise, about 2 seconds total |
| Hurdle / stiff arm | 3–4%; gentler rise, about 2 seconds total |
| Long gain / return | 10–12% |
| Immediately tackled big catch | 10.67–13.33%, default midpoint 12%; about 2.7 seconds |
| Five-yard loss / fourth-down conversion | 8–10% |
| Qualified breakup / knockout | 6–8% |
| Turnover on downs | 10–12% |
| Interception / defensive fumble return | 12–14% |
| Forced fumble / fourth-down goal-line stand / Q4 high-leverage stop | 13–15% |
| Touchdown | 20% opening impact, 16% hold; ends by about 11 seconds |
| Major celebration | 30% opening impact, 22.5% hold; camera ends by about 16 seconds |
| Safety / clinching first down | 16% impact, 12% hold; shorter touchdown timing |
| Ordinary blocked field goal | 14% impact, 10% hold; game-saving Q4/OT blocks use 20/16% |

Pre-snap shake stays on gameplay views. Third down needs ten or fewer yards to go; fourth down needs an offensive go-for-it call. Field goals can get ordinary buildup; routine PATs and kickoffs do not. A qualifying fresh big-game or protected-showcase opening kickoff uses a gentler 4.5–6% shake before occupied-seat and camera scaling. The protected native rivalry event can qualify even when neither team passes the ranked big-game check; team IDs and a high attendance forecast alone cannot select it. Fresh-intro proof, at least 50% fullness, the opening toggle and normal camera safeguards still apply. Its two-second whistle fade is unchanged. The separate hostile opening series keeps its 8–10% pre-snap range until the first down, punt or another drive-ending event. Punt buildup keeps its separate half-strength adjustment.

Shake eases in and adjusts to the actual view's lens, including modded cameras. Horizontal and vertical rotation compensate for zoom separately, so a close-up does not magnify shake uncontrollably. A small framing preference then makes tighter views feel stronger: about 5% more than 2.0.31 near the tested Zoom framing, and 32.5% less near the tested Standard framing and wider. Standard/wider strength includes the tested 10% reduction from 2.0.34; the accepted tight-view strength is retained. It blends smoothly between those points without reading camera preset names. This measures the lens, not physical camera height; a custom narrow lens gets the same treatment even if its camera is high up. Lineup, skill, scoring and eruption effects keep their relative strengths. All five sliders still default to 100. Normal optical correction is limited to 4x on extreme wide views. **Wide Player Camera Compensation** extends that allowance to 6x for unusually wide RTG/player-lock views; leave it Off unless those views still feel weak. It does not add another multiplier to ordinary views. The game's angle, position and zoom stay unchanged.

Standard, Zoom, All-22, Wide and Broadcast each keep their own 0-200% preference slider. At 100%, each uses the automatic lens balance; 50% halves that camera's added movement, 200% doubles it, and 0 turns it off. Existing saved percentages are kept. The selected offense or defense preference follows the controlled team and possession, including post-play reactions. Player-lock and unknown or ambiguous camera choices use automatic zoom balancing without borrowing another preset's slider. Crowd eligibility and the game's Dynamic Camera Effects controls still apply. Recorded replays remain steady. A brief invalid projection can reuse the same output view's verified reading for up to 250 ms; otherwise that output receives no added movement until a valid reading returns. Camera cuts retain the reaction's original timing, and each view receives its own correction. Extreme safety limits and differences in framing mean live visual calibration is still necessary.

At the snap, lineup shake fades over 0.9 seconds while following the new live camera. Run/catch effects build into touchdown/eruption impact over about 300 ms without restarting the motion. The strongest active crowd effect leads, with 15% of the other independent effects added, capped at 15% above the strongest. A truck during a long run can therefore be felt without doubling the shake. Reports of the same event are not extra effects, and overlap does not extend the stronger timer. Scoring celebrations replace the earlier live effects.

Runs, catch-and-runs and qualifying returns use a shared five-second whistle fade by default, adjustable from one to eight seconds. Play distance adds no extra time. Short impacts have their own roughly 2–3-second envelopes; a confirmed force has a sharper 35 ms onset. Camera cuts, timeouts and penalty presentation let earned shake finish on its original timer. Once its matching sound has been confirmed, a brief native presentation sound stop does not cut that earned shake off. This does not start an unearned effect or replay its cheer. Recorded replays stay steady. Pause, interactive replay, new games and the game's Dynamic Camera Effects switch stop added shake.

Native Stadium Pulse camera motion follows DGA's crowd/context admission, with an additional advanced minimum-fullness safeguard. While a fresh DGA crowd-shake output is actually being rendered, native HFA camera motion yields instead of adding another effect. Its latest requested tier is remembered and restored when DGA stops. Mere eligibility or a prepared request does not take ownership. This changes only its camera channel: composure, the HFA meter, play art and gameplay advantages stay native. Setting the advanced filter minimum to zero preserves its opt-out. A cannon blast does not depend on the crowd gate.

### Stadium cannons

All nine native cannon variants use a separate adjustable impact, default 18%, range 0–30%, lasting 0.9 seconds. It works at any crowd fullness, respects camera settings and uses the stronger of cannon/crowd movement rather than adding them. Setting it to zero removes only the added cannon shake. A detected blast gets one numbered log entry with its variant and requested strength; this is not proof of visible output.

### Extra replays

The mod asks the game's own recording and presentation system for more opportunities:

- Big gains can get full-screen replays; the default yardage cutoff is 15.
- Routine runs and completions can get a small replay during play selection.
- Sacks and tackles for loss can get defensive highlights. Takeaways and touchdowns use major-play routing.
- In manual play, confirmed major routes wait for full-screen playback to finish before requesting a separate inset repeat. If the game passes straight to play calling without starting that full-screen sequence, DGA returns a native inset request it held instead of waiting until the next snap loses it. It cannot create a missing recording. The normal skip controls remain available.
- Slow Sim keeps native full-screen presentation and does not ask for an inset, because it has no play-call screen. Mode switches cancel pending inset work.
- Kneels and spikes do not receive routine replay promotion. A running native replay is not interrupted by an inset timeout fallback.

The game still controls recording availability and scene selection. Injuries, penalties, safeties, skips and special presentation can change or interrupt a sequence. Neither full-screen nor inset replay is guaranteed after every qualifying play. The reported base-game repeated kneel-down postgame montage is not fixed.

### Recorded crowds, saves and simulation

Instant Replay, automatic clips and the postgame highlights viewer use attendance recorded near the chosen play. Earlier highlights can show a fuller crowd. Automatic clips also use that recorded crowd for the volume mix; they do not invent old cheers or run new departure decisions. Exiting restores the live crowd and only seats controlled by the mod can return.

Midgame Saves restore recent matching crowd history using the teams, score, time and quarter length. Loading-only records cannot replace actual played history. Pausing records the visible crowd; arrivals hold that level until game time advances. Matching Q1 saves retain peak, kickoff share, arrival deadline and local night status. Otherwise, the mod estimates from the resumed game and says so in the logs. It cannot recreate missing earlier play history exactly.

History keeps the last eight games in the DGA folder. New games and real scenario loads reset ownership correctly; a delayed save notice after Super Sim does not start a second crowd. Normal/Fast Super Sim update attendance from their score/clock; Slow Sim keeps field-visible timing. Returning to manual play clears obsolete presentation work.

## Settings, files and support

The **Gameday Atmosphere** tab is available from User Settings and **Pause > Options > Settings**. In pause-menu settings it sits at the far right, after Auto-Subs. Both routes use the same saved settings. It has 45 controls: **Attendance 13, Soundscape 12, Broadcast 20**. On/Off switches come first; numeric controls follow. Each has plain-language help. The pause menu can show Stadium Fullness beside Resume; a dash means the measurement is not ready.

The private 2.1.6 candidate fixes DGA tab routing when AIO Broadcast Beta1 replaces the shared settings menu. DGA keeps its own controls and leaves the broadcast switcher alone. The supplied files pass the offline compatibility check; both menus still need a live check together.

The full option list follows below. Advanced settings remain in `Plugins/DynamicGamedayAtmosphere/DynamicGamedayAtmosphere.ini`, with defaults, accepted ranges and explanations. They cover arrival/traffic limits, comeback protection, halftime timing, atmosphere admission, reaction thresholds/durations, replay routing, camera profiles and logging. Retired excitement-limiter and independent chant-cut values do not affect the new mix. Gameplay HFA is not an adjustable DGA feature.

**Update 3 resets previous DGA settings once, including test-build settings.** The exact old INI is backed up before replacement. No manual deletion is needed. Later edits are kept. Game saves and native game settings are unaffected. Close both the game and MMC before replacing the DLL.

Important combinations:

- Attendance off leaves native attendance and disables attendance-dependent soundscape; extra replays stay separate. 2020 Mode has its own override.
- Crowd Sound Effects off at launch skips DGA audio hooks. Restart the game/MMC after changing this master switch.
- Extra Crowd Energy off disables the added layer system; the independently configured occupied-seat volume mix can remain.
- Fans Leaving Early at zero disables formula pressure, not independently configured late-score exits or temporary halftime breaks.
- Added Camera Shake off stops DGA motion. The game's Dynamic Camera Effects must also be on for shake to appear.
- The advanced MaximumBoostPercent zero also disables added layers/motion; its legacy higher values do not stack a second gain on the native-bus mix.

### Logging and performance

Install by copying only `DynamicGamedayAtmosphere.dll` into Plugins. The plugin handles runtime loading itself; there is no separate loader EXE or runtime folder to copy. Settings, Logs, Backups, History and tools remain under `Plugins/DynamicGamedayAtmosphere/`. Generated native runtime DLLs live under `MMC/DGA-Runtime/Runtime-version`, outside MMC's recursive plugin scan. Exact previous DGA runtime/helper files are archived there once the game is closed. Existing settings, attendance history and audience foundations are preserved. Other plugin files are left alone. An old runtime warning can appear during the first upgrade scan before cleanup; reopen MMC afterward.

| File in Logs | Contents |
| --- | --- |
| `status.txt` | Readable activation, matchup, crowd, sound, shake and replay summary |
| `DynamicGamedayAtmosphere.log` | Readable events and warnings |
| `technical-status.txt` | Full settings, bindings, proofs and timing counters |
| `technical.log` | Detailed native events, IDs, result proof, audio receipts and replay stages |
| `launcher.log` | MMC startup/loading errors and recovery |

Technical details remain available under normal logging; Debug adds extra traces. Event logs have separate size limits, about 2 MB each by default. Status files are replaced rather than appended. Disk writes and readable formatting run on the background worker. Timings report callback wall time, not FPS or whole-plugin CPU usage.

Repeated replay sampling and expression reads at the whistle were reduced after live OFF/ON testing identified the hitch. Disabled replay additions forward directly to native behavior. Repeated crowd part lookups are reused within a single crowd pass, seat distances are prepared before sorting, and unchanged targets avoid redundant work. Actual seating changes can still take time; there is no universal zero-hitch or FPS guarantee.

The plugin checks native layouts before activating. Launcher failures identify the failed operation and available Windows error details; repeated unchanged failures back off instead of retrying every second. Runtime loading is bounded to the game process/start identity. Logs distinguish a sound request from native voice registration; neither alone proves audible output or rendered shake. No logs upload automatically.

The included local diagnostics tool offers bounded sound/camera previews and a read-only compatibility report. Previews help check output but do not prove automatic play detection, change attendance or alter gameplay HFA. Reports remain local until you choose to share them.

### Compatibility and limits

Live testing includes offline Dynasty and Road to Glory, including the default RTG player-camera capture and shake adjustments. The revised peak-attendance formula remains Dynasty-only. The Windows title update inspected October 30, 2026 is supported. Testing does not establish every rare play or presentation route in both modes; other modes and future title updates need their own compatibility checks. The plugin does not patch the game EXE on disk or ship game files.

Custom stadium songs, new crowd animation behavior, spectacular-catch detection and a replacement postgame highlight picker are not included. Seat desirability is inferred from geometry, not ticket prices. Quiet individual-detail gain is limited by native routing. Replay requests cannot guarantee the game's final presentation.

For a report, send the whole Logs folder and note the teams, quarter/clock, play result and what sounded or looked wrong. Do not send only a request line as evidence that the effect played.

## In-game option reference

45 controls: 13 Attendance, 12 Soundscape and 20 Broadcast

### Attendance options

| Option | What it does |
| --- | --- |
| Dynamic Attendance | Let fans arrive after kickoff, take halftime breaks and leave early. Turn this off to keep the game's normal crowd. Extra replays have their own switch. |
| 2020 Mode | Play with empty stands. Mute the crowd, bands, drums and PA announcer, but keep quiet individual voices and a more open stadium sound. Turn off to bring back normal attendance and sound. |
| Fans Arriving After Kickoff | Start with some fans still outside, then fill their seats during the first quarter. Turn off to start with the full expected crowd. This does not guarantee a sellout. |
| Beat the Traffic | Let fans leave sooner when the home team is far behind late in the game. With 12-minute quarters, this starts at 4:00 in Q4 when down 17+, or gently at 6:00 when down 28+. |
| Visiting Fans | Give visiting fans their own sections and scattered seats. Their share depends on fanbase size, travel and rivalry. Choose before loading a fresh game; changing this does not recolor a crowd already loaded. Bands keep their uniforms. |
| Attendance Profile | Default keeps the normal mod balance. Fuller Crowds raises the expected peak by 8 percentage points in fresh regular-season Dynasty games and seats 7 more of every 100 expected fans at kickoff in those games. Existing caps and weather rules still apply; named showcase rivalries have their own starting ranges. Choose before loading a fresh game. Postseason crowds keep their own rules. |
| Fan Patience | Automatic follows the matchup and weather. Normal uses standard departures. Patient Fans makes people stay longer. Bad Weather makes people leave sooner. This does not change the starting crowd. |
| Ordinary Kickoff Crowd (%) | How much of the expected crowd starts in its seats. At 55, about 55 of every 100 expected fans are seated at kickoff. Smaller stadiums and special games can start fuller. Higher means more fans at kickoff. Applies to fresh games. |
| Late-Arriving Schools: Kickoff (%) | Kickoff crowd for UCLA, USC, Virginia, Miami (FL), Georgia Tech, Maryland and Utah. At 45, about 45 of every 100 expected fans start seated. Higher means more fans at kickoff. Night games and weather can change it. Applies to fresh games. |
| Big Game Kickoff Bonus (points) | Add up to 3 points to the seated share at kickoff. Big games mean two Top 25 teams, or rivals with either team Top 25 or both Top 50. Most regular games stay at 70% of expected fans or less; exceptional games can reach 80%. |
| When Arrivals Finish (% of Q1) | How far into Q1 ordinary schools finish filling up. At 50, fans finish halfway through the quarter. Higher means later arrivals. Big games, late-arriving schools and postseason games keep their own timing. Applies to fresh games. |
| Fans Leaving Early (%) | How strongly the score, time and matchup make fans leave. 100 is normal, 50 is gentler and 200 is stronger. At 0, these departures stop. Halftime breaks and special late-game exits have their own settings. |
| Halftime Seat Breaks (%) | How many fans temporarily leave their seats around halftime. 100 is normal; higher leaves more empty seats and lower leaves fewer. At 0, there are no temporary breaks. Fans who stay at the stadium return early in Q3. |

### Soundscape options

| Option | What it does |
| --- | --- |
| Crowd Sound Effects | Adjust crowd sound to the number of fans present and add reactions to plays. Turn off to keep the game's normal sound. Restart the game and MMC after changing this switch. 2020 Mode controls its own audio. |
| Extra Crowd Energy | Add roars, buildup and stronger reactions when the game and crowd qualify. Sparse crowds can still get simple cheers or disappointment. Bigger effects and shake need enough seats filled. Turn off to stop these extra layers. |
| Opening Atmosphere | Add intro and early-game energy. Qualifying big games and protected rivalries can get opening kickoff shake. If visitors open on offense, keep their first series loud until a first down or punt. Crowd fullness requirements still apply. |
| Play Selection Atmosphere | Build crowd noise while you choose a play and react to successful plays. Close games get extra energy in the last two minutes. Special cheers before the snap favor the home offense; at neutral stadiums either team can get them. |
| Overtime Atmosphere | Keep a qualifying overtime crowd lively between plays, kicks and two-point tries. Important results get a reaction, and crowd shake gets a small 10% lift. The crowd still needs enough seats filled. |
| Upset Watch Sounds | Build excitement from Q2 onward when a home underdog threatens a Top 25 visitor. At least half the seats must be filled. At neutral sites, either underdog can qualify. |
| Live Play Reactions | Let the crowd react during catches, runs, sacks, takeaways and returns. Bigger plays and larger crowds can use stronger sounds. Sparse crowds can still get sounds, but shake needs the normal crowd requirements. |
| Skill Move and Big Hit Reactions | React to successful moves and big hits. Trucks add a quick shake; hurdles and stiff arms add a gentler one. Spins, jukes and ordinary broken tackles add sound only. Big hits add truck-strength shake after a confirmed third- or fourth-down stop or any confirmed loss of yards. |
| Buildup During Passes | Let the crowd build anticipation while a qualifying pass is in the air, then settle after the catch or incompletion. Does not add shake before the catch. Live Play Reactions must be on. |
| Cheers and Disappointment | Home and visiting fans react to good plays and mistakes. More supporters means a bigger response. Turn Away Team Reaction Shake off to keep visitor reactions sound-only. Neutral stadiums keep their usual shared-crowd balance. |
| Major Celebrations | Use the biggest celebration for special moments, such as a very early touchdown, a late winning score, an overtime winner or a game-sealing takeaway. A long touchdown alone gets the normal touchdown celebration. |
| Crowd Volume Adjustment (%) | How much the mod changes crowd volume for the fans present. 100 uses the normal mod mix. Lower stays closer to the game's normal volume; 0 keeps normal volume unless the stands are empty. Does not reduce the game's own excitement. |

### Broadcast options

| Option | What it does |
| --- | --- |
| Show Fullness in Pause Menu | Show the percentage of seats filled beside Resume in the pause menu. It updates when you open the menu. A dash means the crowd measurement is not ready yet. |
| Added Camera Shake | Allow camera shake added by this mod. Turn off to stop all of the mod's added shake. The game's Dynamic Camera Effects setting must also be on. |
| Wide Player Camera Compensation | Camera zoom is balanced automatically. Turn On only if extremely wide RTG or player-lock views still feel weak: it allows extra correction on those views. Normal cameras usually do not need this. Added Camera Shake must be on. |
| Additional Replays | Ask the game for more replays after plays. Turn off to stop these extra requests. The game can still show its own replays. |
| Full-Screen Big Play Replays | Ask for full-screen replays after big plays. The game still chooses what it can show, and you can still skip replays normally. Additional Replays must be on. |
| Small Replays During Play Selection | Allow small replay windows while you choose a play after ordinary runs and completions. Slow Sim has no play-selection screen, so it gets no added small window. Additional Replays must be on. |
| Defensive Play Replays | Allow extra replays after sacks and tackles for loss, along with interception and lost-fumble highlights. Additional Replays must be on. |
| Away Team Reaction Shake | Let visiting fans cause shake after their big plays and stops. More visiting fans means a stronger response. Off removes this shake but keeps crowd sounds. Normal atmosphere rules still apply. Neutral stadiums keep their separate control. |
| Neutral-Site Crowd Shake (%) | How strong the mod's crowd shake is for BOTH teams at a neutral stadium. 75 keeps the usual balance, 50 is half normal home-stadium strength and 100 is the same strength. 0 turns this crowd shake off. The crowd must qualify. Stadium cannon shake has its own setting. |
| Normal Play Selection Shake (%) | Maximum shake during ordinary play selection. Higher is stronger; lower is gentler. At 0, this type is off. Buildup before a punt uses half this strength. A qualifying crowd is still needed. |
| Important Down Shake (%) | Maximum pre-snap shake on qualifying third and fourth downs in close games. Higher is stronger; 0 turns this type off. Home-offense buildup is 25% gentler than when the home crowd is defending. A qualifying crowd is still needed. |
| Live Play Shake (%) | Strength of shake for gains, takeaways and important stops. Higher is stronger; 0 turns it off. Big events get more shake than routine ones. Kickoff returns build from25 to35 yards; punt returns from30 to40. Distinct crowd effects can add a small extra shake without doubling it. Picking up your own fumble adds no shake. |
| Touchdown Shake (%) | Shake level held after the first touchdown impact. Higher is stronger; lower is gentler. At 0, touchdown shake is off. An active run or catch reaction builds smoothly into the celebration. |
| Major Celebration Shake (%) | Shake level held after the first impact of a major celebration, such as a winning play. Higher is stronger; 0 turns this type off. An active play reaction builds smoothly into it. |
| Stadium Cannon Shake (%) | Shake when a stadium cannon fires. Higher is stronger; 0 turns off its shake but keeps the cannon sound. Works even in sparse crowds. Added Camera Shake and the game's Dynamic Camera Effects must be on. |
| Standard Camera Shake (%) | How much added shake you get with the Standard camera. 100 is the recommended balance, 50 is half as strong and 200 is twice as strong. 0 turns added shake off for this camera. The crowd still needs to qualify for shake. |
| Zoom Camera Shake (%) | How much added shake you get with the Zoom camera. 100 is the recommended balance, 50 is half as strong and 200 is twice as strong. 0 turns added shake off for this camera. The crowd still needs to qualify for shake. |
| All-22 Camera Shake (%) | How much added shake you get with the All-22 camera. 100 is the recommended balance, 50 is half as strong and 200 is twice as strong. 0 turns added shake off for this camera. The crowd still needs to qualify for shake. |
| Wide Camera Shake (%) | How much added shake you get with the Wide camera. 100 is the recommended balance, 50 is half as strong and 200 is twice as strong. 0 turns added shake off for this camera. The crowd still needs to qualify for shake. |
| Broadcast Camera Shake (%) | How much added shake you get with the Broadcast camera. 100 is the recommended balance, 50 is half as strong and 200 is twice as strong. 0 turns added shake off for this camera. The crowd still needs to qualify for shake. |

The advanced full-screen replay yardage cutoff remains available as `FullScreenMinimumYards` in the INI. It is hidden from the menu to keep Broadcast at 20 controls. The working build uses the new Away Team Reaction Shake switch. Older percentage values do not multiply the new supporter-based strength.

Update 4 development: routine reaction sounds can use the mildest tested variant; important downs use medium variants and major plays retain strong variants. Occupied seats and fullness still limit the final variant. Native reactions are unchanged, so a native cheer can still be stronger than the DGA request.
