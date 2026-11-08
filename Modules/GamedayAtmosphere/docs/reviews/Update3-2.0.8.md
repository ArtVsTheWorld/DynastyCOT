# Update 3 final review — candidate 2.0.8

## Delivered

The native reaction allowlist now contains the 80 audible selectors from the October 30 listening sessions. All four failed selectors (negative big-hit 13–15 and negative catch 23) and untested size 0 selectors are rejected. Negative catch intensity 3 falls back within its own family to 22. The catalogue includes both user reports and semantic use: [human-readable](../REACTION-AUDIO-CATALOG.md), [machine-readable](../REACTION-AUDIO-CATALOG.json).

The palette selects a matching family from existing verified outcomes. Generic cheers/applause-labelled cheers vary existing successful-play/lineup opportunities; anticipation varies an existing special lineup cheer; surprise varies confirmed breakup/knockout audio. Interceptions can also use the auditioned takeaway family. Actual ascending selectors are capped by event importance, measured occupied mapped seats and fullness. Variant choice is stable for a play/event/epoch. Native crowd-bus gain curves, reaction detection thresholds and shake strengths are unchanged.

A sound-only away counterpart classifies the same already-captured play/skill samples symmetrically. It owns no camera, energy, attendance or native memory writes. It retains one early opportunity plus a real score upgrade, cannot react to an own pickup without opponent-success proof, and resets on context interruptions. The main and shadow serials are explicitly reconciled after gaps. Neutral games skip home-negative counterparts and retain positive support for both teams.

The existing 36 menu controls remain: Attendance 11, Soundscape 12, Broadcast 13, toggles before numeric controls and no Troubleshooting section. The review kept master controls, crowd-mix strength, individual shake strengths, replay scope and attendance timing. It did not expose 80 sound selectors or diagnostics. Updated help explains semantic reaction variety, sound-only away counterparts and neutral symmetry. Help revision 50 refreshes documentation while preserving custom values/comments, schema 2 and reset marker 3.

## Integration defects corrected during the pass

| Finding | Resolution and regression evidence |
|---|---|
| Neutral away-designated live showboat could enter global boo path | Explicit neutral exclusion at the production decision; neutral classification remains positive-only in sound counterparts. Existing showboat/gate regression retained. |
| Audio attack-floor handoff recognized negative 95 only | Central family polarity now cancels positive carry for every audible negative selector. Regression iterates all negative families/sizes. The completed-impact compatibility guard also recognizes family polarity. |
| Existing random 59/83 swap could turn a catch into a sack cheer | Removed; deterministic variety is now selected among semantically compatible families upstream. |
| OT visitor-score situational path retained generic 95 | Passing TD/kick/other score mappings now use their actual score family while remaining sound-only. Dedicated OT mapping checks added. |
| Primary rushing-TD family 18 was excluded from receipt tracking as if always a secondary layer | Receipt follows primary dispatch order; a follow-up does not overwrite it. Synchronous rushing-TD native-voice fixture now verifies registration. |
| New help marker alone would not refresh existing 49 comments | Help refresh includes 50; migration regression preserves a custom disabled reaction switch, 23.25% boost and user note, then checks idempotence. |

## Cross-system review

| Area | Invariants checked |
|---|---|
| Peak / opening attendance | Existing grade/AP demand, native capacity/launch ownership, regular/postseason separation, opener/night/locality modifiers and bounded caps retained. No formula retune in this pass. |
| Arrivals / halftime / exits | Nested target masks, immutable native holes, exact 90% legacy pattern, close-game/comeback/OT protections, shared floors/caps, native phase handling and resumed history retained. |
| Native audio | Every audible selector exercises the same guarded event construction; invalid IDs fail before allocation. Existing one-shot IDs, reentrancy/ownership/expiry guards, bounded queue recovery and overlap policy retained. Queue/voice registration is not heard output. |
| Motion | Occupied-seat curve, fullscreen camera ownership, layered gates, 75% neutral strength, OT multiplier, sound-only skills, own-fumble exclusion and score/final-play handoffs retained. No new camera trigger is fed by the away shadow. |
| Replay / presentation | Existing mode-aware full-screen/inset policies, fresh graph reads, scoped restoration, injury/native interruption behavior, historical-crowd playback and final cleanup retained. This integration does not force the game to render every native replay scene. |
| 2020 Mode / teardown | Independent visible-empty and crowd/band override, native planned turnout/history preservation, exact current/replay target restoration, live toggles and restore/stop cleanup retained. |
| Performance | New palette/lookups are bounded in-memory operations; shadow models reuse existing captured samples. No added actor/graph walks, native reads, per-frame disk I/O or system calls. The user-verified 2.0.5 replay hitch fix and 2.0.6 hot-path changes remain intact. Native fixture timing is not FPS or a live 2.0.8 hitch test. |
| Settings / compatibility | Three sections, 11/12/13 counts, ordering, IDs/ranges, hidden backend preservation, future-schema guard and isolated MMC extraction/storage checks retained. No installed files were changed. |

## Validation

- Full `Source/Build.ps1 -Automatic` native/managed regression suite passed with MSVC warnings treated as errors.
- 828 full-game production-policy attendance simulations and 497 real native configuration-reader cases passed.
- Nine Python catalogue/menu/compatibility tests passed, including JSON-to-runtime allowlist equality.
- New tests cover all 80 selectors reached through the semantic palette, all 80 dispatched through a native-shaped fixture, all 4 exclusions, stable variation, density/occupied-seat/importance ceilings, away-only ownership, no sim/kneel/gap backfill, catch-to-TD upgrade and main/shadow serial alignment.
- After the primary-receipt correction, the final native DLL was rebuilt and the palette, away counterpart, reaction runtime, population, handoff, live reaction, production seating, configuration and native menu-reader tests passed again.
- Final isolated MMC ABI/metadata/resources/extraction/storage checks passed against the rebuilt DLL. Python and diff checks passed.

Private verification logs: `Build/Build-208-verified.log`, `Build/FinalNative208.log`, `Build/PythonTests-208.log`, `Build/HostCheck-208-verified.log`. The staged package records commit and SHA256 values in BUILD-INFO.json.

No further release-blocking failure was found in these checks. This is source/fixture/simulation validation, not a guarantee of bug-free live play, audible sound, rendered sequence, restored appearance or FPS. The five final acceptance checks are in [the release test plan](../UPDATE3-TEST-PLAN.md). No installation or publication was performed.
