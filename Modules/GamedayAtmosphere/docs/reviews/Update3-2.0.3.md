# Update 3 candidate 2.0.3: shake rebalance

This candidate changes presentation strength and transitions. Existing atmosphere/fullness gates, play ownership, loss/stop proof, native HFA policy, accessibility and replay cleanup remain in force. It does not make a sparse unqualified game eligible for added motion.

## Occupied-seat response

The curve now uses a smooth logarithmic response rather than the old low linear floor. Values refer to occupied mapped crowd positions, not real-life venue capacity. It continues below 6,060 and caps at 1.10x; zero occupancy or unknown geometry produces no added crowd motion.

| Occupied positions | Former multiplier | New multiplier |
| ---: | ---: | ---: |
| 3,030 | 0.350x | 0.750x |
| 6,060 | 0.350x | 0.800x |
| 15,000 | 0.470x | 0.896x |
| 30,000 | 0.672x | 0.990x |
| 50,000 | 0.940x | 1.067x |
| 61,888 | 1.100x | 1.100x |

The selected target is multiplied once by this factor, then by 0.75 for either team at a neutral site. Existing camera/zoom compensation follows. A 20% touchdown peak becomes 16% at 6,060 occupied positions and 22% at 61,888 before zoom compensation. Native cannon impacts retain their independent 15% physical-blast setting.

## Event hierarchy

These are stock base strengths before crowd, neutral and camera factors. Context chooses a point inside each range; it no longer compounds a 0.5 defensive/skill multiplier, 1.2 goal-line multiplier or up-to-1.3 context multiplier. Custom live/catch settings scale the corresponding ranges proportionally.

| Existing eligible event | Base strength |
| --- | ---: |
| Ordinary preplay | 2-4% |
| Pre-punt | 1-2% |
| Special lineup cheer / home showboat buildup | 3-5%, gradual |
| Home-offense critical down | 4.5-6% |
| Defense/neutral critical down | 6-8% |
| Qualified opening kickoff / hostile first offensive play | 8-10% |
| Truck, stiff arm, hurdle | 3-4%, soft |
| Immediately tackled long catch | 8-10% |
| Long run, scramble, catch-and-run, qualified return | 10-12% |
| Eligible breakup, knockout, confirmed hit-stick stop | 6-8% |
| Confirmed 5+ yard loss / fourth-down conversion | 8-10% |
| Turnover on downs | 10-12% |
| Interception / defensive fumble recovery or return | 12-14% |
| Forced fumble / fourth-down goal-line stand / Q4 high-leverage stop | 13-15% |
| Safety / game-clinching first down | 16% impact, 12% hold |
| Ordinary blocked field goal | 14% impact, 10% hold |
| Game-saving Q4/OT blocked field goal | 20% impact, 16% hold |
| Touchdown | 20% impact, 16% hold |
| Eruption | 30% impact, 22.5% hold |
| Native cannon blast | 15%, independent of attendance |

Spins, jukes and ordinary broken tackles remain sound-only. A generic hit stick does not acquire a blanket camera trigger. Own-team fumble recoveries cannot create a positive takeaway shake. Run/scramble/YAC buildup still starts at 13 yards and peaks at 20; kick/punt returns retain 30-to-40-yard buildup.

## Timing and settings

Routine impacts now last 2.3 seconds for stops, 2.7 seconds for catches/conversions/downs/takeaways and 3 seconds for forced fumbles/goal-line/high-leverage stops, at stock settings. Soft skill accents last 2 seconds. Live gains and returns retain their separate five-second post-whistle settling. The short-impact adjustment remains available and does not reuse that longer fade.

Forced-fumble onset retains its 35 ms attack and 50 ms handoff; the detector still needs verified proof. Event profile, leverage and onset style are latched with the admitted cue, so an empty outcome on the next frame cannot reset its strength. A lower incoming impact is consumed without restarting a stronger live effect.

An earned gain transitions from its current level to the touchdown or eruption peak over 300 ms, preserving oscillation phase and avoiding an early attack dip. It then settles to the hold level. Isolated scores retain their ordinary fast attack. Touchdowns remain 11 seconds and eruptions 16 seconds; safeties/clinching first downs/blocked kicks use the shorter 70% profile. Final-play carry retains its original deadlines.

The 35-control menu remains Attendance 10 / Soundscape 12 / Broadcast 13. Stock strengths are migrated once to the new defaults; custom values, zero settings, cannon strength and unrelated options are retained. Help and menu factories match the runtime.

## First live checks

1. Start with a smaller or medium home venue after the logs confirm layered atmosphere is active. Compare an ordinary lineup, a critical home-offense down and a 13-to-20-yard run or catch-and-run. A TD after that gain should build into a clear peak without dipping or restarting its motion phase.
2. Check a third/fourth-down breakup or hit-stick stop, a fourth-down conversion and a defensive takeaway. Routine stops should be weaker and shorter than takeaways; a verified force should hit sharply. Own-fumble pickups, ordinary broken tackles and spins/jukes must not create a new shake.
3. Compare the same camera in a large venue, then a neutral game. Large crowds remain stronger, with a much smaller small-versus-large gap. Neutral effects should be symmetric at 75%. Verify pre-punt motion stays restrained and old play motion clears at the next snap/replay/pause.

Unit and simulation checks establish calculated strength, timing, ownership and cleanup. Perceived strength, rendered motion and native replay/audio output require these live observations. This staged candidate is not installed or published by the agent.

## Verification

The final automatic native/managed build passed, including the new event-profile/curve/continuity suite, forced-fumble-to-return coverage, all ordered reaction transitions, native camera/return/replay/final-carry regressions, 490 real configuration-reader cases, settings migration and the 35-control menu. All 828 full-game attendance simulations and 8 Python catalog/compatibility checks passed. The isolated MMC host verified the adapter ABI, version metadata, embedded resources, extraction, settings storage and launcher logging. No live visual or audible result is claimed by these checks.
