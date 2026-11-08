# EA crowd types and protected sections

EA's crowd placement data uses 12 labels:

| Value | Meaning |
| --- | --- |
| 0 | Regular spectator |
| 1 | Band: cymbals |
| 2 | Band: snare drum |
| 3 | Band: bass drum |
| 4 | Band: drum set |
| 5 | Band: trumpet |
| 6 | Band: trombone; EA spells this label `TROMNONE` |
| 7 | Band: saxophone |
| 8 | Band: tuba |
| 9 | Student |
| 10 | Military |
| 11 | Always-standing home spectator without a prop |

## Academy sections

The checked Army, Navy and Air Force stadium placement files use the student
label for their academy sections. They do not use the military label there.
EA's student outfit settings select the schools' uniforms. The same outfit
system also includes Texas A&M and Virginia Tech, so a uniform alone does not
prove that an entire section must stay for the whole game.

The checked files contain 958 student positions at Army, 3,939 at Navy and
1,710 at Air Force. These are authored crowd positions, not real attendance
figures or the total number of students at each school.

The candidate protects:

- All eight musician types from DGA crowd thinning
- Military-tagged spectators
- Student-tagged spectators belonging to a loaded school whose native
  `IsMilitary` field is verified and true

Regular student sections still thin normally. Standing spectators are not
automatically exempt. Protected groups keep their native outfits and are
excluded from visitor appearance reassignment. Cadets and midshipmen count as
fans for fullness and supporter balance; band members do not count as supporters.

Protection prevents DGA from withholding or removing these spectators. It
preserves native model visibility and camera culling. 2020 Mode deliberately
empties these sections too. School identity is captured separately from applying
the revised attendance formula, including the native resume/postseason paths.
If the school identity cannot be verified, DGA does not guess from a team ID.

The band protection passed a live comparison with ordinary fans thinned around
the band. Academy-specific sections still need an in-game check before release.
Native label, stadium placement and school-field evidence is preserved under
`Build/Update4NativeResearch` in the development workspace.
