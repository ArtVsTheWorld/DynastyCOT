# Wide-player camera toggle — 2.0.14

Adds **Wide Player Camera Compensation** under **Broadcast**, directly after Added Camera Shake. It is an On/Off toggle, defaults Off and writes the existing `WidePlayerCameraCompensation` backend key. The help explains that it is for wide RTG/player-lock views, can substantially increase shake and should be turned off again for normal cameras.

The page now has 37 controls: Attendance 11, Soundscape 12, Broadcast 14. Toggles remain before numeric controls and every section stays below 15. The INI remains supported. The native camera policy, shake profiles, HFA handoff, audio, attendance and default settings are otherwise unchanged from 2.0.13; the reset generation stays at 4.

The native menu fixture turns the new toggle on, closes/reopens the page, verifies the saved key, then turns it off and verifies it again. Existing catalogue, bounds, ordering, hidden-setting preservation and future-schema checks remain. Full build, Python, mapped-image and packaged MMC checks are recorded with the staged build. The next installed-build check is to open Broadcast, change the toggle and reopen the page; leave it Off for normal-camera testing.
