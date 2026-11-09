"""Lazy phase-zero singlet addressing for the proposed Proteus finite model.

Python 3.10+, standard library only. Run this file for examples.

This implements the experimental coset / phase-zero projection conventions of
singlet_maps_v4, NOT a derivation of those conventions from all Proteus axioms.
Integer arithmetic is used for coordinates and factor-value metadata. There is
no evaluator for Proteus arithmetic expressions here.

An address is (root occurrence, decomposition path, local singlet index, basis).
A product step selects a particular factor occurrence; a span step is an
EXPLICIT coordinate slice, not the meaning of the Proteus '+' operator.

'baseline' is a local baseline index (the coordinate to investigate for local
+y). 'order' is a position in that context's state-dependent coset traversal.
They are not interchangeable. A lookup names a singlet; it does not assert or
change a part's selected state.

Phase zero fixes all opposing product coordinates to zero. A ghost's winding
number is NOT a factor phase. Extra phase cycles, carry, arithmetic inference,
unknown sizes and multiplication of unvalued span views are not implemented.
"""
from __future__ import annotations

from dataclasses import dataclass, field
from math import gcd
from typing import Literal, Optional, Union

Basis = Literal["baseline", "order"]
Side = Literal["L", "R"]


def _integer(name: str, value: int, minimum: int = 0) -> None:
    if type(value) is not int:
        raise TypeError(f"{name} must be an integer, not {type(value).__name__}")
    if value < minimum:
        raise ValueError(f"{name} must be >= {minimum}")


def _basis(basis: str) -> None:
    if basis not in ("baseline", "order"):
        raise ValueError("basis must be 'baseline' or 'order'")


def _state(size: int, value: int) -> None:
    _integer("size", size, 1)
    _integer("value", value)
    if value >= size:
        raise ValueError("require 0 <= value < size; carry is not silently discarded")


def _index(index: int, size: int) -> None:
    _integer("index", index)
    if index >= size:
        raise IndexError(f"index {index} is outside a {size}-member context")


def traversal_lift(size: int, value: int, position: int) -> int:
    """Unreduced lift for one complete coset traversal, positions 0..size-1."""
    _state(size, value)
    _index(position, size)
    period = size // gcd(size, value)
    coset, turn = divmod(position, period)
    return coset + turn * value


def order_position(size: int, value: int, baseline_index: int) -> int:
    """Inverse of traversal_lift(...) % size, including zero/non-coprime values."""
    _state(size, value)
    _index(baseline_index, size)
    g = gcd(size, value)
    period = size // g
    coset = baseline_index % g
    if period == 1:
        return coset
    turn = ((baseline_index - coset) // g * pow(value // g, -1, period)) % period
    return coset * period + turn


@dataclass(frozen=True)
class Root:
    size: int
    value: int
    label: str = "root"
    # Equal sizes/values/labels in independently constructed roots do NOT
    # establish singlet identity. Reuse the same Root for alternate views.
    _token: object = field(default_factory=object, repr=False)

    def __post_init__(self) -> None:
        _state(self.size, self.value)
        if not isinstance(self.label, str):
            raise TypeError("label must be a string")


@dataclass(frozen=True)
class Mul:
    """Split current occurrence into first*opposing; choose L or R at phase 0."""
    first: int
    opposing: int
    side: Side

    def __post_init__(self) -> None:
        _integer("first", self.first, 1)
        _integer("opposing", self.opposing, 1)
        if self.side not in ("L", "R"):
            raise ValueError("side must be 'L' or 'R'")


@dataclass(frozen=True)
class Span:
    """A consecutive slice in the parent's stated coordinate system.

    Child local baseline j refers to parent start+j in 'basis'. The slice
    supplies no child selected value. This is a view, not a new independent
    infon and not an automatic interpretation of *size+value.
    """
    start: int
    size: int
    basis: Basis = "baseline"

    def __post_init__(self) -> None:
        _integer("start", self.start)
        _integer("size", self.size, 1)
        _basis(self.basis)


Step = Union[Mul, Span]
Path = tuple[Step, ...]


@dataclass(frozen=True)
class SingletRef:
    path: Path
    index: int
    basis: Basis = "baseline"

    def __post_init__(self) -> None:
        if not isinstance(self.path, tuple):
            raise TypeError("path must be an immutable tuple")
        if any(not isinstance(step, (Mul, Span)) for step in self.path):
            raise TypeError("each path step must be Mul or Span")
        _integer("index", self.index)
        _basis(self.basis)


@dataclass(frozen=True)
class SingletId:
    root: Root
    index: int

    def __post_init__(self) -> None:
        _index(self.index, self.root.size)


@dataclass(frozen=True)
class Occurrence:
    """Identity plus unreduced position provenance. Equality includes the lift."""
    root: Root
    lift: int

    @property
    def index(self) -> int:
        return self.lift % self.root.size

    @property
    def identity(self) -> SingletId:
        return SingletId(self.root, self.index)

    @property
    def winding(self) -> int:
        return self.lift // self.root.size

    @property
    def is_ghost(self) -> bool:
        return not 0 <= self.lift < self.root.size


@dataclass(frozen=True)
class _Frame:
    step: Step
    parent_size: int
    parent_value: Optional[int]
    child_size: int
    child_value: Optional[int]


def _frames(root: Root, path: Path) -> tuple[tuple[_Frame, ...], int, Optional[int]]:
    if not isinstance(path, tuple):
        raise TypeError("path must be a tuple")
    size, value = root.size, root.value
    frames = []
    for step in path:
        if isinstance(step, Mul):
            if step.first * step.opposing != size:
                raise ValueError(f"{step.first}*{step.opposing} does not match context size {size}")
            if value is None:
                raise ValueError("a span view has no supplied state/traversal; cannot factor it")
            first_value, opposing_value = divmod(value, step.opposing)
            child_size, child_value = ((step.first, first_value) if step.side == "L"
                                       else (step.opposing, opposing_value))
        elif isinstance(step, Span):
            if step.start + step.size > size:
                raise ValueError("span extends beyond its current context")
            if step.basis == "order" and value is None:
                raise ValueError("this unvalued span has no state-dependent traversal")
            child_size, child_value = step.size, None
        else:
            raise TypeError("path steps must be Mul or Span")
        frames.append(_Frame(step, size, value, child_size, child_value))
        size, value = child_size, child_value
    return tuple(frames), size, value


def part_info(root: Root, path: Path) -> tuple[int, Optional[int]]:
    """Return (local size, model's local value); None marks an unvalued span view."""
    _, size, value = _frames(root, path)
    return size, value


def _convert(index: int, source: Basis, target: Basis,
             size: int, value: Optional[int]) -> int:
    if source == target:
        return index
    if value is None:
        raise ValueError("an unvalued span supports baseline coordinates only")
    if target == "order":
        return order_position(size, value, index)
    return traversal_lift(size, value, index) % size


def locate(root: Root, ref: SingletRef) -> Occurrence:
    """Resolve a path without enumerating the root's singlets.

    Walk the address back to its root, composing local coordinate maps. A
    product child uses the PARENT traversal, never a freshly invented root.
    Preserve the unreduced root lift until identity is requested.
    """
    frames, size, value = _frames(root, ref.path)
    _index(ref.index, size)
    if value is None and ref.basis == "order":
        raise ValueError("an unvalued span supports baseline coordinates only")
    index, basis = ref.index, ref.basis
    for frame in reversed(frames):
        step = frame.step
        if isinstance(step, Mul):
            position = _convert(index, basis, "order", frame.child_size, frame.child_value)
            index = position * step.opposing if step.side == "L" else position
            basis = "order"
        else:
            # The span's baseline was defined by this exact parent slice.
            if basis != "baseline":
                raise ValueError("cannot use a state-dependent order for an unvalued span")
            index += step.start
            basis = step.basis
    lift = index if basis == "baseline" else traversal_lift(root.size, root.value, index)
    return Occurrence(root, lift)


def phase0_index(root: Root, ref: SingletRef) -> int:
    """The requested normalized ROOT-baseline index, in 0..root.size-1."""
    return locate(root, ref).index


def local_index_of(root: Root, path: Path, singlet: SingletId,
                   basis: Basis = "baseline") -> Optional[int]:
    """Inverse lookup in one phase-zero part; None means the singlet is absent.

    This is an inverse ADDRESS map, not multiplicative/arithmetic inversion.
    Identity requires the same root occurrence, not just equal numerical sizes.
    """
    _basis(basis)
    frames, size, value = _frames(root, path)
    if basis == "order" and value is None:
        raise ValueError("an unvalued span supports baseline coordinates only")
    if singlet.root != root:
        return None
    index, current_basis = singlet.index, "baseline"
    for frame in frames:
        step = frame.step
        if isinstance(step, Mul):
            position = _convert(index, current_basis, "order", frame.parent_size, frame.parent_value)
            if step.side == "L":
                index, remainder = divmod(position, step.opposing)
                if remainder:
                    return None
            else:
                if position >= step.opposing:
                    return None
                index = position
            current_basis = "order"
        else:
            position = _convert(index, current_basis, step.basis, frame.parent_size, frame.parent_value)
            if not step.start <= position < step.start + step.size:
                return None
            index = position - step.start
            current_basis = "baseline"
    return _convert(index, current_basis, basis, size, value)


def demo() -> None:
    root = Root(12, 5)
    paths = {"3 of 3*4": (Mul(3, 4, "L"),),
             "4 of 3*4": (Mul(3, 4, "R"),),
             "baseline [0:5]": (Span(0, 5),),
             "baseline [5:12]": (Span(5, 7),)}
    print("*12+5; numbers below are ROOT baseline identities")
    for name, path in paths.items():
        size, _ = part_info(root, path)
        refs = [locate(root, SingletRef(path, j)) for j in range(size)]
        print(f"{name:20s} indices={[r.index for r in refs]}, lifts={[r.lift for r in refs]}")
    a = locate(root, SingletRef(paths["3 of 3*4"], 1))
    b = locate(root, SingletRef(paths["baseline [5:12]"], 3))
    print(f"Factor local 1 and suffix local 3: same identity={a.identity == b.identity}; "
          f"lifts={a.lift},{b.lift}; baseline={a.index}")
    print("\nThree distinct size-2 factor occurrences in *8+5:")
    r8 = Root(8, 5)
    factors = ((Mul(2, 4, "L"),),
               (Mul(2, 4, "R"), Mul(2, 2, "L")),
               (Mul(2, 4, "R"), Mul(2, 2, "R")))
    for name, path in zip("ABC", factors):
        print(name, [phase0_index(r8, SingletRef(path, j)) for j in range(2)])
    print("\nCoordinate distinction in the opposing 4 of *8+3:")
    r83 = Root(8, 3)
    path = (Mul(2, 4, "R"),)
    print("local baseline 1 ->", phase0_index(r83, SingletRef(path, 1, "baseline")))
    print("local order    1 ->", phase0_index(r83, SingletRef(path, 1, "order")))
    print("\nThree consecutive length-4 spans in the *12+5 traversal:")
    for start in (0, 4, 8):
        path = (Span(start, 4, "order"),)
        print(start, [phase0_index(root, SingletRef(path, j)) for j in range(4)])


if __name__ == "__main__":
    demo()
