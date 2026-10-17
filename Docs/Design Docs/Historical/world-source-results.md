# World source paths and acquired type labels

`%W` and `%ctx` use one environment-source route in PathSelection. Agent supplies
its current world to GraphNormalizer at normalization entry. Standalone clients
can publish or replace it with `setWorld()`. A replacement retains the old source
for existing query evidence; completed queries reconsider on explicit normalization.
Definitions encountered in field traversal publish their meaning and are not
ordinary value candidates. Searching definitions themselves is a future capability,
as clarified by the user; declarations are not being removed from the graph.

An unknown `?` result inherits a known source type label when acquiring its
interpretation. A numeric `_` retains its numeric interpretation; an already named
target retains its name. This matches the checked Proteus3 distinctions and fixes
`query/firstType`. Source nodes stay intact, and repeated normalization adds no
redundant work. The same acquired type-label fix also recovers `r1/extent/exactSparseUnknownLast`.

Validation: source build succeeded; graphWorld 10/10, graphTypeCopy 6/6, and all
1,087 prior focused checks pass (1,103 total). All six old suites were rerun:
246 pass, 112 fail, five explicit ambiguity skips. Legacy helper checks remain
102/125. No prior passes were lost and no processes crashed or timed out.
No fixture expectations changed. Logs: `/tmp/p4-test-migration/` and
`/tmp/p4-compat-audit-*`.

The fork lacked the original checkout's ignored `Resources/public` link. Restored
that local dependency to `../../World/public`; the build copies it into assets.
No public model files were changed. The two world-loader fixtures now reach their
real semantic/lifecycle blockers instead of failing to open assets. They still
fail and are not classified as ambiguity skips.

The ambiguity manifest records one unresolved span placement and four user-world
lookups past a known untyped value. Its fixture hashes prevent future changes from
silently inheriting an old exemption. See [test-ambiguities.md](../../test-ambiguities.md).
