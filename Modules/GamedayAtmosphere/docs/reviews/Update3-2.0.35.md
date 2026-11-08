# Update 3: 2.0.35 final shake tuning and hotfix publication

The user tested Standard/wider camera preferences at 90% in 2.0.34 and accepted that strength. The broad framing multiplier changes from 0.75 to 0.675, folding the extra 10% reduction into the default 100 setting. The accepted tight-view multiplier remains 1.05. The common gain stays 1.10, giving broad output gain 0.7425 and tight output gain 1.155 before optical correction and personal preferences.

The same smooth lens curve covers intermediate and modded cameras. No preset-ID dependence, extra native reads, settings reset or live configuration changes are introduced. Existing personal percentages are retained; users who left their temporary test sliders at 90 should return them to 100 to avoid applying that reduction twice.

Projection tests compare broad output with 2.0.34 at preference 90 and tight output with the accepted 2.0.33 gain. Actual runtime fixtures retain broad, intermediate and tight matrix checks across every effect category and four personal preferences. Full build and packaging evidence is recorded privately under Build/Publish2035 and the staged BUILD-INFO.

The hotfix notes are cumulative from the v2.0.21 source tag. The original Update 3 notes remain historical. The published hotfix is updated to the verified 2.0.35 source and ZIP, with the previous source tag retained rather than moved. The external loader and flat four-file package layout remain in use.

The latest live evidence is the user's accepted Zoom view and 90% broad-view test in the preceding builds. Matrix equivalence is not a new rendered-pixel measurement or an FPS result. Attendance, sound curves, trigger eligibility and reaction timers receive no new tuning in this revision.
