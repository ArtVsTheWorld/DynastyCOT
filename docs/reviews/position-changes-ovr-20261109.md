# Position Changes projected OVR correction

The live Position Changes preview exposed exact projected overalls even with
Roster Fog enabled. Cole Sullivan showed 82/81/80 in the destination-position
table while the sidebar already used grades. This was a separate native
PositionChangesProvider, outside the existing roster display adapters.

The new adapter retains the first 180 bytes of the fingerprinted native
GetRowData method. Its display-only suffix changes the projected OVR column
(native key1). The returned array, numeric values, sorting keys, position-change
commands, depth counts, disabled rows and native selection styling are retained.
It uses the existing player grade scale and fog options. Dynasty presentation
scope, full native body fingerprints and hook ownership checks remain required.
The bank is appended to the existing indexed dispatcher; existing bank indices
and the working NIL, Ratings and navigation implementations are unchanged.

No save, player position, settings file, installed DLL or published history was
changed by this test. Live helpers were source-built under ignored Build and
loaded only into the authorized running process. They last until game exit.
No release was published.

## Verification

- Observed the original numeric leak in the running game.
- User confirmed the first live adapter worked. Captured Sullivan's projected
  grades (B-, C, C-, F) with native counts and selection colors intact.
- Final live capture also showed Jermaine Flynn's projections as D+, D, D-, F.
- User found Back/reopen slightly slow and could not distinguish an improvement
  after the leaner helper was loaded. A live speed improvement is **not proven**.
- Reduced the general string-rating whitelist to a short numeric conversion
  path in source. Seven alternating batches of identical inputs, 1600 rows per
  batch, took median415.77ms before and139.17ms after in the Python bytecode
  verifier (66.53% reduction). This is not a game/FPS/menu timing measurement.
- 10,123 focused checks passed: all five player grade modes, boundaries,
  numeric/canonical-text inputs, native styles, unchanged underlying values,
  reordered/absent/duplicate columns, reused displays and option behavior.
- Complete recruiting-fog build script passed from its module working directory,
  including existing NIL edits/deferred redraws, prepublication Ratings,
  highlight, circle fitting, Dynasty scope, currency isolation and player UI
  regressions. Native suites passed7317 model/distribution checks,2491 Analytics
  integration checks, recruiting-fog checks and267 native/owned dispatch shapes.
- Dispatcher fixture:10,000 dispatches1.7403ms linear versus0.2191ms indexed;
  this is an offline lookup benchmark, not live menu responsiveness.

Private evidence: Build/PositionChangesLive. Native build output:
Modules/DynastyEnhancer/Build/BeyondTheFieldRecruitFog-p202611100118.dll.
This diagnostic build has its own Analytics build ID and must be paired with a
matching social native in the next package; it is not a drop-in distributed DLL.

## Remaining acceptance

Confirm Back/reopen timing in a clean, packaged process without temporary
helpers. Check player switches and the currently active AIO combination again
after packaging. Grade modes and fog Off behavior passed automated fixtures;
not every option was toggled in the user's live Dynasty during this test.
The separate GamedayAtmosphere2.1.10-td1 replay candidate remains pending its
clean-install Play Now and touchdown acceptance tests.

## Authorized 1.0.3 release follow-up

The user subsequently authorized publishing this Beyond the Field fix. Both
native engines were source-built with the same Analytics build ID. The
standalone builder now embeds that tested fog engine instead of retaining the
older reference bundle's fog bytes. All other baseline content/helpers and
data paths remain unchanged. The actual standalone DLL passed750 isolated
integration checks covering duplicate loading, data preservation, extraction,
coexistence and COT switching. Release integrity metadata pins the exact
source revision, native payloads and ZIP/DLL hashes. The private replay changes
are excluded from this source commit and release.
