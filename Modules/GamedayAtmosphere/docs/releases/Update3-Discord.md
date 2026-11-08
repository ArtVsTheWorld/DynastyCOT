**DYNAMIC GAMEDAY ATMOSPHERE — UPDATE 3**
**1/7 — A rebuilt attendance system**

- Fresh regular-season Dynasty games now get a DGA peak-attendance calculation instead of simply keeping EA's number
- The overall spread is calibrated against real 2025 FBS attendance-to-capacity data, giving struggling programs sparse crowds while strong programs can regularly reach capacity
- School support uses Stadium Atmosphere 35%, Team Prestige 25%, Program Tradition 20%, Brand Exposure 10%, Conference Prestige 5% and AP rank 5% — using the ratings in your current Dynasty, so programs can grow or decline
- Season results, opponent quality and rivalry interest affect demand, with extra help for convincing breakout seasons so a smaller school can earn a big crowd
- Home openers get a variable 10–25% demand boost, night games 5–15%, senior day 10%, same-state matchups 5–10% and service-academy matchups 5–15%
- These are relative demand boosts, not percentage points or automatic sellouts, and their combined increase is capped at 40% before the ordinary weather and capacity limits
- A new Attendance Profile lets you choose Default or Fuller Crowds, which adds 8 peak-capacity points and 7 points to the kickoff share within the applicable limits
- Choose the profile before loading a fresh game — it does not refill an existing game or rewrite a Midgame Save

---

**UPDATE 3 — 2/7**
**A completely rebuilt crowd mix**

- Crowd sound now uses the native audio mixer and follows both actual fullness and occupied seat count, so a small full stadium and a huge full stadium no longer sound the same
- Continuous background noise, collective cheers/boos and individual fan detail have separate balances — sparse crowds lose more of the wall of noise while keeping scattered voices and reactions
- The vanilla-volume crossover ranges from roughly 70% fullness in the smallest venue to 30% in the largest, with fuller crowds growing beyond vanilla
- Dense backgrounds can reach +3.5 dB, rising toward +4 dB in the largest packed crowds, while collective reaction gain stays capped at +30% signal
- At 25% fullness in the smallest venue, background sound is 50% of vanilla; nonempty floors are 40% background/chants, 55% collective reactions and 95% individual detail
- Chants follow the background mix, with no separate chant cut; the old excitement limiter is gone, so native big-play excitement is preserved
- Native reaction support expands from 5 choices to 80 listening-tested choices across 27 usable families, including catches, hits, takeaways, touchdowns, kicks, cheers, boos, disappointment and anticipation
- Stronger variants suit bigger moments and occupied crowds, and DGA cheers can overlap vanilla cheers without repeated callbacks firing the same event again
- A sustained background roar can fill the gaps above 60% fullness and 10,000 occupied seats, sharing its channel with stronger play reactions rather than repeatedly firing cheer samples
- Commentary and field sounds keep their normal direct volume; gain percentages describe signal level, not literal perceived loudness

---

**UPDATE 3 — 3/7**
**New in-game controls, fullness display and 2020 Mode**

- New Gameday Atmosphere tab in the game's settings, with 38 plainly explained controls: Attendance 12, Soundscape 12 and Broadcast 14
- Change arrival/departure balance, attendance profile, crowd effects, shake and replay options in game, with the advanced INI still available
- Stadium Fullness now appears beside Resume in the pause menu and updates when you open it
- Fullness means the share of seats actually occupied right now, not the share of the expected crowd — a 60% peak with 55% of those fans seated means about 33% stadium fullness at kickoff
- Sound and shake follow that measured crowd, and a dash means the seating measurement is not ready yet
- New optional 2020 Mode empties the stands and mutes collective crowd noise, bands, drum cadence and the PA announcer
- 2020 Mode keeps quiet individual detail and lifts the existing outdoor ambience/reverb for a more open stadium sound, then restores the normal crowd and mix when switched off
- Wide Player Camera Compensation has its own Broadcast switch, off by default — leave it off for normal cameras and use it only when you want the extra allowance for very wide player/RTG views

---

**UPDATE 3 — 4/7**
**Fuller special games, separate kickoff crowds**

- Peak attendance and kickoff attendance are separate: ordinary games usually seat 50–60% of their expected crowd initially, with context bonuses and gradual arrivals afterward
- Smaller home stadiums get up to 8 extra kickoff points for easier access; the late-arriving group remains UCLA, Miami, Georgia Tech and Maryland, with Utah now included
- Ordinary marquee openings usually cap at 70% of expected fans, exceptional regular-season openings can reach 80%, bowls/CFP start at 70–80% and championships at 80–90%
- Eight major rivalry events now get a full expected peak regardless of ordinary program demand, with protected kickoff bands
- Florida–Georgia, UCLA–USC, Florida State–Miami, Notre Dame–USC and Alabama–Auburn start 70–80% full
- Oklahoma–Texas starts 80–90% full, Army–Navy 90–95%, and Michigan–Ohio State 85–95%
- Those special rules follow the native rivalry event rather than team-ID pairs, so TeamBuilder replacements retain them when they retain that event
- Big-game rules now agree across the mod: two Top 25 teams, or rivals with either team Top 25 or both Top 50
- Night detection uses the verified stadium-local kickoff time, rather than assuming a late Eastern listing makes every West Coast game a night game
- These attendance plans are set during a supported fresh Dynasty launch, not week advance; postseason keeps its existing peak-attendance path

---

**UPDATE 3 — 5/7**
**More natural filling, thinning and late-game crowds**

- Refined seating desirability keeps valuable midfield sideline areas fuller, with smaller group gaps and less uniform thinning across sections
- Arrivals and seat changes wait for suitable breaks rather than visibly popping fans in or out during live action
- Halftime breaks have more varied timing and two return waves, while close big games keep more fans seated until the half
- Temporary seat breaks and permanent departures stay separate, so people who have actually gone home do not return with the halftime crowd
- Close games, credible upsets and comebacks get more patience; decided games get stronger traffic pressure based on the score, clock, possession and remaining timeouts
- Home night blowouts of 21+ points begin building extra departure pressure in late Q3, then increase through Q4 instead of protecting the winning crowd so strongly
- Lower-scoring second halves create more departure pressure than shootouts, with a gentler effect when the game is close
- Rain/snow can accelerate late exits, while postseason, neutral-site and comeback protections retain their own rules
- A 15% actual-capacity floor stops further permanent departures when a valid crowd baseline is known — it does not force sparse kickoff crowds upward or refill empty seats
- Better Midgame Save history selection preserves Q1 arrivals and the crowd you actually had, while recorded replays show their historical crowd without changing live attendance

---

**UPDATE 3 — 6/7**
**More responsive plays and rebalanced shake**

- Successful trucks get the big-hit “ooh”; hurdles, stiff arms, jukes and spins get stronger cheers, with jukes/spins and ordinary broken tackles remaining sound-only
- Verified home showboating builds atmosphere, while visitor showboating gets boos when layered atmosphere qualifies
- Runs, scrambles and catch-and-runs build from 11 to 15 yards, with a swell into the peak cheer/roar; kickoff returns build from 25 to 35 yards and punt returns from 30 to 40
- Interceptions and defensive fumble recoveries get prompt takeaway cheers and sustained return reactions; picking up your own fumble does not earn a takeaway effect
- Confirmed losses of 5+ yards can shake without waiting for a sack label; fourth-down conversions, breakup/knockout stops and turnover-on-downs handling are more reliable
- Dedicated goal-line stand shake needs fourth down, and the special high-leverage stop is limited to Q4
- Once the crowd qualifies, shake scales linearly from 0.80x at 6,060 occupied positions to 1.25x at 61,888, so the same stadium strengthens as fans arrive; neutral-site crowd shake uses 75% strength for either team
- Distinct effects can gently add together, capped at 15% above the strongest, without extending its timer or double-counting the same event
- Zoomed views retain more shake: an old 50% correction now gives about 64%, while ordinary wide views stay capped at 1x; native HFA camera motion yields during DGA shake without changing gameplay HFA
- Shake blends into touchdowns and finishes through timeout/penalty presentation on its original clock; native cannons have their own adjustable 18% impact

---

**UPDATE 3 — 7/7**
**Overtime, replays, fixes and compatibility**

- Overtime gets steadier atmosphere, a small shake lift and better coverage for stops, made/missed field goals and two-point tries
- Earned final-play reactions can finish into postgame instead of cutting off, with eruptions keeping their longer winning celebration
- Major-play replay handling separates the full-screen sequence from the later play-selection inset; Slow Sim gets no added inset because it has no play-call screen
- Better fallback handling when full-screen selection does not start, while injuries, skipped scenes and missing native recordings can still affect what EA shows
- Improved catch/run-to-score handoffs, defensive fumble ownership, stale sack flags, delayed results and dropped cheer requests
- Less repeated replay/crowd work, cached unchanged data and added timing counters to address post-play hitches
- Readable status/log files now sit alongside detailed technical logs, keeping troubleshooting information without making the everyday log a wall of internals
- Offline Dynasty and Road to Glory have been tested, including the wider RTG player camera; the revised peak-attendance formula remains Dynasty-only
- Updated for the Windows title update checked October 30, with 138 native test programs, 936 full-game policy simulations and an isolated MMC host check passing locally
- Close the game and MMC before replacing the DLL; old settings reset once with an automatic backup, so you do not need to delete your INI
- Only the DLL belongs directly in Plugins — settings, logs, backups and extracted tools stay together in Plugins/DynamicGamedayAtmosphere

**Download:** https://github.com/ArtVsTheWorld/DynamicGamedayAtmosphere/releases/tag/v2.0.21
