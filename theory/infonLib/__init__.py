"""Experimental infon arithmetic and singlet mappings for Proteus theory.

The stable API here is intentionally limited to finite scalar numeric infons.
Multiplicative mixed-radix composition has a direct scalar inverse; the
singlet-factor context remains an experimental structural model.
Additive composition reports its evidence status and is NOT yet axiomatized.
"""
from .core import Infon
from .singlets import (
    FactorContext,
    GhostRef,
    base_of,
    canonical_traversal,
    location_of,
    split_all,
    successor,
    traversal_lift,
)
from .multiplicative import (
    MultiplicativeComposition,
    compose_multiplicative,
    decompose_multiplicative,
)
from .additive import (
    AdditiveComposition,
    AdditiveStatus,
    AdditiveWitness,
    SearchLimitError,
    compose_additive,
    decompose_additive,
    find_additive_witness,
)

__all__ = [
    "Infon",
    "FactorContext",
    "GhostRef",
    "base_of",
    "canonical_traversal",
    "location_of",
    "split_all",
    "successor",
    "traversal_lift",
    "MultiplicativeComposition",
    "compose_multiplicative",
    "decompose_multiplicative",
    "AdditiveComposition",
    "AdditiveStatus",
    "AdditiveWitness",
    "SearchLimitError",
    "compose_additive",
    "decompose_additive",
    "find_additive_witness",
]
