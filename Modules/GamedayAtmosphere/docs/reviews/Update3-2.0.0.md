# Update3 / 2.0.0 test candidate

The new native bus mix replaces both sparse-crowd excitement suppression and the older graph-volume parameter cut. One owned, immutable native preset contains Crowd plus Anticipation, Beds, Chants/Response, Reactions, StudentSection, Sweeteners and Swells. All eight entries fade and retire together; only Gain and the zero-crowd parent mute state are changed. Existing user audio sliders, commentary and field buses are not written.

Fullness and mapped occupied seating determine a30-70% size-dependent vanilla crossover. Packed large crowds reach the 30% positive signal-gain cap. Sparse listening anchors preserve the approved70/90/50% Kennesaw comparisons, with the user-directed final30% bed/chant-9.5dB, reaction-7.75dB and detail+0.75dB. Individual detail may be more prominent in sparse crowds than under the denser bed; it does not need to be monotonic independently. Continuous beds/collective reactions remain monotonic. The brief9/18dB diagnostic boosts are excluded.

The old quieter-voice selector, sparse excitement scaling and soft stadium-size ceiling are removed. The remaining native excitement helper can only raise earned reactions above the native envelope; it cannot lower native excitement. Graph-volume parameter writes are removed from the production callback. Existing eligibility, sound-only vs layered reactions, Cannon, shake gates, manual/Slow Sim replay handling, event dynamics and attendance policies are preserved. Passive mix gains replace the former independent normal/high-leverage/eruption volume bonuses; event/roar energy still comes from their existing native requests and upward excitement.

All gains are quantized to avoid insignificant preset churn, with the positive cap applied exactly. User-slider/context disable and zero mute retire or replace immediately. Other changes are spaced beyond the owned quarter-second fade. Presets remain immutable and retained while native transitions can reference their storage. Manager replacement drops handles retired by the engine; release faults stop starts but continue retrying retirement. Unknown layouts fail closed to native audio.

The35-control menu retains Attendance10/Soundscape12/Broadcast13. The former small-crowd excitement slider becomes Crowd Mix Strength. Help revision47 preserves user values and personal notes. Old advanced limiter/cut/chant fields are retained as ignored compatibility data.

## Verification and limits

Native model tests cover the listening anchors, invalid input, occupied-size ordering, crossover neutrality, gain cap, chant parity and zero mute. Runtime tests inspect the eight native entries, state/Gain properties, stable updates, immediate mute, ownership/manager replacement and retirement after a release fault. Integration fixtures exercise paused intro, historical automatic replay, interactive replay, ordinary pause and postgame behavior. Full build/simulation, Python and isolated MMC host results are recorded in the staged BUILD-INFO.

Auditions establish listening preferences in one small venue. Large-venue levels, audible zero mute and production presentation transitions still require installed-build checks. The18dB individual-detail test produced little/no audible difference despite observed requested gains: source routing/event availability or effective gain remains unresolved. Do not describe that control as proven audible. No new custom voice source is invented.

Source save DYNASTY-DGA183 is read only. The Week1/2 checklist uses its saved fixtures. No installation, game modification, save modification or publication is performed by the build task.
