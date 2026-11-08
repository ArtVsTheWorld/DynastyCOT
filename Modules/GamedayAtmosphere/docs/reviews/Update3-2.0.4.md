# Update 3 candidate 2.0.4: post-play hitch investigation

## Evidence and confidence

The user reported a hitch after almost every play during BC away at Cincinnati on 2.0.3. With DGA disabled, the user then observed no post-play hitches across more than ten plays. This is strong DGA-associated A/B evidence. It does not isolate a particular function.

Cincinnati used 18,288 mapped positions. Arrivals reached 100% during Q1 and the hitch continued. The initial 101.8 ms attendance-decision maximum already existed before kickoff; there was no new >=16 ms decision hit in the last preserved status. The atmosphere callback stayed below 0.7 ms. Those counters excluded replay crowd restoration and natural engine visibility updates, so they could not clear those paths.

Source inspection found that replay entry forced a complete native crowd update even when historical and already-applied live targets matched. Replay exit forced another update unconditionally. Routine replay opportunities can repeat those transitions after plays. The redundancy is established in code; its contribution to the observed frame hitch requires live verification.

## Change

Record the target and mode only after a verified successful visibility application. A replay refresh skips forced native traversal when that same target/mode is already applied and there is no renderer-generation change, retry or pending layout. Genuine target changes and restoration still apply normally. Cache state is invalidated on match reset.

Natural visibility callbacks now mark replacement seated renderers/mask buffers dirty even at 100% live attendance. This keeps an equal-target optimization from overlooking streamed geometry. Tiny sideline draws remain optional. Historical attendance, original empty seats, pattern ordering, arrivals, departures, audio, reactions and gameplay rules are preserved.

Add independent wall-time counters for crowd application, replay crowd checking, forced native crowd passes and natural crowd updates. Forced/native measurements include engine work plus DGA visibility handling; the natural measurement also includes controller observation. None is an isolated whole-plugin CPU or frame-time measurement. Timing is once per pass/callback, never once per spectator.

## Offline checks

The production scene test exercises 20 unchanged replay entry/exit cycles with no added forced native passes. Different historical targets and restoration to live density still traverse and restore the correct masks. Additional cases cover equal nonzero targets, new controller generations, retries, missing native callbacks, explicit restore mode, cold cache and a replacement mask buffer discovered by natural callbacks at 100% attendance. The existing scene regression continues through Midgame Save, native baseline restoration, Sim, halftime, departures, streamed scenes and replay seeks.

A private synthetic scene benchmark measured roughly 1.8-1.9 ms for a forced 18,288-position pass and 5-6 ms for 56,099 positions on this run. The equal-target entry/exit checks performed no native pass and fell below the three-decimal millisecond display. The native test callback is a stub; these values are mechanism evidence, not measured game stalls or proof that the hitch is gone. Natural DGA visibility overhead remains a separate possible contributor and is now timed during live play.

## Required live retest

After closing game/MMC, install the staged 2.0.4 adapter and confirm the runtime version. Repeat a comparable BC-Cincinnati game with the same camera, graphics and other plugins. Play at least 10-15 ordinary plays, allowing several automatic/inset replays to start and finish. Note whether the hitch occurs at the whistle, replay entry/exit or play selection. Keep status and normal logs; the new counters distinguish replay/application work from natural crowd updates.

Check one genuinely changing crowd/replay state during Q1 arrivals and one pause/interactive replay return. The visible crowd must still restore correctly. Keep release acceptance pending until this live comparison confirms the hitch is cleared or identifies the remaining path. The agent did not install the candidate or change the user's disabled game/MMC setup.

## Build verification

Full automatic native/managed regression checks and 828 attendance simulations passed. Eight Python catalog/compatibility checks passed. After adding the natural-update timing scope, the final native DLL was rebuilt and the production scene regression rerun successfully. The isolated MMC host verified the final payload, metadata, resources, extraction and storage. Live hitch clearance remains pending.
