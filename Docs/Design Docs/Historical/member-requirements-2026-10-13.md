# Ancestor member requirements — 2026-10-13

This cycle extends member correspondence to containing definitions and ancestor listSpecs. It reuses ordinary field/span matching and constraint merging. It does not add implicit temporal preservation or prove that Slipstream renders state 2.

## Rule and representative traces

A member first uses its own relationships. If more facts are needed, its retained parent cursor can request the corresponding ancestor requirements. The scope matters: a listSpec applies to one member of its owning list, and the descendant must be located inside that member's corresponding template.

| Request | Retained route and result |
| --- | --- |
| `shape:{location:{z:_},unused:0}.location.z`, with `#shape={location:{z:3},unused:(1=2)}` | Unknown z → containing shape definition → location correspondence → z correspondence → ordinary scalar merge gives 3. The unrelated unused value is not a goal. |
| `{{location:{z:3}}\|{location:{z:_}}}.first.location.z` | Unknown z → enclosing member's listSpec → location then z correspondence → 3. The listSpec remains stored and unchanged. |
| Same template with a demanded z:4 | The applicable template contributes z:3; ordinary merge rejects. No staged write is published. |
| An ancestor template supplying only z:_ | Exhausted source returns partial evidence; the caller continues its remaining sources. No fabricated value, repeated template discovery or subscription. |
| `box:{x:1,seen:_}.seen`, with `#box=box:{x,seen=%.x}` | The borrowed definition retains its instance; `%` reads that represented box's x. Another box with x:2 returns 2. |
| A pending template body or demanded leaf | Wait at that exact input. Unrelated input creates no frames or correspondence positions. Completion continues the retained route; cancellation publishes nothing. |
| A whole list of such members | Existing shared template propagation supplies each member. The test makes no member-to-parent lookups. |

An unspecified corresponding container supplies no descendant facts. The cube probe exposed a regression where projecting through a bare `parts` field incorrectly demanded a list layout. A reduced synthetic model with a partially known color record reproduces it; correspondence now exhausts that uninformative route and allows the other evidence to supply the known coordinate.

A borrowed constraint retains navigation context while delimiting its own ancestor-source discovery. This prevents its unknown field from rediscovering the original caller's constraint. Self binds to the represented object instead of the surrounding selection expression. Named layout borrowing retains the definition marker needed by that same self provider.

The default demand policy is unchanged: obtaining one leaf is not a proof that every unrelated ancestor field is coherent. Whole-object requests have their own constraints. Named definitions may use record field identity even when their current representation is ordered and omits another field; ambiguous identities and unresolved member layouts still require proof.

## Work measurements

The fresh behavioral suite measures requests through Agent/WorldManager with synthetic inputs. Depth fixtures construct small parsed wrappers directly, isolating solver growth from the previously observed deep-text parser cost.

| Shape | Size | Frames | Other measured work |
| --- | ---: | ---: | --- |
| Sibling width | 8 / 32 / 128 | 5 / 5 / 5 | 28 / 76 / 268 correspondence positions |
| Ancestor depth | 8 / 32 / 128 | 10 / 34 / 130 | 23 / 95 / 383 ancestor steps; 32 / 128 / 512 positions |
| Whole-list members | 8 / 32 / 128 | 53 / 197 / 773 | 35 / 131 / 515 relationships; zero parent lookups |

These establish linear growth for the tested unambiguous layouts. They do not establish a general bound for ambiguous matching or many independent leaf requests. A whole-list caller still receives the shared traversal. Completed whole-list requests create no new goals.

## Historical reference

The user requested main-branch precedence over conflicting recent suggestions. A clean archive of `/home/bruce/devl/Proteus`'s literal `main` at `86321ea75fbdbfa3cfd8cf356eb3eb2495b4efc8` was built in `/tmp/p4-main-reference`. Its T-list scalar and compound controls retain unknowns and allow differing successive values. `%?` prints as two items. A member-definition query exceeded its three-second test limit and establishes no semantic result.

Therefore this cycle does not implement D062's proposed implicit preservation/independence rules. The historical implementation intended for that later comparison is awaiting clarification. Explicit identities and previously tested temporal templates retain their existing semantics.

Evidence: `/tmp/p4-member-cycle/main-semantics.tst`, `main-results-final.log`, and `/tmp/p4-main-reference-build.log`. No historical source files were changed.

## Remaining boundaries

A template whose RHS expression must first establish its list layout still reports that missing proof. In particular a descendant reached through an ancestor `_=%tprev` template is not solved merely by enabling literal template projection. Sparse-span ancestor templates and definition-instance writes remain separate cases. The real cube's named layouts, face queries and any intended implicit temporal behavior must still be tested before moving Slipstream onto this engine.

## Cube probe after regeneration

The same diagnostic snapshots as D061 were used, through the regenerated shared library. These remain integration probes, not permanent test inputs.

| Request | Outcome | Frames / relationships | Result or remaining boundary |
| --- | --- | --- | --- |
| Whole two-state expression | Partial | 2051 / 1515 | State 2's written zLoc remains unknown |
| State 2 directly | Partial | 1451 / 1118 | Record meaning still has unresolved relationships |
| State 2's written zLoc directly | Partial | 7 / 6 | Corresponding record field is not established |
| First solved center's zLoc | Complete | 58 / 21 | zLoc:1.1 |
| First solved center's front-color hue | Unsupported | 55 / 19 | Correspondence needs a list layout |
| Two-state expression followed by `@[2].thing.location.zLoc` | Unsupported | 3 / 0 | Named list layout needs template correspondence |

The known coordinate remains correct; its demand now inspects more ancestor evidence than D061's 53-frame request. The whole expression and direct-state counts are unchanged. This cycle supplies a missing reusable capability but does not yet complete the real cube or infer its second state. No Slipstream build, host-interface migration, or visible GUI verification is claimed. Logs: `cube-final-*.log` and `cube-final-summary.json` in the temporary evidence directory.

## Validation

Both maintained CodeDog targets regenerate successfully: library 139.12 s and executable 199.16 s. The independent regenerated-library client passes 470/470 new checks. The complete retained executable reports **24 failures / 534 registrations**, preserving every prior failure name and diagnostic. All 56 WorldManager suites pass. The failures remain sixteen defaults and eight historical compiled cases.

All fifteen protected `.tst` inputs and unrelated `myTests.tst` are unchanged. No Slipstream source or live resource was modified. The maintained changes are CodeDog, tests, and design/evidence documents in `/home/bruce/devl/Proteus4-redesign`.

Final evidence in `/tmp/p4-member-cycle`: `library-build-final2.log`, `test-build-final2.log`, `member-final2.log`, `full-final.log`, `validation.json`, and `cube-final-summary.json`. Full-run log SHA-256: `cb28463f1c8d41787498d952048854bc81f0620b310d04f606de706f61bc9765`. Temporary generated-C++ prototypes remain diagnostic artifacts; they are not the maintained implementation.
