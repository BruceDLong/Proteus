# Field source prerequisites and written type metadata

The field walk previously stopped at a bare source without exposing the goal
that could supply its structure. It also stopped at every non-self source RHS,
even one already represented in the source. Source readiness now uses the
existing RHS representation proof and exposes an exploratory size-and-value
prerequisite when necessary. The graph can select a useful field without waiting
for unrelated unknown fields. The shared walk covers direct lookup, chained
lookup, and choices; no recursive normalization call is added.

Compiled definition requirements distinguish their normalized nominal key from
the original WordUse. New members copy the latter, preserving spelling and model
metadata without sharing mutable declaration words. This fixes the original
`query/getNthSub2` output (`CC:'ccc'`) without changing its expectation.

The 13 new focused cases cover definition/RHS sources, chains, intersection
spans, direct goals, unrelated unknowns, late definitions, cancellation, cost,
independent WordUse objects, and spelling in grouping and grammar output.
All previous 1,115 focused checks pass: **1,128/1,128**. Full source regeneration
and build succeeds. The six old language suites report **252 pass, 104 fail,
7 explicit ambiguity skips**; default tests are **236 pass, 27 fail, 7 skips**.
No previous passes were lost. Legacy helper checks remain **102/125**, with no
crashes, timeouts, or incomplete summaries.

The additional ambiguity concerns when absent T-list context proves permanent
`UNDEFINED`. `{1 %tprev 3}` must not fall back to its S-list predecessor, but
another identity may establish temporal meaning later. Its input/expectation
are unchanged and the exact fixture is recorded in test-ambiguities.md.
Clear temporal navigation requirements remain failures until implemented.

Logs: `/tmp/p4-test-migration/field-source-final-*` and
`/tmp/p4-compat-audit-*`. No old fixture expectations changed in this checkpoint.
