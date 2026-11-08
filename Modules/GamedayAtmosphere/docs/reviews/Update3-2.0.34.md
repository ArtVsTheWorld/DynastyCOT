# Update 3: 2.0.34 broad-view shake adjustment

The user accepted Zoom in the latest candidate and asked for Standard and wider views to reach the 2.0.32 strength. The broad-lens multiplier changes from 0.85 to 0.75. With the common 1.10 reference, that gives exactly the 0.825 gain from 2.0.32: about 11.8% below 2.0.33 for broad framing. The tight-lens multiplier stays at 1.05, preserving the accepted Zoom strength.

The existing smooth transition uses the same actual projection anchors: about 54.45 degrees vertical FOV for broad framing and 37.90 degrees for tight framing. Intermediate lenses blend between 0.75 and 1.05. Camera preset names and IDs do not control this curve; modded lenses receive the same treatment. The lens does not identify physical camera height.

All five personal camera sliders still default to 100, and saved choices are preserved. Event strengths relative to each other, occupied-seat scaling, audio, attendance, reaction handoffs, safety bounds and accessibility controls keep their existing behavior. The technical status and full feature list reflect the new broad multiplier.

The projection regression compares broad output with the historical 2.0.32 gain and tight output with the accepted 2.0.33 gain across every effect category. Runtime fixtures also compare actual output matrices for broad, tight and intermediate lenses at four personal preferences. The curve sweep retains its boundedness, continuity, monotonicity and aspect checks.

Exact build and package results are recorded in the private staging BUILD-INFO. The revised broad-view strength still needs a live visual check. Matrix estimates do not measure visible pixels or FPS. This work stages the build privately without installing, pushing or publishing it.
