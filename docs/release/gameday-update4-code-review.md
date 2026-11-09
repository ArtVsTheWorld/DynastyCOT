# Gameday Atmosphere Update 4 comparison

Compared the COT integration source directly against local Git tag `v2.0.35`, commit `193662a9af440ef9cdc6903171e1a30b8acb0164`. The standalone checkout was read only. 53 production source/build/configuration files differ (line endings normalized). Discord draft: 1692 characters.

| Public claim | Current implementation | Baseline result |
|---|---|---|
| Audience/TeamBuilder turnout | [AudienceModel.h](../../Modules/GamedayAtmosphere/Source/AudienceModel.h#L13) | New source file |
| Visiting demand and distribution | [AwayCrowd.h](../../Modules/GamedayAtmosphere/Source/AwayCrowd.h#L30) | New source file |
| Winning visitor departures | [AwayCrowd.h](../../Modules/GamedayAtmosphere/Source/AwayCrowd.h#L226) | New source file |
| Persistent crowd plans | [VisitorPlans.h](../../Modules/GamedayAtmosphere/Source/VisitorPlans.h#L10) | New source file |
| Protected groups | [ProtectedFanBudget.h](../../Modules/GamedayAtmosphere/Source/ProtectedFanBudget.h#L7) | New source file |
| Shake/sound pairing | [MotionSoundPairing.h](../../Modules/GamedayAtmosphere/Source/MotionSoundPairing.h#L6) | Changed since 2.0.35 |
| Deficit-sensitive background | [BackgroundRoar.h](../../Modules/GamedayAtmosphere/Source/BackgroundRoar.h#L19) | Changed since 2.0.35 |
| Replay destination handoff | [MajorReplay.h](../../Modules/GamedayAtmosphere/Source/MajorReplay.h#L7) | Changed since 2.0.35 |
| Direct managed loading | [ManagedRuntimeLoader.cs](../../Modules/GamedayAtmosphere/Source/ManagedRuntimeLoader.cs#L27) | Changed since 2.0.35 |
| Fumble team proof | [LiveFumble.h](../../Modules/GamedayAtmosphere/Source/LiveFumble.h#L4) | Changed since 2.0.35 |

The comparison checks implementation changes, not rendered or audible behavior. Existing 2.0.35 features such as in-game settings, general arrivals/departures, camera preferences and reaction sound variety are not presented as new Update 4 features. Kicker celebrations are not advertised: a live kicker shot did not establish celebration animation support.

Branding changes retain the native scope, signature and history checks. No claim of universal overhaul compatibility, measured FPS improvement or native gameplay home-field advantage changes is made.
