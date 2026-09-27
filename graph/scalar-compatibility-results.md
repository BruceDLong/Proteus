# Scalar compatibility and Agent context lookup

Strict scalar constraints now share a three-way decision: compatible,
contradictory, or unresolved. Positive constraints combine their available facts.
A negative constraint complements only a proved match/mismatch; missing value,
size, or nominal evidence stays unresolved. Equivalent negative constraints are
redundant, while different negative constraints remain conjoined.

`ScalarPredicate.h` owns those decisions. `ScalarRelation.h` uses the existing
scalar goal to record exclusion evidence without copying positive values or
sizes into a negative template. Field predicates reuse the complement rule.
Ordinary scalar acquisition uses the same compatibility helper. List
correspondence checks explicit member contradictions before publishing member
identities or fills. No changes were made to the generic search scheduler.

Rejected strict constraints fail their required goal and retain both endpoints
and the RHS. Successful exclusion records rendering evidence but never equates
the two nodes. Source facts remain available to independent searches.

## Existing test updates

Inputs and modes are unchanged in these six default fixtures; their expected
text now retains the rejected RHS:

- `merge/directAtomic/numInvertEqualReject`
- `merge/directAtomic/numInvertWildcardReject`
- `merge/directAtomic/strInvertWildcardReject`
- `merge/directType/numRejectStr`
- `merge/directType/strRejectNum`
- `merge/directType/listRejectNum`

Two older focused assertions expected unsupported/pending behavior for known
contradictions. `unit/graphFoundation/spans/nestedIsOne` and
`unit/graphFoundation/overlay/knownTypeRetained` now require rejection and check
that the original data is retained and no contradictory identity is created.
The accepted exclusion fixture keeps its original expected result.

## Implicit context

`%ctx` supplies the Agent's existing topic-history POV. Unary `~word` already
parses as `%ctx~word`; it now uses the same field/external-property lookup and
result publisher as explicit-source resolution. The binding is a live graph
edge. There is no copy of the topic-history graph or new context search loop.

Standalone `GraphNormalizer` users can supply a source through `setContext()`.
Agent initialization supplies `AgentContext.topicHistoryPOV`. Pending searches
can resume when a source is supplied or changed. Completed queries reconsider
it on explicit normalization, consistent with the existing lifecycle contract.
A query whose value or size is still unknown remains subscribed and can react
immediately to a published context change. Mutations of a supplied context must
be published under the normal graph runtime update boundary.

Whole-context validation exposed two ordinary membership lifecycle issues.
The shared list counter now counts `mNone` (standalone) infons as ordinary
members, just as it counts parser-marked `mItem` infons. AgentContext membership
edits invalidate a derived count and completion metadata; explicit size
constraints are preserved. The Agent test counts a one-item history, adds a
second topic, then culls back to one, checking the proved size at each step.

## Validation

- Source regeneration and C++ build passed.
- **1,087/1,087** focused graph/parser checks passed, including 20 new scalar
  checks and ten context cases.
- Scalar cases cover both operand polarities, unresolved values, nominal
  identity, delayed acceptance/rejection, cancellation, and retained operands.
- Context cases cover explicit and implicit lookup, whole-context access, late
  sources, cancellation, cost exclusion, unresolved earlier candidates, completed
  query replay, empty results, and the public Agent entry point with addition/culling count invalidation.
- Defaults: **229/270**, up from **222/270**. The seven targeted default
  constraints are now decided; six also have the retention expectations above.
- All six older suites: **244/363**, with no lost passes, crashes, timeouts, or
  incomplete summaries. Legacy helpers remain **102/125**.

The Proteus3 comparison used `/home/bruce/devl/Proteus/LocalBuild/TestProteus`.
It passed the nine original inversion/type fixtures; the tenth selected fixture
was the previously changed numeric-rejection rendering contract. The older
LinuxTestBuild executable is stale and was not used as the semantic baseline.
Logs: `/tmp/p4-scalar-compat/proteus3-reference.log`,
`/tmp/p4-compat-context-final-build.log`, `/tmp/p4-compat-focused-final.log`, and
`/tmp/p4-compat-audit-*`.

## Remaining boundaries

Composite exclusions, general negative-conjunction simplification, and direct
qualified scalar exclusions beyond the existing sequence predicates remain
symbolic. Declared type meaning inside standalone negative constraints still
needs read-only definition evidence; this change does not apply a positive
meaning to a negative object. Contextual definition expansion remains to be
implemented. The binding decision in `open-questions.md` is now settled: an
instance keeps its original application context, including on explicit
normalization; future rebinding will need caller authorization. Overloaded
meanings and the remaining older-suite failures are unchanged.
