# **Update 5 Hotfix**

**More Reliable Startup**
- Added the same startup protection tested in Beyond the Field. If Windows is briefly updating the game's loaded-file list, GDA retries that check instead of immediately giving up.
- This also protects the final check after loading, which could otherwise report failure even when the runtime was already present.
- Retries are brief and limited to checking that list. The plugin does not repeat an uncertain runtime load.

**What Stays the Same**
- Crowd plans/history, settings, attendance, reactions, audio, camera and replay behavior are unchanged.
- This is preventive hardening for the shared loader. GDA did not reproduce BTF's failed startup in the observed session; automated tests are separate from live acceptance.

**Install**
- Close MMC and the game, then replace **GamedayAtmosphere.dll** in your usual **Plugins** folder. COT users can explicitly check for updates and choose **2.1.39** for GDA only.
- Separate BTF missing-path reports remain under investigation pending their logs. This release does not claim to fix them.
