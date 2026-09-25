# Original-test recovery: represented RHS information

The first blockers in the selected original normalization fixtures were not
missing span mapping. The engine already acquired the expected values and list
shapes, but bare numeric literals retained unknown sizes. This correctly kept
`isNormed` false, which made the printer retain repeated RHS text.

A separate, costed representation proof now allows `_ = 2` to print `2` while
remaining symbolic and subscribed. It also checks closed ordered list layouts,
including nested lists and finite embedded spans, before suppressing a parent
RHS. It neither changes the requested facts nor removes the original relation.
A later publication invalidates the proof and can still supply size information
or reveal a contradiction through the retained edge. Unproved RHSs stay visible.

The generic graph scheduler, span-alignment algorithm, original language inputs,
and original expected strings are unchanged. The change does not implement
open-prefix/template mapping, loose correspondence, string segmentation,
contextual references, or positional language selectors.

## Validation

- Full `codeDog Proteus.Lib.dog` and `codeDog streamReplyTest.dog` source
  generation and C++ builds passed.
- `python3 graph/run_language_baseline.py`: **38/38**, previously **27/38**.
  The 11 recovered originals are `merge/num/literalEqual`,
  `merge/num/literalToUnknown`, `merge/num/unknownFromLiteral`, `merge/list/nums`,
  `merge/list/multiID`, `merge/list/N_dots3`, `merge/list/dots`,
  `trav/list/withEmpty`, `noRHS/dots/nestedOut`, `noRHS/dots/nestedIn`, and
  `parse/innerID`. These are output matches; some results intentionally remain
  symbolic because their numeric sizes or open tails are unknown.
- Additional unchanged originals `merge/list/N_dots1`, `merge/list/N_dots2`,
  and `word/types` now pass through the ordinary test path. `word/deref` still
  passes. The two N_dots cases keep the original embedded-span rendering.
- `unit/graphRepresentation/`: **18/18**. Covers partial scalar/list results,
  nested and embedded members, unknowns, conflicts, loose/contextual work,
  open tails, cost exclusion, late size/conflict publication, cancellation,
  shared infon storage after handle release, unresolved types, extent mismatch,
  repeated runs, and late conflicts in a represented list member.
- Existing focused suites: graph foundation **155/155**, entry/lifecycle
  **16/16**, growing streams **19/19**, scalar definitions **36/36**, finite
  definitions **41/41**, lazy templates **36/36**, parser **53/53**. Including
  the new proof tests, **374/374** focused checks pass.
- Standalone streaming cases `1, 2, 4, 5, 6, 8`: **33/33 checks passed**.
- `git diff --check` passed.

## Next original-test milestones

These cases were rerun and still fail; this is a targeted sample, not a census
of the full language suite.

| Original fixture | Next relevant capability |
|---|---|
| `parse/grouping1` | Repeated member span mapping plus loose list correspondence |
| `parse/catHatDogPig` | Text spans and loose string/list correspondence |
| `parse/reps1`, `merge/lookahead` | Variable repetition and lookahead, beyond uniquely anchored spans |
| `parse/lang` | Text matching, alternatives, and repeated grammar definitions |
| `query/getNthSub`, `query/chain` | Language selector/path migration |
| `prev/plus1` | Contextual member references and arithmetic |

For `parse/grouping1`, the parser marks the RHS POV as `looseMode` for `==`.
`ProteusGraphAccess.plain()` excludes loose relationships, so `relationSupported()`
does not expose that RHS to the correspondence walk. That gate must be replaced
with a real loose-correspondence route, not simply removed: one repeated pair
must correspond to two source positions, unlike strict one-member identities.

The next mapping milestone should use `parse/grouping1` as a visible language
target, with its mapping and loose-correspondence prerequisites tracked
separately. Shared definitions must stay read-only; already-demanded occurrence
POVs must retain their identity; the unrequested tail must stay compressed.

The full legacy suite and sanitizers were not run for this change. All work is
local to the private Proteus4 fork; the original Proteus checkout is untouched.
