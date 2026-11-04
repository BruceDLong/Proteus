# Infon theory applet — occurrence counters (experimental)

The existing parent/coset views are preserved. When studying the **Quotient** component with **Phase = 0** and nontrivial ordered factor sizes, choose the **Counters** stage to inspect a constructive occurrence-ledger model.

## How to investigate

1. Choose parent size and value, and a nontrivial factor in the **Divisor** selector.
2. Choose **Quotient** and **Phase = 0**.
3. Select the **Counters** stage. The original parent baseline remains visible.
4. Use **Next +N** to advance one individual singlet occurrence, or **Next boundary** to jump to the next rational threshold. **Previous +N** and **Previous boundary** rewind the display using recorded frames.
5. Each singlet card displays its permanent root index `r`, its occurrence counter `c`, and its current unwrapped position `r + c × parent.size`. The main transport Play/Step controls advance the same counter frames.
6. Choose **Result** to see the completed history and the resulting component baseline. An independent canonical-factor check is displayed separately.

For example, `*10+5` with factors 5 and 2 crosses threshold `1/2`. The selected roots `[0,5,1,6,2]` finish with unwrapped occurrences `[0,6,15,22,31]`, yielding baseline `[0,6,5,2,1]`. The root indices never change; only their occurrence counters advance.

## Mathematical scope

The implementation in `src/occurrenceLedger.ts` is based on the grouped-threshold experiment:

* Select the first `a` singlets of a parent `*N+v`, where `N=a*b`.
* Examine reduced rational boundaries `k/q` in `(floor(v/b)/a, v/N]`, from high to low, using **exact integer comparisons**.
* At each boundary, participants with identical traversal indices modulo `q` meet at a phase. Nonzero-phase tie groups reverse their local precedence; the nonzero multiples of `q` pass the zero cut.
* Satisfy the resulting precedence constraints by advancing individual singlets from position `r+c*N` to `r+(c+1)*N` until the new order is consistent. No identity is changed and no occurrence counter decreases.
* Compare the final constructed component baseline against the independent canonical-factor reconstruction **after** processing every event, as a regression oracle only.

The exact occurrence coordinates use a **history-preserving convention**. We do not yet know whether this convention follows uniquely from infon identity axioms; another minimal-final-lift policy can produce the same ordering but different coordinates. Likewise, the block-step rotation premise is not a proven multiplication theorem.

**Other phases, the strided factor, and additive decomposition** still use earlier reconstruction logic and are explicitly labeled as such in the interface. The new experiment does not silently extend its proof obligations to those cases.

## Tests

With Node 22+ from `theory/applet`:

```sh
npm run test:occurrences
```

The test enumerates every nontrivial ordered phase-0 factorization of parents sized 4–120, all values (25,716 cases), checking resulting root identities, strict occurrence ordering, all increments as multiples of the parent size, and agreement with the independent canonical-factor oracle.

The normal applet build uses:

```sh
npm ci
npm run build
```

These UI build commands require installed npm dependencies. Passing the exhaustive arithmetic test alone does not establish that the full React app renders correctly, nor does it settle the underlying theoretical assumptions.
