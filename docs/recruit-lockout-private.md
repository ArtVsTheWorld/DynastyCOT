# Recruit lockout divider — private 1.0.46

The active November 26 read-only inspection matched Cayden Pinkney by both names
and Recruit serial `4281:524` (UI ID `561119756`) in League `1044222270`.
Two reads of the native board-model record returned `Locked=false`, native
interest rank 4 and UI stage 2 (Top 3). The native serializer at `0xa0e96ca`
reads its `+0x60a` byte and submits that value under the `Locked` property.
Default was visually inspected too: East Point remained below Locked Out;
the user confirmed actions remained editable. This is not solely a Feedback
grouping defect. DRIPlus RM18R22 is active, but causation was not established.
Do not describe this investigation as proof that DRI changes that rule.

The verified Overview method inserts its divider at a stage-dependent ordinal
and shades rows from that ordinal. It does not consult the current-school
`Locked` flag. Feedback can additionally reorder schools within broad tiers,
so using its new positions loses the native cutoff membership.

The fix carries each school's original native ordinal in transient UI metadata.
Native entry routes that did not run the optional DTO transform keep their
original row index, but only when neither reordering marker is present. Missing
metadata on an already-transformed row never authorizes that fallback.
The same verified Overview method decorates the matching school row with its
native stage membership, using the existing native cutoff and stage constants.
For the current user's school, the actual native `Locked` boolean wins over
position. A bounded post-population pass partitions the existing row objects
and moves the existing divider; order within each group is preserved. An
already-correct list incurs no provider invalidation. An empty locked group
does not retain a dangling divider. Earlier stages do not gain new headers.

For other schools, preserve the native stage membership, not a new prediction
of their dealbreakers, coach abilities or mod-specific recruiting permissions.
The DTO does not expose a separate authoritative Locked boolean for each rival
school. Only the current-school flag is directly verified; no claim is made
that rival-team action permissions were individually measured.

Default/Rank Only keep native numbers and ordering within these groups.
Feedback/Blind keep their presentation and concealment. Missing/malformed
ownership, stage, order or boolean data retain native presentation rather than
inventing eligibility. Native callbacks remain protected by complete method
fingerprints, class/method discovery, dependency and Dynasty scope checks.
The correction runs during population, not Draw/focus. It changes no Recruit,
Player or interaction records, native lock flags, actions, hours, NIL, visits,
commitments, selection on the Recruiting Board, saves or published history.

The native board-model read was read/query only. Development did not inject,
install, restart or publish. Automated VM/C++ fixtures validate presentation and
source contracts; candidate menu rendering and live callback cost still need
manual-install acceptance. Test both a truly locked recruit and an unlocked
school outside the nominal stage cut, then switch among all interest modes and
reopen cards. Keep the game running until ready for a normal manual DLL update.
