# Proteus singlet-addressing experiment, v5

This is a finite, executable **reference model**, not a finished Proteus arithmetic
engine. It extends the proposed coset / phase-zero conventions in
`singlet_maps_v4.py`. Tests check implementation consistency under those
conventions; they do not prove that the conventions follow from the complete
infon theory.

## Discrete summation in Proteus

The working rule for a structural sum uses the first singlet of the immutable
parent as the shared singlet. In the parent's traversal, A takes the prefix
and B takes the first singlet followed by the remaining suffix. The shared
singlet gives the size `sA+sB-1`; the child values must add to the parent's
value. For `A+A`, the remaining singlets are distinct and correspond in order.
Both copies have the same value, including when that value is unknown.
In the Proteus evaluator the shared first singlet justifies the
size formula. The evaluator computes sizes and values directly with GMP or
machine integer arithmetic. Singlet enumeration belongs to the reference
experiment.

The reference script and applet calculate both `B0` (shared first singlet)
and `Bend` (shared last singlet of A). For the parent `*6+3`, this command
shows which singlets and child values each choice yields.

```sh
python infon_additive_prefix_table.py 6 --ghosting when-needed --case 3 4
```

In Proteus, `**(*4+2 +(*3+1))` gives `*4+3`. The signed operand supplies
a contextual displacement. The structural sum `++(*4+2 *3+1)` gives
`*6+3`. Both joins take ordered lists. In `++(&{*4+2 *3+1} *2+1)`,
the embedded list supplies the first two operands and the result is `*7+4`.
Ordinary parentheses group one infon.
[D129 records the language decision](../../../Docs/Design%20Docs/normalization-design-decisions.md#d129-grouping-and-ordered-multiplicative-and-additive-joins).

Under the proposal, the selected view
determines how far a cursor displacement can carry. Unknowns used in range
descriptions constrain the unreduced values.

See the [parser](../../../infonIO.dog#L1484),
[numeric evaluation](../../../WorldManager.dog#L2567),
[range fixtures](../../../Tests/numeric-range-tests.tst), and
[additive decomposition applet](../../applet/src/additiveModel.ts#L275).

## Run

Python 3.10 or later, no third-party packages:

```sh
python singlet_maps_v5.py
python test_singlet_maps_v5.py
```

The second command exits unsuccessfully on any failed test. The captured test
output from this delivery is in `test_results.txt`.

## The input address

An address consists of a particular `Root` object, a decomposition path, a local
singlet index, and its coordinate basis:

```python
from singlet_maps_v5 import Root, Mul, Span, SingletRef, locate, phase0_index

root = Root(12, 5)
ref = SingletRef(path=(Mul(3, 4, "L"),), index=1)
assert phase0_index(root, ref) == 8
```

`Mul(a,b,"L")` means: split **this occurrence** into first size `a` and opposing
size `b`, then choose the first factor's phase-zero section. `"R"` chooses the
opposing factor. Its factors must multiply to the current context size.

For `2_A * (2_B * 2_C)`, the three paths are:

```python
A = (Mul(2, 4, "L"),)
B = (Mul(2, 4, "R"), Mul(2, 2, "L"))
C = (Mul(2, 4, "R"), Mul(2, 2, "R"))
```

These are three distinct addresses even when the factor sizes or selected values
agree. In `Root(8,5)` their two phase-zero baseline members are respectively
`[0,4]`, `[0,2]`, and `[0,5]`.

Reusing the **same Root object** allows comparison between different
decompositions. Constructing two roots with equal size/value does not identify
their singlets.

## Local +y, versus traversal position

The default `basis="baseline"` names singlet `y` in the selected part's LOCAL
baseline. It is not the root-baseline index `y`, and not automatically traversal
position `y`. Naming a singlet does not assert or change a selected value.

`basis="order"` explicitly means a position in that part's state-dependent
coset traversal. In the opposing size-4 factor of `Root(8,3)`:

```python
root = Root(8, 3)
part = (Mul(2, 4, "R"),)
assert phase0_index(root, SingletRef(part, 1, "baseline")) == 1
assert phase0_index(root, SingletRef(part, 1, "order")) == 3
```

The distinction prevents a coordinate lookup from silently treating contextual
`+y` as a global shift. This module does NOT yet parse `*x+y` or implement its
state assertions, repeated addition, overflow, carry or unknown operands.

## Explicit additive spans

`Span(start,size,basis)` specifies a contiguous slice in the parent's named
coordinates. Child local baseline `j` names parent coordinate `start+j`.
The default parent coordinate is `baseline`.

This is a **span view**, not the assertion that `*12+5` itself means a 5+7 split.
It does not give both child spans independently selected values. Span values
remain unspecified; state-dependent traversal/factorization of such a span is
rejected rather than invented. Baseline sub-slices and spans inside valued
product factors are supported.

```python
root = Root(12, 5)
factor = SingletRef((Mul(3, 4, "L"),), 1)
suffix = SingletRef((Span(5, 7),), 3)
a, b = locate(root, factor), locate(root, suffix)
assert a.identity == b.identity
assert (a.index, b.index) == (8, 8)
assert (a.lift, b.lift) == (20, 8)
```

Thus two routes refer to the same singlet without destroying their different
unreduced-position provenance.

## The product / consecutive-span correspondence

The v4 candidate model gives `Root(12,5)` this ordered traversal of root IDs:

```
0, 5, 10, 3, 8, 1, 6, 11, 4, 9, 2, 7
```

Explicitly partition that ORDER into three consecutive spans of length 4:

```
span 0:   0   5  10   3
span 1:   8   1   6  11
span 2:   4   9   2   7
```

Under the candidate product rule, the first size-3 factor's phase-zero members
are the block starts `[0,8,4]`. The opposing size-4 factor's phase-zero members
are the first block `[0,5,10,3]`.

More generally, if P denotes a parent's traversal-to-root map:

```
first_factor.order[i] = P[i*b]
opposing_factor.order[j] = P[j]
block_i.baseline[j] = P[i*b+j]
```

These are identities of root singlets, and the tests compare unreduced lifts as
well. The blocks are consecutive in the specified ORDER; they need not be
consecutive in the numeric root BASELINE. The relationship is a concrete,
testable model of how product sections and additive span views share singlets;
it is not a proof of all meanings of Proteus '+' and '*'.

## Mapping algorithm

For a context of size n and value v, let g=gcd(n,v), m=n/g. The proposed order is

```
(coset, turn) = divmod(position, m)
lift = coset + turn*v
local_baseline_id = lift % n
```

`order_position()` inverts that permutation. For a product split of a context,
child values retain the earlier model's `divmod(parent_value,b)` convention.
Local baseline references are first converted into the child's order. The left
factor maps order i into parent order i*b; the right maps order j into parent
order j. A span maps its local baseline into the declared parent coordinate by
its starting offset. Compose these maps while walking the path backwards, then
apply the root traversal once, or use a canonical root-baseline representative
if the path ends in root-baseline coordinates.

The algorithm does **not** materialize n singlets. It stores a frame per path
step; integer arithmetic cost still depends on operand bit lengths. A test
looks up a member in a root with 2**256 states without enumeration.

## Ghosts and identity

`locate()` returns an `Occurrence`:

- `index`: root-baseline representative, `lift % root.size`.
- `lift`: unreduced root position retained along this lookup route.
- `identity`: `(particular root, normalized index)` packaged as `SingletId`.
- `winding`: quotient of the lift by root size; this is NOT a factor phase.
- `is_ghost`: whether the retained lift is outside the root baseline interval.

Compare `.identity` for singlet identity. Compare whole occurrences only when
the lift/provenance should agree too. A direct baseline reference can use a
non-ghost representative of a singlet reached as a ghost through another path.
Child factors never change the ROOT modulus used for final ghost identity.
Local coordinate conversions do use the child's local size, as they must.

Here 'phase zero' is the candidate convention of fixing all opposing product
coordinates at zero. The function does not claim to find a chronological first
occurrence across unspecified extra phase cycles.

## Inverse lookup

```python
from singlet_maps_v5 import local_index_of
root = Root(12, 5)
path = (Mul(3, 4, "L"),)
found = locate(root, SingletRef(path, 1))
assert local_index_of(root, path, found.identity) == 1
assert local_index_of(root, (Span(5, 7),), found.identity) == 3
```

An absent singlet, or a singlet from a different root, returns `None`.
This is inverse addressing, not division or inverse arithmetic inference.

## Test coverage and boundaries

The tests use an independent, explicitly enumerated cycle-walking/list-slicing
implementation of the same candidate model. They cover all canonical values and
all ordered binary factorizations for root sizes 1 through 64; selected nested
factorizations and reassociations; non-coprime and zero values; baseline and
traversal span coordinates; nested spans; spans inside factor occurrences;
identity across alternate paths; ghost lifts; inverse membership; invalid
addresses; and a huge root that cannot be enumerated.

Captured run: 10 test groups passed, with 1,108,446 oracle lookups, 1,108,446
inverse round trips, 25,424 membership checks, 49,831 block-identity checks and
39,264 reassociation checks. These are finite tests of the specified model, not
an exhaustive test of all finite infons.

Still outside the experiment: deriving the coset/projection conventions from
the theory, validating full selected-state recomposition, nonzero product
phases, arithmetic composition/inversion, carries, unknown sizes/densities,
and iteration inference. In particular, phase-zero sections alone do not
supply a general Cartesian-product reconstruction rule. Do not flatten child
selected values into traversal positions as though those coordinates were the
same.

Host integer operations, including divmod, remain in the reference model for
coordinate calculation and inherited factor-value metadata. This is not yet
an arithmetic-free singlet kernel. That boundary is explicit rather than hidden
inside the name 'infonic arithmetic'.
