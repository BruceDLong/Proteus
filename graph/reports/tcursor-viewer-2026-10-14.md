# TCursor and Slipstream materialization — 2026-10-14

The viewer uses the replacement WorldManager and TCursor. A scene materializes the parts, poses and colors it reads into its own occurrences. The temporal source model and definitions remain unchanged. The renderer does not demand unrelated face-slot metadata.

## Causes and shared fixes

1. Acquiring an inherited record layout evaluated unrelated composite descendants and could reject before the required fields became available. Layout acquisition now borrows those descriptions. Actual field constraints still evaluate deferred inputs through ordinary frames; a demanded contradiction rejects without publication.
2. Deferred fields lost their enclosing definition context, or read the original partial instance after other fields had been materialized. They retain provenance and bind a borrowed parent wrapper when demanded. Two instances of the same definition now return their own supplied colors and inherited defaults.
3. Supplied-work mode suppressed an explicit selector's correspondence input along with unsolicited ancestor discovery. The input is now consumed through the existing parent cursor. An unknown named value can still acquire its own definition when ancestor discovery is disabled.
4. Repeated ordered types were treated as record keys. Field merging requires a unique RHS layout; duplicate fields in a record instance still fail. An unchanged deferred payload does not need another retention merge.

There is no cube-name branch in WorldManager, second solver, goal restart, or change to `:` semantics. Scene requests use the ordinary materialize/solve path and retain their handles through waits.

## Synthetic coverage and work

`unit/worldManager/tCursor` now has 243 checks, up from 173. New fixtures use small synthetic definitions, not mutable user/world files. They check two differently colored instances, inherited defaults, ordered repeated member types, deferred contradictory metadata, explicit metadata rejection, inherited composite constraints, source preservation, stable cursor storage and state 2 retaining 3.5.

With 8, 32 and 128 unused contradictory metadata fields, initialization uses **11 frames and 9 relationships** in all three cases. Existing temporal-chain and wide-record bounds remain covered. This proves that undemanded metadata width does not add solving work in this family; it is not a universal complexity claim for arbitrary inference or independent field queries.

## Validation

The maintained shared library and Slipstream both regenerate successfully from CodeDog. A client linked to that final shared library passes **243/243 cursor checks**. All 15 protected language fixtures match HEAD, and both live Slipstream model files match their pre-task hashes.

A private Xvfb run opens the real board, terminal and colored cube. Real button clicks request index 1 from index 0 and then index 0 from index 1; debugger return checks show `TCursor::goNext() = true` and `TCursor::goPrev() = true`. Screenshots confirm the colored cube in state 2 and after returning. No `DISPLAY` or `TCURSOR` errors occur. The existing preview animation explains differing visible faces between captures. The bounded run stays alive and its task-owned processes are then stopped.

Evidence is in `/tmp/p4-cursor-viewer/`: `official-cursor.log`, `navigation.log`, `state1.png`, `state2.png` and `previous.png`. The regenerated executable passes all **57 WorldManager suites** and reports **24 failures / 535 registrations**, with exactly the same failing names as the prior baseline (16 default fixtures and 8 historical compiled tests). The full log SHA-256 is `9551b5a336ca8b2d5c09e024ddf682aa6f87a765601dcedbc456e7cdc9664991`; machine-readable results are in `/tmp/p4-cursor-viewer/validation.json`. Final CodeDog build times were 172.23 s for the shared library, 125.90 s for Slipstream, and 220.50 s for TestProteus. `git diff --check` and manifest JSON validation pass.

## Remaining scope

Ancestor temporal listSpec transitions, general inverse transitions, and broader sparse alignment remain deferred. The live world loader still reports its existing unsupported scalar/world and open-source span-alignment requests. Those are distinct from the successful cube display path. Model files were not modified to make rendering pass.
