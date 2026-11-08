# AIO Broadcast settings compatibility: private 2.1.6

## Report and cause

The reported Gameday Atmosphere tab remained visible but appeared to contain
another menu's controls. This report came from other users; it has not been
reproduced visually on this PC.

The supplied AIO-Broadcast-Beta1.fbmod identifies itself as 0.7-beta1. Its SHA256
is ef277cbda5e2603cfc87c25d74b3479ec47f5c24ba34a7a0a182355f4173f51e.
It replaces the shared common and frontend Scriptpod assets. Its active common
GenericSettings initializer is appended at 0x45b30, while the original at
0x237c0 remains in the file. The root's control 37 points to the appended copy.

DGA previously changed the unknown-tab fallback at 0x2892c in the original
copy. AIO's active copy has the same category switch at 0x4af30 and its fallback
at 0x4b1a4. Without DGA's extension there, its unknown category returns zero,
which selects native settings. This explains a visible tab with wrong contents.
The complete 663-byte route function and its 668-entry class dictionary match
the stock versions exactly. AIO's separate broadcast controls do not use DGA's
category or option IDs.

## Change

- Follow the active initializer through the supported common movie's root and
  control descriptors, rather than assuming the old method offset is active.
- Walk only that initializer, skipping function bodies by their declared sizes.
  No heap scan, other movie scan or disk UI modification is added.
- Check the whole route function outside the existing 32-byte DGA fallback,
  plus argument/register layout. Accept only a stock fallback or DGA's own one.
  Ambiguous, truncated, unfamiliar or externally changed routes are rejected.
- Support file-relative pointers and native pointer fixups, with guarded
  reads, a matching cached movie/root relationship and a 2 MiB offset limit.
- Change only the active fallback. Existing category cases and AIO's broadcast
  code remain untouched. The same helper serves frontend and pause settings.
- Record one readable compatibility message when the active route moved.
  Parsing occurs when listing settings tabs, not during gameplay frames.

The native settings provider's existing row, vtable, ownership and save guards
remain. No settings schema, reset policy, option count or gameplay tuning is
changed by this fix. The prior Update 4 development work remains included.

## Verification and limits

Permanent regression fixtures cover active initializer selection, stale copies,
both action-control forms, pointer fixups, repeated opening, ambiguous routes,
foreign edits, unknown opcodes, bad pointers and truncation. An additional
read-only integration run uses the locally extracted stock and supplied AIO
APT files. It verifies their actual active offsets, successful repeated routing,
and that changes are confined to the 32-byte fallback. No extracted game or
third-party mod assets are committed or packaged.

Offline results do not establish that either menu renders correctly in the
live game. Test with AIO enabled: open DGA, change a harmless setting, leave and
reopen it, then verify the broadcast switcher still works. Repeat from the
pause-menu settings. Unknown later AIO revisions require their own check.

Build and integration evidence is under Build/AIOBroadcastCompat-20261105 and
Build/Update4Aio216. Nothing is installed or published by this work.

The full build completed with 159 PASS lines and 936 full-game simulations.
After the cached-root relationship was checked against preserved native-cache
evidence, the final native payload and both settings test executables were
compiled again. The stock/AIO integration and isolated MMC/package checks
passed. The private ZIP contains only the plugin DLL, README and feature list;
no supplied mod assets are included. Hashes and exact checks are recorded in
Build/AIOBroadcastCompat-20261105/final-verification.json.
