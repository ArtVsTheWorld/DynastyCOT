# Update 3 Hotfix

Version 2.0.35 · All changes since 2.0.21

**Install:** close the game and MMC, unzip, and copy both `DynamicGamedayAtmosphere.dll` and the `DynamicGamedayAtmosphere` folder beside it into MMC's `Plugins` folder. Merge folders and replace matching files. The Markdown files are just reading material

**Settings & Installation**
- Gameday Atmosphere is now available under Pause > Options > Settings, at the far right after Auto-Subs, sharing the same saved settings as the Dynasty/RTG tab
- 44 plainly explained settings, including separate Standard, Zoom, All-22, Wide and Broadcast shake sliders from 0–200%
- The ZIP opens directly to the DLL, its matching runtime folder and documentation, with the loader packaged separately from the DLL
- Fixed deeply nested settings paths and improved launcher errors, including the failing file and operation
- Existing Update 3 settings are preserved, while Update 2 settings still receive the intended one-time refresh automatically

**Camera Shake**
- Shake balances itself using the actual lens and FOV, including modded cameras, with tighter Zoom-like views stronger than Standard/wider views
- The latest Standard/wider tuning includes the tested extra 10% reduction, while the accepted Zoom strength is retained
- All camera sliders default to 100 for the new balance, so return any temporary 90% test settings to 100 after updating
- Away Team Reaction Shake is now a 0–100% slider, defaulting to 0, with 100 matching the home-team reaction
- Neutral-Site Crowd Shake is a separate 0–100% slider for both teams, defaulting to the existing 75% balance
- Trucks are 50% stronger, confirmed big hits use the same strength on defensive stops or losses, and immediately tackled big catches get a stronger impact
- Improved touchdown and eruption shake through brief sound handoffs and celebration-camera changes to reduce the stop-start feel
- Opening-kickoff shake is gentler at 6–8% before crowd and camera scaling

**Sound & Reactions**
- The visitors' first offensive play gets a faster, fuller hostile roar and swell
- Eruptions use the biggest cheer and roar variants, while pass-flight buildup uses the strongest native swell
- Negative visitor reactions cover gains, catches, skills, returns, takeaways and defensive stops, with optional shake following the away slider
- Independent skill, result and celebration sounds can overlap each other and vanilla reactions, with a separate slot for coinciding skill reactions
- New reactions no longer cancel already-scheduled follow-up cheers, and away shake follows its verified play through sound and whistle handoffs
- Close one-score games build atmosphere for either offense throughout the last two minutes of Q4, with the usual crowd requirements

**Attendance**
- UCLA gets a 14% soft demand discount and USC 10% at regular home games, with sellouts still possible and protected rivalry turnout preserved
- USC and Virginia join the late-arrival group

**Fixes & Performance**
- Cut repeated camera-settings checks, protected against incomplete memory reads and kept projection fallbacks tied to the correct camera and game
- Clearer technical camera output and more reliable diagnostic logging, while preserving the details needed to troubleshoot
