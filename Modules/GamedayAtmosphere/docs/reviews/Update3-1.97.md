# Update 3 — seat desirability and smaller gaps

Candidate 1.97.0 restores the pre-1.96 spatial pattern as its base. The user rejected the broad patches and upper-bowl appearance in 1.96, then preferred this revised candidate during a live comparison and approved staging it.

## Scope

Keep the original row/depth retention, seeded spatial fields, family groups and stragglers. Below 90% retained crowd, gradually add a bounded preference for midfield sidelines and useful viewing height, with earlier thinning toward end zones. Occasional short row-aligned gaps replace the extra broad patches added in 1.96. The adjustment reaches its full blend at 50% retained crowd. At 90% and above, the pre-1.96 seat selection is exactly preserved for the same geometry, native baseline and match seed.

Geometry is a heuristic for viewing desirability, not native ticket prices, deck IDs or verified seating sections. The general front/depth advantage remains, but proximity to the field alone does not determine which seats stay occupied. Native holes remain empty. The same target still produces the same total number of populated model positions; this is spatial selection only.

No NativeRuntime, arrival/departure formula, timeline/history format, replay, audio, camera or crowd-animation logic changes are included. Pattern preparation runs once per renderer; visibility retains its existing rank lookup. No new performance measurement is claimed.

## Approved live comparison

The installed runtime was 1.96.0, on Kyle Field with 56,099 originally populated model positions. The existing 1.96 rank vector was reconstructed offline and matched byte for byte. Old-base and candidate ranks were built from that exact geometry, original mask and seed. A bounded preview substituted only DGA-owned rank contents and existing preview controls, using the native visibility refresh. No DLL installation, executable-code change or new hook was used.

Both patterns were compared at 70%, 50% and 30%; the candidate was also viewed at 90%. The 70/50 screenshot pairs shared a camera composition. The camera moved between the 30% captures, so that pair supports qualitative comparison rather than an exact screenshot overlay. The rank/model count comparison held geometry, baseline and seed constant throughout. The user chose the candidate and explicitly approved staging it.

| Retained crowd | Visible model positions | Midfield sideline: old → candidate | Lower end zone: old → candidate |
| --- | --- | --- | --- |
| 90% | 50,490 | 98.3% → 98.3% | 95.8% → 95.8% |
| 70% | 39,270 | 89.7% → 95.7% | 82.5% → 80.2% |
| 50% | 28,050 | 74.1% → 93.6% | 63.3% → 56.7% |
| 30% | 16,830 | 50.7% → 74.4% | 40.2% → 33.6% |

These are region occupancies measured on one native model layout, not real attendee counts or universal stadium targets. Midfield is the heuristic lower sideline region within 22 m of midfield; lower end zone is beyond the field end line and below 20 m in the existing field-aligned coordinate system. Native model positions do not necessarily equal ticketed capacity.

Restoration was verified: original rank contents, all borrowed DGA preview controls, original 60% visible crowd (33,660 positions), and unchanged attendance history. The preview process exited successfully. Private evidence is in Build/SeatDesirability1970/live-review.json, live-layout-metrics.csv, PatternChecks.log and control-health.json.

## Validation and remaining check

The full automatic native/managed build and regression suites passed, including 828 production-policy full-game simulations. All eight Python tests passed. Isolated MMC ABI, metadata, resources, extraction, storage and logger-fallback checks passed. Targeted tests cover exact legacy 90/95/100% fingerprints, native-hole exclusion, exact/nested counts, repeatability, desirability/group bounds and multiple seeds. Every loss threshold on the captured live layout also passed nested-count and native-hole checks.

The live preview establishes acceptance of this pattern in this venue. The packaged 1.97 adapter still needs an ordinary installed-build smoke test, and other stadium geometries have not received live visual acceptance. Fresh games can use different seeds. No game/MMC files, INI or saves are changed by staging; nothing is installed or published. Crowd-animation research is separate from this candidate.
