# Proteus Infon Decomposer

The app has Multiplicative and Additive views. Within Additive, **Current method**
keeps the deployed prefix-extraction model, and **Signed construction** builds a
pair of operations on a fixed root carrier. Switch methods while retaining the
parent size, parent value, and A size. The two methods can assign different real
states even when their numeric labels agree.

## Compare the methods

1. Select **Additive** and set parent size 7, parent value 5, and A size 3.
2. In **Current method**, A is `*3+2`; either overlap option gives B `*5+3`.
3. Select **Signed construction**. A value can be 1 or 2, giving respectively
   `*3+1 + *5+4` or `*3+2 + *5+3`.
4. Choose A value 1 and the **Return paths** stage. A's return from root 0 uses
   occurrence positions `0, -1, -2, -3, 1`, with increments `-1, -1, -1, +4`.
   Its net displacement is +1. Root identities stay attached to their positions
   on the repeated parent baseline.
5. Inspect **Composition** and **Checks** to see the actual per-state results.

The four signed stages show real-state membership, complete first-return paths,
A-then-B composition, and the calculated conservation checks. Coset boundaries
are explicit, including singleton cosets for value zero. Intermediate signed
steps and complete return displacement are displayed separately.

## Meaning of the signed construction

For a parent `*N+v`, choose A size `a`, set `b=N+1-a`, and choose A value `x` in

```
max(0, v-b+1) <= x <= min(a-1, v).
```

B value is `y=v-x`. The supports are A = `{0,...,a-1}` and
B = `{0,a,...,N-1}`. Only root 0 is shared. These ranges enumerate every ordered
numeric split with `a,b >= 1`, `0 <= x < a`, and `0 <= y < b`; the total number
for the parent is `(v+1)(N-v)`, including `*1+0` children where allowed.

An operation is stored as a periodic permutation window: `F(i)` is an integer
occurrence position, and `F(i+kN)=F(i)+kN`. Reducing it modulo N gives the root
successor; keeping the integer retains signed movement and lap information.

The direct construction starts with B's canonical forward lift `Q0`, fixing
A-exclusive roots. With `T(z)=z+v`, form `P0=Q0^-1 T`. Let `alpha0` be its
first-return map on A and `alpha` the desired A rotation with its exact forward
return displacement. The correction `K=alpha alpha0^-1` acts on A and fixes
the other roots. The result is

```
FA = K P0
FB = Q0 K^-1
FB FA = T.
```

For these contiguous supports, K preserves each finite A block. This is the
property used by the signed existence proof to preserve B's returns and prevent
displacement on ghost-only cycles. The implementation uses linear passes and
disjoint return routes, so constructing one certificate takes O(N) time and
space; it does not search over permutations.

The displayed checks are computed from the resulting windows: full-carrier
permutations, support coverage and overlap, exact local successors and net
displacements at every real return, zero displacement on ghost-only cycles,
signed displacement totals `Nx` and `Ny`, and `FB(FA(i))=i+v` at every root.

The complete argument is in [SIGNED_ADDITION.md](SIGNED_ADDITION.md).

This chooses one completion for each numeric split on the stated supports. It
does not enumerate all interleaves, prove arbitrary completions equivalent, or
claim that reconstructing children from their numeric labels preserves a prior
context. The children factor the parent operation; they need not be subsequences
of the parent's traversal. This applet does not change the Proteus engine.

## Development and validation

```
npm ci
npm test
npm run build
npm run dev
```

Tests use Node's test runner and the existing TypeScript compiler; no additional
test dependency is needed. The model tests verify the finished windows with
independent literal return walks, including singleton children, zero values,
negative intermediate steps, and all 40,920 canonical pairs through parent size
30. A focused callback test checks switching methods, shared inputs, overlap
selection, playback cleanup, stage changes, and clamping to a singleton parent.
The app's interactive size limit remains 24. Nineteen additional static React
render checks covered all signed stages and a failed certificate; browser paint
and responsive screenshots were unavailable in the validation environment.

## Server baseline

The server snapshot from 2026-11-26 was the authority for Current method. Its
JavaScript asset was `index-IYaEMYFn.js` (SHA-256
`c060205ce0c7e83a8929f1e67cb07a8f0e8f2c01512ede9c28f7a01f43d5972e`), and its
CSS asset was `index-D2nYh880.css` (SHA-256
`a5dd9d7fd0e616b7dba2272a273ef604a6fd573be2b95685afd02da47067d52b`).

Before making changes, its extracted model was compared against repository
commit `730c9b321f388e53faaf2806c34881dbb0c6d992`: all 4,900 additive models
and 16,296 multiplicative analyses in the UI domain matched. There were 276
identical controlled static React renders, and all 637 ordered CSS declarations
matched after explicit semantics-preserving minifier normalization. This is
snapshot evidence; it does not assert byte-identical builds, event-handler
equivalence, or anything about later server changes.

## Deployment

`npm run build` produces `dist/index.html` and `dist/assets/` with relative asset
URLs suitable for the existing `/infonDecomposer/` directory. Install both from
the same build, publishing assets before the new HTML and retaining the previous
HTML/assets for rollback.

The repository's `build.sh --upload` currently has its transfer commands
commented out. It does not deploy a build. Use the established server connection
and the confirmed app directory when publishing; no server path is assumed here.
