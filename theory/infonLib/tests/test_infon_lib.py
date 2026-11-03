"""Regression tests for the finite-scalar prototype; run from Proteus/theory."""
from dataclasses import replace
from pathlib import Path
import tempfile
import unittest

from infonLib import (
    FactorContext,
    GhostRef,
    Infon,
    SearchLimitError,
    base_of,
    canonical_traversal,
    compose_additive,
    compose_multiplicative,
    decompose_additive,
    decompose_multiplicative,
    find_additive_witness,
    location_of,
    split_all,
    successor,
    traversal_lift,
)


class ScalarTests(unittest.TestCase):
    def test_text_and_json_roundtrips(self) -> None:
        for inf in (Infon(1, 0), Infon(12, 5), Infon(23), Infon(5, 0)):
            self.assertEqual(Infon.parse(str(inf)), inf)
            self.assertEqual(Infon.from_json(inf.to_json()), inf)
            with tempfile.TemporaryDirectory() as directory:
                path = Path(directory) / "infon.json"
                inf.save(path)
                self.assertEqual(Infon.load(path), inf)
        self.assertEqual(Infon.parse("  * 12 + 5 "), Infon(12, 5))
        self.assertEqual(str(Infon(8)), "*8+_")

    def test_invalid_values_are_not_implicitly_modded(self) -> None:
        for size, value in ((0, 0), (5, 5), (5, -1), (True, 0), (5, True)):
            with self.subTest(size=size, value=value):
                with self.assertRaises(ValueError):
                    Infon(size, value)
        with self.assertRaises(ValueError):
            Infon.parse("*5+-1")
        with self.assertRaises(ValueError):
            Infon.from_dict({"size": 5})
        with self.assertRaises(ValueError):
            Infon(5).require_value()


class SingletTests(unittest.TestCase):
    def test_baseline_inverse(self) -> None:
        for n in range(1, 49):
            for v in range(n):
                positions = canonical_traversal(n, v)
                self.assertEqual(sorted(positions), list(range(n)))
                for position, root_id in enumerate(positions):
                    self.assertEqual(location_of(n, v, root_id), position)
                    self.assertEqual(base_of(n, v, position), root_id)

    def test_ghost_lifts_not_extra_singlets(self) -> None:
        root = FactorContext.root(12, 5)
        self.assertEqual(root.ordered_lifts, tuple(range(0, 60, 5)))
        self.assertEqual(root.base_members(), (0, 5, 10, 3, 8, 1, 6, 11, 4, 9, 2, 7))
        self.assertEqual(GhostRef(15, 12).base, 3)
        self.assertEqual(GhostRef(15, 12).offset, 1)
        self.assertTrue(GhostRef(15, 12).is_ghost)
        self.assertFalse(GhostRef(3, 12).is_ghost)
        self.assertEqual(traversal_lift(12, 5, 3), 15)
        self.assertEqual(canonical_traversal(12, 4),
                         (0, 4, 8, 1, 5, 9, 2, 6, 10, 3, 7, 11))

    def test_nested_factor_identity(self) -> None:
        root = FactorContext.root(12, 5)
        a, bc = root.split_phase0(2, 6)
        b, c = bc.split_phase0(2, 3)
        ab, alt_c = root.split_phase0(4, 3)
        alt_a, alt_b = ab.split_phase0(2, 2)
        self.assertEqual(tuple(p.ordered_lifts for p in (a, b, c)),
                         tuple(p.ordered_lifts for p in (alt_a, alt_b, alt_c)))
        self.assertEqual(tuple(p.ordered_lifts for p in split_all(root, (2, 2, 3))),
                         tuple(p.ordered_lifts for p in (a, b, c)))
        for factor in (a, b, c):
            for i in range(factor.size):
                self.assertEqual(
                    factor.root_index_at_local_baseline(i),
                    factor.baseline_lifts[i] % root.size,
                )


class MultiplicativeTests(unittest.TestCase):
    def test_compose_decompose_roundtrip(self) -> None:
        for n in range(1, 31):
            for v in range(n):
                parent = Infon(n, v)
                for first_size in range(1, n + 1):
                    if n % first_size:
                        continue
                    result = decompose_multiplicative(parent, first_size)
                    self.assertEqual(result.parent, parent)
                    rebuilt = compose_multiplicative(result.left, result.right)
                    self.assertEqual(rebuilt.parent, parent)
                    self.assertEqual(rebuilt.left_context.ordered_lifts,
                                     result.left_context.ordered_lifts)
                    self.assertEqual(rebuilt.right_context.ordered_lifts,
                                     result.right_context.ordered_lifts)

    def test_twelve_states_and_associativity(self) -> None:
        result = compose_multiplicative(Infon(3, 1), Infon(4, 1))
        self.assertEqual(result.parent, Infon(12, 5))
        self.assertEqual(result.left_context.ordered_lifts, (0, 20, 40))
        self.assertEqual(result.right_context.ordered_lifts, (0, 5, 10, 15))
        self.assertEqual(result.root_index("left", 1), 8)
        self.assertEqual(result.root_index("right", 3), 3)
        a, b, c = Infon(2, 1), Infon(3, 2), Infon(4, 1)
        left_assoc = compose_multiplicative(
            compose_multiplicative(a, b).parent, c
        ).parent
        right_assoc = compose_multiplicative(
            a, compose_multiplicative(b, c).parent
        ).parent
        self.assertEqual(left_assoc, right_assoc)
        self.assertEqual(left_assoc, Infon(24, 21))


class ExperimentalAdditiveTests(unittest.TestCase):
    def test_ghost_completed_seven_state_example(self) -> None:
        result = compose_additive(Infon(4, 2), Infon(4, 3))
        self.assertEqual(result.parent, Infon(7, 5))
        self.assertEqual(result.status, "witnessed_in_model")
        self.assertTrue(result.is_witnessed)
        witness = result.witness
        self.assertIsNotNone(witness)
        assert witness is not None
        self.assertTrue(witness.verify())
        self.assertEqual(set(witness.left_real) & set(witness.right_real), {0})
        self.assertEqual(set(witness.left_real) | set(witness.right_real),
                         set(range(7)))
        self.assertEqual(
            tuple(witness.right_successor[witness.left_successor[i]]
                  for i in range(7)),
            successor(7, 5),
        )
        self.assertFalse(replace(witness, right_successor=tuple(range(7))).verify())

    def test_additive_inverse_and_zero(self) -> None:
        choices = decompose_additive(Infon(7, 5), 4)
        self.assertTrue(any(x.left == Infon(4, 2) and x.right == Infon(4, 3)
                            for x in choices))
        zero_case = compose_additive(Infon(2, 0), Infon(3, 0))
        self.assertEqual(zero_case.parent, Infon(4, 0))
        self.assertEqual(zero_case.status, "witnessed_in_model")
        self.assertTrue(zero_case.witness.verify())

    def test_unsearched_is_not_failure(self) -> None:
        result = compose_additive(Infon(5, 2), Infon(4, 3))
        self.assertEqual(result.parent, Infon(8, 5))
        self.assertEqual(result.status, "not_searched")
        self.assertIsNone(result.witness)
        with self.assertRaises(SearchLimitError):
            find_additive_witness(Infon(5, 2), Infon(4, 3))
        with self.assertRaises(SearchLimitError):
            decompose_additive(Infon(8, 5), 5)


if __name__ == "__main__":
    unittest.main()
