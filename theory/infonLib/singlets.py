"""Phase-0 singlet identity maps from the existing multiplicative experiments.

This is the singlet_maps_v4 hypothesis, not a proof of the full Proteus
composition semantics.  An unwrapped lift records provenance, whereas
lift % root_size identifies the root-baseline singlet.
"""
from __future__ import annotations

from dataclasses import dataclass
from math import gcd, prod
from typing import Sequence


def traversal_lift(size: int, value: int, position: int) -> int:
    """Unwrapped root-baseline index at a phase-0 traversal position."""
    if size < 1 or not 0 <= value < size or not 0 <= position < size:
        raise ValueError("invalid size, value, or traversal position")
    g = gcd(size, value)
    per_coset = size // g
    coset, phase = divmod(position, per_coset)
    return coset + phase * value


def base_of(size: int, value: int, position: int) -> int:
    """Root-baseline identity at a traversal position."""
    return traversal_lift(size, value, position) % size


def location_of(size: int, value: int, baseline_index: int) -> int:
    """Inverse of base_of, including multi-coset cases and value zero."""
    if size < 1 or not 0 <= value < size or not 0 <= baseline_index < size:
        raise ValueError("invalid size, value, or baseline index")
    g = gcd(size, value)
    per_coset = size // g
    coset = baseline_index % g
    if per_coset == 1:
        return coset
    step = (((baseline_index - coset) // g)
            * pow(value // g, -1, per_coset)) % per_coset
    return coset * per_coset + step


def canonical_traversal(size: int, value: int) -> tuple[int, ...]:
    return tuple(base_of(size, value, i) for i in range(size))


def successor(size: int, value: int) -> tuple[int, ...]:
    """Successor permutation indexed by root-baseline singlet identity."""
    if size < 1 or not 0 <= value < size:
        raise ValueError("invalid size or value")
    return tuple((i + value) % size for i in range(size))


@dataclass(frozen=True)
class GhostRef:
    """One unwrapped appearance of a root singlet; not an extra root state."""

    lift: int
    root_size: int

    def __post_init__(self) -> None:
        if self.root_size < 1:
            raise ValueError("root_size must be positive")

    @property
    def base(self) -> int:
        return self.lift % self.root_size

    @property
    def offset(self) -> int:
        return self.lift // self.root_size

    @property
    def is_ghost(self) -> bool:
        return not 0 <= self.lift < self.root_size


@dataclass(frozen=True)
class FactorContext:
    """A particular factor occurrence, with distinct root singlet identities.

    ordered_lifts is indexed by the LOCAL phase-0 traversal.
    baseline_lifts is indexed by the LOCAL canonical baseline.
    Both contain ROOT-baseline lifts, not local singlet numbers.
    """

    name: str
    root_size: int
    size: int
    value: int
    ordered_lifts: tuple[int, ...]
    baseline_lifts: tuple[int, ...]

    @classmethod
    def root(cls, size: int, value: int, name: str = "root") -> FactorContext:
        if size < 1 or not 0 <= value < size:
            raise ValueError("invalid root infon")
        order = tuple(traversal_lift(size, value, i) for i in range(size))
        return cls(name, size, size, value, order, tuple(range(size)))

    @classmethod
    def from_order(
        cls, name: str, root_size: int, value: int, ordered_lifts: Sequence[int]
    ) -> FactorContext:
        order = tuple(ordered_lifts)
        size = len(order)
        if size < 1 or size > root_size or not 0 <= value < size:
            raise ValueError("invalid child size or value")
        if len({lift % root_size for lift in order}) != size:
            raise ValueError("factor singlets must be distinct in the root")
        # Construct the local baseline for the proposed canonical child.
        # This is a reconstruction, NOT a proof that the baseline is inherited.
        baseline = tuple(
            order[location_of(size, value, b)] for b in range(size)
        )
        return cls(name, root_size, size, value, order, baseline)

    def split_phase0(
        self, first_size: int, second_size: int,
        first_name: str = "first", second_name: str = "opposing"
    ) -> tuple[FactorContext, FactorContext]:
        """Proposed ordered * factor decomposition from the v4 experiment."""
        if first_size < 1 or second_size < 1 or first_size * second_size != self.size:
            raise ValueError("factor sizes must multiply to the context size")
        a_value, b_value = divmod(self.value, second_size)
        a_order = tuple(self.ordered_lifts[i * second_size]
                        for i in range(first_size))
        b_order = self.ordered_lifts[:second_size]
        return (
            self.from_order(first_name, self.root_size, a_value, a_order),
            self.from_order(second_name, self.root_size, b_value, b_order),
        )

    def root_index_at_position(self, position: int) -> int:
        """Root-baseline singlet identity at LOCAL traversal position."""
        return self.ordered_lifts[position] % self.root_size

    def root_index_at_local_baseline(self, index: int) -> int:
        """Root-baseline singlet identity at LOCAL baseline index."""
        return self.baseline_lifts[index] % self.root_size

    def members(self) -> tuple[GhostRef, ...]:
        return tuple(GhostRef(lift, self.root_size)
                     for lift in self.ordered_lifts)

    def base_members(self) -> tuple[int, ...]:
        return tuple(lift % self.root_size for lift in self.ordered_lifts)


def split_all(root: FactorContext, factors: Sequence[int]) -> tuple[FactorContext, ...]:
    """Right-nested ordered factorization preserving occurrence identity."""
    factors = tuple(factors)
    if not factors or any(x < 1 for x in factors) or prod(factors) != root.size:
        raise ValueError("factor sizes must multiply to context size")
    current = root
    pieces: list[FactorContext] = []
    for i, size in enumerate(factors[:-1]):
        first, current = current.split_phase0(
            size, current.size // size, f"factor_{i}", f"remainder_{i+1}"
        )
        pieces.append(first)
    pieces.append(current)
    return tuple(pieces)
