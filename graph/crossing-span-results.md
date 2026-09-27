# Crossing open spans

After common prefix and suffix correspondences, a crossing equation has the form
`P + gapA = gapB + S`, where `P` and `S` are finite written regions. When they
cannot overlap, one shared unknown middle describes both gaps:

```text
gapA = middle + S
gapB = P + middle
```

The existing opaque-node span planner proposes that shape. The correspondence
goal proves that every shorter overlap of the written ends contains a definite
mismatch, using ordinary scalar/header facts. Unknown comparisons do not prove
mismatch. A known extent can instead establish the middle's nonnegative size
arithmetically. No candidate gap lengths or combinations of unknown members are
enumerated.

The Set creates one shared middle and independent identity-backed member views.
Both outer gaps can close while their common middle remains open. Ordinary graph
normalization follows the resulting identities; later input can fill the middle
from either side, and the original equality stays searchable. Cost includes the
new members and identities before publication. Large known middles stay compressed.
The printer preserves known bounds on open literal lists, including those with
no repetition template, so printing and reparsing does not discard their size.

Proteus3's parent-member continuation similarly preserves unknown material with
`addNewSubDots`/`keepDots` and `insertChildDotsBefore`. The graph version expresses
that unresolved region explicitly and publishes it only after an overlap proof.
It does not reproduce the legacy agenda's tentative-member lifecycle.

## Boundaries

Ambiguous shorter overlaps remain symbolic. For example, the old `align4b`
fixture has a compatible boundary `4`; it does not justify choosing a particular
middle or duplicating that value. Contextual/composite comparisons that cannot
prove a mismatch also defer. Parser-owned tails and repetition-template gaps
retain their existing deferred behavior. Definition application explicitly
defers these structural plans instead of treating them as finite fills.

The original `align3` and `align4` examples now normalize to the same symbolic
structures as their expected results. Their exact assertions still fail because
of the legacy `&{ ... }` spelling. Five focused language checks verify the
canonical results and equivalent parsing; original fixture text is unchanged.
`align4b` remains a semantic deferral, not a spelling-only difference.

## Validation

- Regenerated with `codeDog Proteus.Lib.dog`; `Marker: Build Successful`.
- 934/934 focused checks, including 26 parent-projection cases, 13 crossing-span
  cases, five canonical language checks, and all 890 previous focused checks.
- Shared span planner: 374/374 cases passed.
- Unchanged original defaults: 192/270 passing, 78 exact assertion failures.
  Every fixture retained its prior pass/fail status; no crashes or timeouts.

Logs: `/tmp/p4-field-cross-final-build.log`,
`/tmp/p4-field-cross-final-focused.log`, and
`/tmp/p4-field-cross-final-default/results.json`.
No sanitizer, GUI integration, or full compiled legacy-unit pass is claimed.
