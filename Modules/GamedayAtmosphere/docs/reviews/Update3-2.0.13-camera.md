# Update 3 camera correction — 2.0.13

## Observed problem

The 2.0.12 live status reported a 7.2% critical-down envelope, 3.1× view compensation and 22.2% final DGA output in the user's normal game. Native HFA was also reported active in that sample. This proves the DGA amplification; it does not isolate the native HFA camera's visual contribution.

The general projection correction allowed up to 4× after earlier wide-player/RTG camera testing. The captured wide-player projection and the reported ordinary view overlap, so field of view alone cannot identify RTG. No guessed game-mode or camera-type binding has been added.

## Change

- Default wide-view compensation is capped at 1× for every added motion category, including touchdowns, live reactions and cannon impacts. Existing close-up attenuation and invalid-projection fallback remain.
- The previous bounded wide-player compensation is retained through `WidePlayerCameraCompensation=1` in the advanced Broadcast INI. It defaults off, is not added to the 36-control menu and is explicitly described as an opt-in for wide RTG/player-lock views. It must be turned off again for normal broadcast views. This is not automatic mode detection.
- Native HFA camera motion yields while a fresh DGA crowd-shake output is actually being rendered for the current owned match and cue. Prepared requests and mere eligibility do not claim motion ownership. The latest native camera tier is remembered and restored when the DGA output stops. Independent cannon output does not claim this crowd-motion ownership.
- Only the native camera event channel changes. Gameplay HFA, composure, play art, the meter and native audio remain unchanged. Existing crowd/context admission, accessibility, pause/replay checks, scene ownership and the native-filter zero opt-out remain.
- Technical status reports the compensation policy and corrects the obsolete two-voice cheer-limit wording. Native/DGA cheer overlap remains enabled.

Occupied-seat scaling, configured event strengths, attendance, reaction timing and audio curves are not retuned. The settings reset generation stays at 4; existing current-generation custom settings are retained and the new option defaults off.

## Checks

The renderer fixture reproduces the captured 7.2% case: default output remains 7.2%, while the explicit wide-player path retains its approximately 22.2% result. Both policies run through all motion categories, wide/close projections, output-matrix guards and unchanged source/cue timing. The captured RTG impact/hold progression remains covered under explicit opt-in.

Native HFA tests exercise idle admission, rendered-DGA ownership, updated native tiers while suppressed, latest-tier restoration, unrelated event channels, balanced references, stale/wrong-world rejection and the explicit filter opt-out. The configuration audit includes the new option's default, range, invalid values and disabled behavior. Full build, Python, mapped-image and packaged MMC results are recorded with the staged build.

## Live check

Install with the game and MMC closed, then repeat an ordinary lineup, defensive third down and touchdown in the same game. With the new option off, technical status should show no view amplification above 1×. Check native HFA camera restoration after the DGA effect ends and confirm gameplay HFA remains present. Repeat a wide-player view separately with the advanced option enabled only if extra compensation is wanted. Automated checks do not establish the new build's perceived strength or eliminate the need for this short comparison.
