"""Ordered multiplicative composition and its phase-0 decomposition."""
from __future__ import annotations

from dataclasses import dataclass

from .core import Infon
from .singlets import FactorContext


@dataclass(frozen=True)
class MultiplicativeComposition:
    """One ordered * composition, including the proposed singlet witnesses."""

    left: Infon
    right: Infon
    parent: Infon
    left_context: FactorContext
    right_context: FactorContext

    def root_index(self, component: str, position: int) -> int:
        """Map a component's traversal-position singlet to root baseline."""
        if component == "left":
            return self.left_context.root_index_at_position(position)
        if component == "right":
            return self.right_context.root_index_at_position(position)
        raise ValueError("component must be 'left' or 'right'")


def compose_multiplicative(left: Infon, right: Infon) -> MultiplicativeComposition:
    """Ordered composition: *a+x followed by *b+y -> *(a*b)+(x*b+y).

    The scalar law is exact for ordered mixed-radix composition. The attached
    factor singlet map follows the existing experimental phase-0 rule.
    """
    x, y = left.require_value(), right.require_value()
    parent = Infon(left.size * right.size, x * right.size + y)
    a, b = FactorContext.root(parent.size, parent.require_value()).split_phase0(
        left.size, right.size
    )
    assert (a.size, a.value) == (left.size, x)
    assert (b.size, b.value) == (right.size, y)
    return MultiplicativeComposition(left, right, parent, a, b)


def decompose_multiplicative(
    parent: Infon, first_size: int
) -> MultiplicativeComposition:
    """Recover ordered factors of a parent (second size is inferred).

    Return the same composition object as compose_multiplicative so its
    contextual singlet maps are available in both directions.
    """
    v = parent.require_value()
    if type(first_size) is not int or first_size < 1 or parent.size % first_size:
        raise ValueError("first_size must be a positive factor of parent.size")
    second_size = parent.size // first_size
    x, y = divmod(v, second_size)
    result = compose_multiplicative(Infon(first_size, x), Infon(second_size, y))
    assert result.parent == parent
    return result
