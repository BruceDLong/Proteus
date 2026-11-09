"""Regression/property checks for the explicitly proposed finite mapping model.

The enumerating oracle below uses cycle walking and list slicing, rather than
v5's gcd/modular-inverse formula and reverse path composition. Agreement checks
the implementation of the model; it does not prove the model's Proteus axioms.
"""
from itertools import product
from math import prod
import unittest

from singlet_maps_v5 import (
    Root, Mul, Span, SingletRef, SingletId, locate, phase0_index,
    local_index_of, part_info, traversal_lift, order_position,
)

COUNTS = {"oracle_lookups": 0, "inverse_round_trips": 0,
          "membership_checks": 0, "block_identities": 0,
          "reassociation_checks": 0}


def walk_lifts(n, v):
    """Independent enumerator: each new coset starts at the least unused ID."""
    used = set()
    answer = []
    for start in range(n):
        if start in used:
            continue
        lift = start
        while lift % n not in used:
            used.add(lift % n)
            answer.append(lift)
            lift += v
    return answer


def oracle_context(root, path):
    n, v = root.size, root.value
    baseline = list(range(n))
    ordered = walk_lifts(n, v)
    for step in path:
        if isinstance(step, Mul):
            if step.side == "L":
                ordered = ordered[::step.opposing]
                n, v = step.first, v // step.opposing
            else:
                ordered = ordered[:step.opposing]
                n, v = step.opposing, v % step.opposing
            local_order = [x % n for x in walk_lifts(n, v)]
            baseline = [ordered[local_order.index(k)] for k in range(n)]
        else:
            source = baseline if step.basis == "baseline" else ordered
            baseline = source[step.start:step.start + step.size]
            n, v, ordered = step.size, None, None
    return n, v, baseline, ordered


class MappingTests(unittest.TestCase):
    def check_path(self, root, path, membership=False):
        n, v, baseline, ordered = oracle_context(root, path)
        self.assertEqual(part_info(root, path), (n, v))
        for basis, lifts in (("baseline", baseline), ("order", ordered)):
            if lifts is None:
                continue
            ids = [lift % root.size for lift in lifts]
            self.assertEqual(len(set(ids)), n)
            for j, expected in enumerate(lifts):
                ref = SingletRef(path, j, basis)
                found = locate(root, ref)
                self.assertEqual(found.lift, expected, (root, ref))
                self.assertEqual(found.index, ids[j])
                COUNTS["oracle_lookups"] += 1
                self.assertEqual(local_index_of(root, path, found.identity, basis), j)
                COUNTS["inverse_round_trips"] += 1
            if membership:
                inverse = {base: j for j, base in enumerate(ids)}
                for base in range(root.size):
                    self.assertEqual(local_index_of(root, path, SingletId(root, base), basis),
                                     inverse.get(base))
                    COUNTS["membership_checks"] += 1

    def test_all_roots_through_64(self):
        for n in range(1, 65):
            for v in range(n):
                root = Root(n, v)
                lifts = walk_lifts(n, v)
                self.assertEqual(sorted(x % n for x in lifts), list(range(n)))
                for p, lift in enumerate(lifts):
                    self.assertEqual(traversal_lift(n, v, p), lift)
                    self.assertEqual(order_position(n, v, lift % n), p)
                self.check_path(root, ())

    def test_all_binary_factorizations_through_64(self):
        for n in range(1, 65):
            for a in range(1, n + 1):
                if n % a:
                    continue
                for v in range(n):
                    root = Root(n, v)
                    for side in ("L", "R"):
                        self.check_path(root, (Mul(a, n // a, side),), membership=n <= 16)

    def test_nested_factors_and_reassociation(self):
        for a, b, c in product(range(2, 6), repeat=3):
            n = a * b * c
            if n > 64:
                continue
            right = ((Mul(a, b*c, "L"),),
                     (Mul(a, b*c, "R"), Mul(b, c, "L")),
                     (Mul(a, b*c, "R"), Mul(b, c, "R")))
            left = ((Mul(a*b, c, "L"), Mul(a, b, "L")),
                    (Mul(a*b, c, "L"), Mul(a, b, "R")),
                    (Mul(a*b, c, "R"),))
            for v in range(n):
                root = Root(n, v)
                for size, p, q in zip((a, b, c), right, left):
                    self.check_path(root, p)
                    self.check_path(root, q)
                    for basis in ("baseline", "order"):
                        for j in range(size):
                            self.assertEqual(locate(root, SingletRef(p, j, basis)),
                                             locate(root, SingletRef(q, j, basis)))
                            COUNTS["reassociation_checks"] += 1

    def test_spans_and_contextual_offsets(self):
        # All nonempty contiguous spans for small roots, in BOTH coordinates.
        for n in range(1, 13):
            for v in range(n):
                root = Root(n, v)
                for start in range(n):
                    for size in range(1, n - start + 1):
                        for basis in ("baseline", "order"):
                            self.check_path(root, (Span(start, size, basis),), membership=n <= 6)
        root = Root(12, 5)
        # Nested + offsets must be added in the context they actually belong to.
        path = (Span(5, 7), Span(2, 4))
        self.check_path(root, path, membership=True)
        self.assertEqual(phase0_index(root, SingletRef(path, 3)), 10)

    def test_spans_inside_factor_occurrences(self):
        for n in range(1, 25):
            for a in range(1, n + 1):
                if n % a:
                    continue
                for v in range(n):
                    root = Root(n, v)
                    for side in ("L", "R"):
                        path = (Mul(a, n // a, side),)
                        size, _ = part_info(root, path)
                        for start in range(size):
                            for basis in ("baseline", "order"):
                                self.check_path(root, path + (Span(start, size - start, basis),))

    def test_additive_blocks_and_product_sections_share_singlets(self):
        # Product sections pick block starts / the first block of the SAME order.
        for n in range(1, 33):
            for a in range(1, n + 1):
                if n % a:
                    continue
                b = n // a
                for v in range(n):
                    root = Root(n, v)
                    for i in range(a):
                        first = locate(root, SingletRef((Mul(a, b, "L"),), i, "order"))
                        head = locate(root, SingletRef((Span(i*b, b, "order"),), 0))
                        self.assertEqual(first, head)
                        for j in range(b):
                            block = locate(root, SingletRef((Span(i*b, b, "order"),), j))
                            whole = locate(root, SingletRef((), i*b+j, "order"))
                            self.assertEqual(block, whole)
                            COUNTS["block_identities"] += 1
                            if i == 0:
                                opposing = locate(root, SingletRef((Mul(a, b, "R"),), j, "order"))
                                self.assertEqual(opposing, block)

    def test_named_examples_and_ghosts(self):
        root = Root(12, 5)
        a = locate(root, SingletRef((Mul(3, 4, "L"),), 1))
        b = locate(root, SingletRef((Span(5, 7),), 3))
        self.assertEqual((a.lift, b.lift, a.index, b.index), (20, 8, 8, 8))
        self.assertEqual(a.identity, b.identity)
        self.assertNotEqual(a, b)
        self.assertTrue(a.is_ghost)
        self.assertFalse(b.is_ghost)
        self.assertEqual(a.winding, 1)
        # First/second/third 2 in the same root must not collapse by size/value.
        r8 = Root(8, 5)
        paths = ((Mul(2, 4, "L"),),
                 (Mul(2, 4, "R"), Mul(2, 2, "L")),
                 (Mul(2, 4, "R"), Mul(2, 2, "R")))
        self.assertEqual([[phase0_index(r8, SingletRef(p, j)) for j in range(2)]
                          for p in paths], [[0, 4], [0, 2], [0, 5]])
        # A change in coordinates must not silently reinterpret a local +1.
        r83, path = Root(8, 3), (Mul(2, 4, "R"),)
        self.assertEqual(phase0_index(r83, SingletRef(path, 1, "baseline")), 1)
        self.assertEqual(phase0_index(r83, SingletRef(path, 1, "order")), 3)

    def test_identity_requires_same_root(self):
        r1, r2 = Root(12, 5), Root(12, 5)
        ref = SingletRef((Mul(3, 4, "L"),), 1)
        a, b = locate(r1, ref), locate(r2, ref)
        self.assertEqual(a.index, b.index)
        self.assertNotEqual(a.identity, b.identity)
        self.assertIsNone(local_index_of(r1, ref.path, b.identity))

    def test_huge_root_does_not_enumerate(self):
        n = 2**256
        root = Root(n, 5)
        path = (Mul(2, n//2, "L"),)
        ref = SingletRef(path, 1)
        found = locate(root, ref)
        self.assertEqual(found.lift, 5*(n//2))
        self.assertEqual(found.index, n//2)
        self.assertEqual(local_index_of(root, path, found.identity), 1)

    def test_invalid_addresses_and_unsupported_semantics(self):
        for args in ((0, 0), (-1, 0), (12, -1), (12, 12), (1, 1)):
            with self.assertRaises(ValueError):
                Root(*args)
        for args in ((True, 0), (12, 1.5)):
            with self.assertRaises(TypeError):
                Root(*args)
        root = Root(12, 5)
        with self.assertRaises(ValueError):
            phase0_index(root, SingletRef((Mul(3, 5, "L"),), 0))
        with self.assertRaises(IndexError):
            phase0_index(root, SingletRef((Mul(3, 4, "L"),), 3))
        with self.assertRaises(ValueError):
            phase0_index(root, SingletRef((Span(6, 7),), 0))
        with self.assertRaises(ValueError):
            phase0_index(root, SingletRef((Span(0, 4),), 0, "order"))
        with self.assertRaises(ValueError):
            phase0_index(root, SingletRef((Span(0, 4), Mul(2, 2, "L")), 0))
        with self.assertRaises(ValueError):
            phase0_index(root, SingletRef((Span(0, 5), Span(0, 2, "order")), 0))
        with self.assertRaises(ValueError):
            Span(0, 0)
        with self.assertRaises(ValueError):
            Mul(2, 6, "wrong")
        with self.assertRaises(TypeError):
            SingletRef([], 0)


if __name__ == "__main__":
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(MappingTests)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    print("\nCase counts (not a proof of the underlying theoretical conventions):")
    for name, count in COUNTS.items():
        print(f"  {name}: {count:,}")
    raise SystemExit(not result.wasSuccessful())
