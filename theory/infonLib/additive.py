"""Experimental additive composition with a common root singlet 0.

This is NOT a theorem about infonic addition.  It implements the earlier
ghost-completion permutation hypothesis as a finite search.  The ghost
pass-through occurrences in this model must not be conflated with unwrapped
out-of-baseline lifts used by the multiplicative singlet mapper.

For A=*a+x and B=*b+y, the CANDIDATE parent is *(a+b-1)+(x+y).
A witness uses real singlet sets SA and SB, sharing exactly root singlet 0,
and root-baseline successor permutations EA and EB satisfying:

    EB(EA(i)) = (i+x+y) mod (a+b-1)
    first_return(EA, SA) = rotation(a, x)
    first_return(EB, SB) = rotation(b, y)
    wraps(EA) = x, wraps(EB) = y

Permitting arbitrary permutations subject to these tests remains a
hypothesis.  Finding a witness proves only satisfiability of THIS model.
"""
from __future__ import annotations

from dataclasses import dataclass
from functools import lru_cache
from itertools import combinations, permutations
from typing import Literal

from .core import Infon
from .singlets import successor

AdditiveStatus = Literal["witnessed_in_model", "no_witness_in_model", "not_searched"]


class SearchLimitError(ValueError):
    """Exhaustive ghost-permutation search would exceed its configured bound."""


def _wraps(perm: tuple[int, ...]) -> int:
    """Count transitions crossing the chosen root-baseline cut at zero."""
    return sum(target < source for source, target in enumerate(perm))


def _first_return(perm: tuple[int, ...], selected: tuple[int, ...]) -> tuple[int, ...]:
    """Induced successor ranks of selected singlets in ascending baseline order."""
    ranks = {identity: rank for rank, identity in enumerate(selected)}
    if len(ranks) != len(selected) or not selected:
        raise ValueError("selected singlets must be nonempty and distinct")
    out: list[int] = []
    for identity in selected:
        at = perm[identity]
        while at not in ranks:
            at = perm[at]
        out.append(ranks[at])
    return tuple(out)


@dataclass(frozen=True)
class AdditiveWitness:
    """A FULL witness under the experimental shared-zero permutation model."""

    left: Infon
    right: Infon
    parent: Infon
    left_real: tuple[int, ...]
    right_real: tuple[int, ...]
    left_successor: tuple[int, ...]
    right_successor: tuple[int, ...]

    @property
    def left_ghosts(self) -> tuple[int, ...]:
        return tuple(i for i in range(self.parent.size) if i not in self.left_real)

    @property
    def right_ghosts(self) -> tuple[int, ...]:
        return tuple(i for i in range(self.parent.size) if i not in self.right_real)

    def verify(self) -> bool:
        """Check all model constraints independently of how witness was found."""
        n = self.parent.size
        if n != self.left.size + self.right.size - 1:
            return False
        if self.parent.value != self.left.require_value() + self.right.require_value():
            return False
        sa, sb = self.left_real, self.right_real
        if (len(sa) != self.left.size or len(sb) != self.right.size
                or tuple(sorted(set(sa))) != sa
                or tuple(sorted(set(sb))) != sb
                or set(sa) & set(sb) != {0}
                or set(sa) | set(sb) != set(range(n))):
            return False
        ea, eb = self.left_successor, self.right_successor
        if set(ea) != set(range(n)) or set(eb) != set(range(n)):
            return False
        if len(ea) != n or len(eb) != n:
            return False
        if tuple(eb[ea[i]] for i in range(n)) != successor(n, self.parent.require_value()):
            return False
        if _wraps(ea) != self.left.require_value() or _wraps(eb) != self.right.require_value():
            return False
        return (
            _first_return(ea, sa) == successor(self.left.size, self.left.require_value())
            and _first_return(eb, sb)
                == successor(self.right.size, self.right.require_value())
        )


@dataclass(frozen=True)
class AdditiveComposition:
    left: Infon
    right: Infon
    parent: Infon
    status: AdditiveStatus
    witness: AdditiveWitness | None

    @property
    def is_witnessed(self) -> bool:
        return self.status == "witnessed_in_model" and self.witness is not None


@lru_cache(maxsize=256)
def _find_witness(left: Infon, right: Infon) -> AdditiveWitness | None:
    x, y = left.require_value(), right.require_value()
    n = left.size + right.size - 1
    parent = Infon(n, x + y)
    p = successor(n, parent.require_value())
    target_left = successor(left.size, x)
    target_right = successor(right.size, y)

    # A root-baseline singlet is real in exactly one child except for 0.
    others = tuple(range(1, n))
    # At most n=7 by default: exhaustive, factorial, intentionally modest.
    a_maps = tuple(perm for perm in permutations(range(n)) if _wraps(perm) == x)
    for a_extra in combinations(others, left.size - 1):
        sa = (0, *a_extra)
        sa_set = set(sa)
        sb = (0, *(i for i in others if i not in sa_set))
        for ea in a_maps:
            if _first_return(ea, sa) != target_left:
                continue
            inverse = [0] * n
            for i, target in enumerate(ea):
                inverse[target] = i
            # Unique EB satisfying EB o EA = parent's successor.
            eb = tuple(p[inverse[i]] for i in range(n))
            if _wraps(eb) != y or _first_return(eb, sb) != target_right:
                continue
            witness = AdditiveWitness(left, right, parent, sa, sb, ea, eb)
            assert witness.verify()
            return witness
    return None


def find_additive_witness(
    left: Infon, right: Infon, *, max_parent_size: int = 7
) -> AdditiveWitness | None:
    """Exhaustively find one witness, or None if no witness IN THIS MODEL.

    Raises SearchLimitError rather than confusing an unsearched size with a
    mathematical impossibility.  Does not prove general infonic validity.
    """
    left.require_value()
    right.require_value()
    n = left.size + right.size - 1
    if type(max_parent_size) is not int or max_parent_size < 1:
        raise ValueError("max_parent_size must be a positive integer")
    if n > max_parent_size:
        raise SearchLimitError(
            f"parent size {n} exceeds exhaustive search bound {max_parent_size}"
        )
    return _find_witness(left, right)


def compose_additive(
    left: Infon, right: Infon, *, max_parent_size: int = 7
) -> AdditiveComposition:
    """Construct a candidate sum AND classify its experimental witness status."""
    x, y = left.require_value(), right.require_value()
    parent = Infon(left.size + right.size - 1, x + y)
    if type(max_parent_size) is not int or max_parent_size < 1:
        raise ValueError("max_parent_size must be a positive integer")
    if parent.size > max_parent_size:
        return AdditiveComposition(left, right, parent, "not_searched", None)
    witness = find_additive_witness(
        left, right, max_parent_size=max_parent_size
    )
    if witness is None:
        return AdditiveComposition(left, right, parent, "no_witness_in_model", None)
    return AdditiveComposition(left, right, parent, "witnessed_in_model", witness)


def decompose_additive(
    parent: Infon, first_size: int, *, max_parent_size: int = 7
) -> tuple[AdditiveComposition, ...]:
    """Enumerate witnessed additive splits with first_size real singlets.

    The returned list is exhaustive only for this permutation hypothesis and
    the explicitly bounded parent size, not for general Proteus arithmetic.
    """
    value = parent.require_value()
    if type(first_size) is not int or not 1 <= first_size <= parent.size:
        raise ValueError("first_size must lie in [1, parent.size]")
    if parent.size > max_parent_size:
        raise SearchLimitError("parent too large for exhaustive additive decomposition")
    second_size = parent.size + 1 - first_size
    smallest_x = max(0, value - (second_size - 1))
    largest_x = min(first_size - 1, value)
    results: list[AdditiveComposition] = []
    for x in range(smallest_x, largest_x + 1):
        y = value - x
        comp = compose_additive(
            Infon(first_size, x), Infon(second_size, y),
            max_parent_size=max_parent_size,
        )
        if comp.is_witnessed:
            assert comp.parent == parent
            results.append(comp)
    return tuple(results)
