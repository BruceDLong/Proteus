# Growing RHS prefixes — 2026-09-24

Strict ordered correspondence can now use the completed prefix of a RHS while
its parser is still accepting input. Graph access advertises these relationships
through the existing work-list/registry edges and list-correspondence goal. The
generic search runtime and finite span planner are unchanged.

A prefix walk stops at the first parser-owned unfinished member. It describes
an unknown remainder even when the parser has not yet set `tailUnfinished`.
The alignment Set may copy completed members into independent LHS views, but
withholds all fills, closure, and size writes to the parser-owned RHS. The
incoming list's final size is not used until its parse completes. Existing
size declarations on the LHS still constrain its own shape. Both wholly unknown
and explicitly list-typed unknown targets can adopt the list interpretation.
Completed embedded spans flatten; completed nested lists occupy one position.
Sparse/open remainders remain compressed, and creation plans retain their cost.

Early results remain conditional. The semantic adapter keeps weak witnesses of
the ordered RHS body on dependent results, propagates those witnesses through
copying and correspondence, and marks them unresolved while input is pending.
If later input changes that interpretation (for example a comma or postfix
function), a dependent search fails explicitly. This slice does not roll back
partial Sets or replan an unordered/function interpretation. Failed results and
their original constraints remain inspectable. Parse failures and later
conflicting RHS constraints also prevent successful completion.

Parser publications wake subscribed searches. Cancellation stops further Sets.
The streaming harness reads in-progress values under the same graph lock and
marks input pending before starting its producer, so its assertions do not race
with normalization or mistake an unstarted producer for EOF.

Validation:

- CodeDog regeneration and C++ builds passed for `Proteus.Lib.dog` and
  `streamReplyTest.dog`.
- `unit/graphStream/`: **19/19 passed**, covering typed/unknown/existing targets,
  read-only parser tails, cancellation, route limits, embedded/nested prefixes,
  billion-item sparse tails, reinterpretation, conflicts, unsupported loose
  correspondence, parse failure, late constraints, transitive and registry
  routes, member subscriptions, and a fresh handle after cancellation.
- Existing graph semantic tests: **155/155 passed**.
- Public entry and lifecycle tests: **16/16 passed**.
- Real streaming cases `1, 2, 4, 5, 6, 8`: **33/33 checks passed**. Case 6 failed
  all three original checks before this change; it now passes those three and
  an additional assertion distinguishing list size from member normalization.

The list `{1 2 3}` finishes with exactly three independent members and
`infSize = 3`. Member sizes do not enter that list count. Bare numeric members
retain their unknown individual sizes in the current foundation, so that
particular whole-list result remains symbolic with `isNormed = false`. The
explicit-size numeric fixtures also verify fully concrete completion.

No full legacy suite or ThreadSanitizer run was performed. Loose correspondence,
streaming templates/intersections, and definition/path semantics remain outside
this slice. The generic runtime and span planner were not changed.
