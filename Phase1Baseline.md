# WorldManager Phase 1 Baseline

Captured on 2026-08-20 from the ordinary unprotected LocalBuild test run:

```text
cd LocalBuild
./TestProteus
```

The result remained at the established baseline of 5 failures out of 250 tests:

- `concat/parseSrc`
- `innr/oneOf`
- `this/lvl1`
- `search/adj1`
- `sparse/pending/writeSpanAcrossSparseConcreteSparse`

The run produced:

- 12,612 AItem execution traces
- 276 normalization coverage reports
- 1,162 executions ending rejected
- 10,823 executions that enqueued AItem work
- 4,118 executions with an observable LHS change
- 558 executions with an observable RHS change
- 4,600 executions with a step change
- 7,298 executions with a pending-task change
- 0 unclassified executions
- 0 reachable generated `ACTION`/unsupported handlers

The conservative invariant checks found 28 loose-string merge executions whose RHS carried an invalid, apparently uninitialized `orderMode` both before and after the execution. Phase 1 records this as `invalidOrderMode`; it does not change the legacy behavior or include the unstable raw value in semantic case keys.

The complete filtered trace and coverage stream is stored in `Phase1Baseline.log.gz`. Its uncompressed SHA-256 is:

```text
c8cd81b1a25a9dd3948e0e3a4df3cf8e232c864d8fb2be0b02f31e7baf4b52fb
```

Inspect it with:

```text
gzip -dc Phase1Baseline.log.gz
```

The generated active rule file was deterministic across repeated generation. `WorldManagerRules.dog` had SHA-256:

```text
80b1688fe41f7fdceec0b9a24ef4d7e9dc892aa02a8b562ee967f9170c34980e
```

Generator coverage remained:

```text
mergeSize: 1 / 32 cases handled
merge:     499 / 800 cases handled
startProp: 16 / 16 cases handled
```
