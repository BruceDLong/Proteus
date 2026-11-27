# Constructive existence of signed additive factorizations

**Status:** A proof of existence for every finite pair in the stated signed
model. This does not prove that the model is the uniquely intended singlet
semantics, or that independently reconstructed completions are coherent.
No Proteus engine code is changed or executed.

## Theorem

Let

\[
a,b\geq1,\qquad 0\leq x<a,\qquad 0\leq y<b,
\qquad N=a+b-1,\qquad w=x+y.
\]

Use the fixed root carrier \(\mathbb Z/N\mathbb Z\), with real subsets

\[
A=\{0,\ldots,a-1\},\qquad B=\{0,a,\ldots,N-1\}.
\]

There exist periodic bijections \(F_A,F_B:\mathbb Z\to\mathbb Z\), of
period \(N\), such that:

1. Their residue maps are permutations of the same \(N\) root identities.
2. First return to the periodic real set \(A+N\mathbb Z\) is the canonical
   local rotation by \(x\), with each real-to-real route having its minimal
   forward net displacement on the root baseline.
3. First return to \(B+N\mathbb Z\) has the corresponding property for
   local value \(y\).
4. Every residue cycle containing no real state has net displacement zero.
5. \(F_B(F_A(z))=z+w\) for every integer \(z\).

The construction below computes one such pair with \(O(N)\) arithmetic
operations on an explicit carrier. It needs no permutation search or integer
offset search.

The integer coordinate is an occurrence/displacement cover. Treating its lap
coordinate as an independently observable state would require counting that
state in the larger configuration model.

## Construction

Write \(L=a-1\), so \(N=L+b\). Denote the periodic real sets by
\(\mathcal A=A+N\mathbb Z\) and \(\mathcal B=B+N\mathbb Z\).
The actual integer intervals are

\[
A_k=\{kN,\ldots,kN+L\},\qquad
G_k=\{kN+L+1,\ldots,kN+N-1\}.
\]

Thus \(\mathcal A\) is the union of the \(A_k\); the \(G_k\) are its
ghost intervals; and \(\mathcal A\cap\mathcal B=N\mathbb Z\).
An interval \(G_k\) is empty when \(b=1\).

Let \(\beta_y\) move forward by \(y\) places in the increasing periodic
list \(\mathcal B\). Define \(Q_0\) to equal \(\beta_y\) on
\(\mathcal B\), and to fix every integer in
\(\mathcal A\setminus\mathcal B\). This is a periodic bijection. It
already has the exact desired \(B\) behavior and has only fixed ghost
cycles.

Set

\[
P_0=Q_0^{-1}\circ\tau_w,\qquad \tau_w(z)=z+w.
\]

Let \(\alpha_0\) be the exact lifted first-return map of \(P_0\) on
\(\mathcal A\). Let \(\alpha_x\) be the desired canonical lifted
rotation on \(\mathcal A\). For \(s\in A_0\),

\[
\alpha_x(s)
=s+x+(b-1)\left\lfloor\frac{s+x}{a}\right\rfloor.
\]

The correction is

\[
K=\alpha_x\circ\alpha_0^{-1}\quad\text{on }\mathcal A,
\qquad K=\mathrm{id}\quad\text{on its complement}.
\]

Finally define

\[
\boxed{F_A=K\circ P_0,\qquad F_B=Q_0\circ K^{-1}.}
\]

Algebra immediately gives \(F_BF_A=\tau_w\). The important remaining
point is that the correction does not introduce unwanted winding into
\(B\) or into ghost-only cycles. That is established by the following
interval lemma.

## Interval lemma

For \(u\in\mathbb Z\), let \(r=u\bmod N\), with
\(0\leq r<N\). Directly from the definition of the increasing
\(\mathcal B\) list,

\[
Q_0^{-1}(u)=
\begin{cases}
u,&1\leq r\leq L,\\
u-(L+y),&L+1\leq r\leq L+y,\\
u-y,&\text{otherwise}.
\end{cases}
\]

Empty ranges contribute no case. In particular, the middle range is empty
when \(y=0\). The residue \(r=0\) belongs to the last case.

Consequently, with \(r=(z+x+y)\bmod N\),

\[
P_0(z)=
\begin{cases}
z+x+y,&1\leq r\leq L,\\
z+x-L,&L+1\leq r\leq L+y,\\
z+x,&\text{otherwise}.
\end{cases}
\tag{1}
\]

### Ghost intervals can exit only into the next real interval

For \(z\in G_0=[L+1,N-1]\), equation (1) gives

\[
P_0(z)\in G_0\cup A_1.
\tag{2}
\]

To see this, consider the three branches:

- In the \(z+x+y\) branch, the destination has residue in
  \([1,L]\). Since \(z+x+y>L\), it must lie in \(A_1\).
- The \(z+x\) branch lies between \(L+1\) and \(N+L-1\), hence
  in \(G_0\cup A_1\).
- In the \(z+x-L\) branch, \(u=z+x+y\) must have crossed \(N\).
  Otherwise \(u\leq L+y\) would imply \(z+x\leq L\), a
  contradiction. Writing \(u=N+r\), its destination is at least
  \(N+1-y\geq L+2\), and it is at most \(N-1\) because
  \(x\leq L\). Thus it remains in \(G_0\).

Periodicity gives \(P_0(G_k)\subseteq G_k\cup A_{k+1}\) for every
\(k\). A ghost-only orbit therefore remains in one finite ghost interval,
so every ghost-only cycle has zero net displacement.

### A real starting state enters the correct interval

Take \(s\in A_0\), and put \(v=s+x\). There are two cases.

If \(v\leq L\), then \(u=v+y\leq L+y<N\). Formula (1)
puts the first step either in \(A_0\) or in
\([1-y,-1]\subseteq G_{-1}\). A possible endpoint zero belongs to
\(A_0\). By (2), an excursion into \(G_{-1}\) can first return only
to \(A_0\).

If \(v\geq L+1\), then \(u=v+y\geq L+1+y\). For \(u<N\),
formula (1) gives \(P_0(s)=v\in G_0\). For \(u\geq N\), the
bound \(u\leq N+L-1\) makes its residue either zero or an
\(A\)-exclusive residue. A positive such residue gives an immediate
point in \(A_1\). Residue zero gives \(N-y\), which is in \(G_0\)
when \(y>0\), and is the real point \(N\in A_1\) when \(y=0\).
By (2), any excursion into \(G_0\) first returns to \(A_1\).

Every residue cycle starting at a real state returns to a real state in
finite time, since the residue map is a finite permutation. Therefore

\[
\alpha_0(s)\in
A_{\left\lfloor(s+x)/a\right\rfloor}.
\tag{3}
\]

This is exactly the period block containing \(\alpha_x(s)\).

## Completion of the proof

Equation (3), together with periodicity and bijectivity of first return,
implies that \(K=\alpha_x\alpha_0^{-1}\) preserves each finite interval
\(A_k\) as a set. Thus all its cycles have zero net displacement. This
includes the cycle through the shared root occurrence \(kN\); it returns
to that same occurrence, not a different lap.

For \(F_A=KP_0\), the correction fixes every ghost integer. A path starting
at a real \(A\) state therefore follows the \(P_0\) path until its first
return to \(\mathcal A\), at which point \(K\) changes its endpoint
from \(\alpha_0(s)\) to \(\alpha_x(s)\). Its exact first-return lift
is consequently \(\alpha_x\). Its ghost-only cycles are unchanged from
\(P_0\), and hence have zero net displacement.

For \(F_B=Q_0K^{-1}\), a non-root \(B\) state is fixed by \(K^{-1}\)
and immediately follows its desired \(Q_0\) transition. Starting at a root
\(kN\), the \(K^{-1}\) cycle visits only the finite interval \(A_k\).
At its final return to that same root occurrence, \(Q_0\) sends it to
\(\beta_y(kN)\). Because \(Q_0\) fixes every \(A\)-exclusive
integer, this description is also the actual first-return path of
\(Q_0K^{-1}\). Thus its first-return lift is exactly \(\beta_y\).
Its ghost-only cycles are the \(K^{-1}\) cycles disjoint from the root,
all contained in a finite \(A_k\), so their net displacements are zero.

Both full maps remain periodic bijections of the same root carrier, and

\[
F_BF_A=Q_0K^{-1}KP_0=Q_0P_0=\tau_{x+y}.
\]

This proves every assertion of the theorem. The empty-interval conventions
also cover \(a=1\), \(b=1\), zero values, and \(N=1\).

## Implementation and scope

`src/signedAdditiveModel.ts` implements this construction. Its public
`buildSignedAdditiveModel(N, w, a, x)` returns integer windows for both
operations and a certificate of their real returns, ghost-only cycles and
composition. It validates the finite integer inputs.

`tests/signedAdditiveModel.test.mjs` independently walks the returned edge
increments for all 40,920 ordered numeric splits through parent size 30. It
checks requested versus returned parameters, exact absolute-coordinate paths,
coset boundaries, root-edge coverage, signed totals and parent composition.
The finite audit verifies the implementation; the argument above establishes
the theorem for all sizes within the stated mathematical model.

The linear operation count comes from constructing and inverting windows,
then following the disjoint real-to-real routes. Each ghost is visited by at
most one such route; a ghost-only cycle need not be traversed to compute the
first-return correction. All membership tests used for traversal are sets.
Integers used by the constructor are of magnitude bounded by a constant
times \(N\).

The theorem proves that a completion exists for every scalar pair with these
specified contiguous supports. It does not assert that every alternative
support placement or every residue completion admits signed offsets. It
also does not make independently chosen completions equivalent under
arbitrary later observations. The construction chooses a definite
normalization, and its compatibility with nesting, reassociation, and
additional shared views remains a separate obligation.

## Residue factorization without arithmetic: a separate warning

If winding conditions are omitted, first-return factorization is much less
restrictive. For *any* desired parent permutation \(C\) and child
permutations \(\alpha,\beta\) on two sets sharing one root, extend
\(\beta\) by fixing \(A\setminus B\), call that extension \(Q_0\),
and put \(P_0=Q_0^{-1}C\). With \(\alpha_0=\mathrm{FR}_A(P_0)\),
set \(K=\alpha\alpha_0^{-1}\) on \(A\), fixing the complement.
Then \(P=KP_0\), \(Q=Q_0K^{-1}\) have the desired residue first
returns and satisfy \(QP=C\). The \(K\)-cycle containing the shared
root is inserted into its \(B\) edge; all other \(K\)-cycles involve
only \(B\)-ghosts.

Thus projected first-return equations can fit an arbitrary parent
permutation. The signed net-displacement requirements and homogeneous
parent lift are the constraints that give this particular factorization
its arithmetic meaning.
