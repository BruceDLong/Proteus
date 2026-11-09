"""Behavioral checks for canonical additive ghost reconstruction.

Run beside infon_additive_prefix_table.py with:
    python3 test_infon_additive_prefix_table.py
"""

import subprocess
import sys
import unittest
import csv
import io
from pathlib import Path

import infon_additive_prefix_table as lab


def walked_traversal(size, value):
    """Independent cycle walking, rather than the implementation's gcd formula."""
    seen = set()
    order = []
    for start in range(size):
        current = start
        while current not in seen:
            seen.add(current)
            order.append(current)
            current = (current + value) % size
    return order


class AdditiveGhostTests(unittest.TestCase):
    def test_known_repair_and_root_ghosts(self):
        row = next(r for r in lab.table_rows(7, ghosting="when-needed", ghost_step="descents")
                   if (r["x"], r["k"]) == (5, 5))
        a = row["A_details"]
        self.assertEqual(row["A_singlets"], (0, 5, 3, 1, 6))
        self.assertIsNone(a["value_before"])
        self.assertEqual((row["A_value"], row["Bend_value"]), (3, 2))
        self.assertTrue(row["SumEnd"])
        self.assertEqual(a["baseline"], (0, 3, 6, 5, 1))
        self.assertEqual(a["lifts"], (0, 3, 6, 12, 15))
        self.assertEqual(a["moves"], ((5, 5, 12), (1, 1, 15)))
        # The root has seven states; reducing these lifts modulo child size
        # five would destroy the selected identities.
        self.assertEqual(tuple(lift % 7 for lift in a["lifts"]), a["baseline"])
        self.assertNotEqual(tuple(lift % 5 for lift in a["lifts"]), a["baseline"])

    def test_structural_repairs_do_not_force_the_sum(self):
        for row in lab.table_rows(7, ghosting="when-needed", ghost_step="descents"):
            if (row["x"], row["k"]) == (3, 4):
                self.assertEqual((row["A_value"], row["Bend_value"]), (2, 2))
                self.assertEqual(row["A_details"]["status"], "applied")
                self.assertFalse(row["SumEnd"])
            if (row["x"], row["k"]) == (4, 3):
                self.assertEqual((row["A_value"], row["Bend_value"]), (2, 3))
                self.assertEqual(row["A_details"]["status"], "not needed")
                self.assertEqual(row["Bend_details"]["status"], "not needed")
                self.assertFalse(row["SumEnd"])

    def test_identity_minimality_canonicality_and_unchanged_children(self):
        for size in range(1, 25):
            for mode in lab.BASELINE_MODES:
                before = list(lab.table_rows(size, mode))
                after = list(lab.table_rows(size, mode, ghosting="when-needed", ghost_step="descents"))
                self.assertEqual(len(after), size * size)
                for old, new in zip(before, after):
                    for child in ("A", "B0", "Bend"):
                        selected = new[child + "_singlets"]
                        details = new[child + "_details"]
                        baseline, lifts = details["baseline"], details["lifts"]
                        self.assertEqual(selected, old[child + "_singlets"])
                        self.assertEqual(set(baseline), set(selected))
                        self.assertEqual(tuple(lift % size for lift in lifts), baseline)
                        self.assertTrue(all(a < b for a, b in zip(lifts, lifts[1:])))
                        sources = dict(zip(details["initial_baseline"], details["initial_lifts"]))
                        previous = -1
                        for singlet, lift in zip(baseline, lifts):
                            self.assertGreaterEqual(lift, sources[singlet])
                            self.assertEqual((lift - sources[singlet]) % size, 0)
                            # No smaller forward representative can meet both
                            # the retained source occurrence and order constraints.
                            self.assertTrue(lift - size < sources[singlet] or lift - size <= previous)
                            previous = lift
                        value = new[child + "_value"]
                        generated = tuple(baseline[i] for i in walked_traversal(len(selected), value))
                        self.assertEqual(generated, selected)
                        if old[child + "_value"] is not None:
                            self.assertEqual(value, old[child + "_value"])
                            self.assertEqual(baseline, old[child + "_baseline"])
                            self.assertEqual(lifts, details["initial_lifts"])
                            self.assertEqual(details["status"], "not needed")
                            self.assertFalse(details["moves"])
                        else:
                            self.assertEqual(details["status"], "applied")
                            self.assertTrue(details["moves"])
                        if new["x"] == 0 or len(selected) == 1:
                            self.assertEqual(value, 0)
                            self.assertEqual(details["status"], "not needed")

    def test_already_wrapped_occurrences_and_extra_turns(self):
        # A B baseline can wrap once around the root without reconstruction.
        row = next(r for r in lab.table_rows(7, ghosting="when-needed", ghost_step="descents")
                   if (r["x"], r["k"]) == (4, 4))
        details = row["Bend_details"]
        self.assertEqual(details["initial_lifts"], (5, 6, 9, 10))
        self.assertEqual(details["lifts"], (5, 6, 9, 10))
        self.assertEqual(details["moves"], ())
        # Retained source windings are lower bounds, never reduced away.
        self.assertEqual(lab.forward_lifts(7, (0, 2, 1), {0: 14, 1: 15, 2: 16}),
                         (14, 16, 22))

    def test_reference_totals_and_legacy_output(self):
        for size, canonical_count, ghost_count in ((7, 32, 34), (11, 60, 76)):
            self.assertEqual(sum(r["SumEnd"] for r in lab.table_rows(size)), canonical_count)
            self.assertEqual(sum(r["SumEnd"] for r in lab.table_rows(size, ghosting="when-needed", ghost_step="descents")),
                             ghost_count)
        self.assertEqual(sum(r["SumEnd"] for r in lab.table_rows(7, closure_mode="inherited")), 27)

    def test_wrong_sum_mode_retries_only_A_and_retains_its_current_occurrences(self):
        for size in range(1, 17):
            for mode in lab.BASELINE_MODES:
                original = list(lab.table_rows(size, mode, ghosting="when-needed", ghost_step="descents"))
                retried = list(lab.table_rows(size, mode, ghosting="wrong-sum-a", ghost_step="descents"))
                self.assertEqual(len(retried), size * size)
                for before, after in zip(original, retried):
                    expected_triggers = tuple(name for name in ("Sum0", "SumEnd") if not before[name])
                    self.assertEqual(after["A_retry_sums"], expected_triggers)
                    self.assertEqual(after["Sum0_before_A_retry"], before["Sum0"])
                    self.assertEqual(after["SumEnd_before_A_retry"], before["SumEnd"])
                    for child in ("B0", "Bend"):
                        self.assertEqual(after[child + "_details"], before[child + "_details"])
                    retry = after["A_retry"]
                    if not expected_triggers:
                        self.assertIsNone(retry)
                    else:
                        self.assertIsNotNone(retry)
                        self.assertEqual(retry["initial_baseline"], before["A_baseline"])
                        self.assertEqual(retry["initial_lifts"], before["A_details"]["lifts"])
                        self.assertEqual(retry["value_before"], before["A_value"])
                        # With the present descent rule a canonical child's
                        # value and reconstructed baseline are already fixed.
                        self.assertEqual(retry["status"], "unchanged")
                        self.assertEqual(retry["moves"], ())
                        self.assertEqual(retry["lifts"], before["A_details"]["lifts"])
                    self.assertEqual(after["A_value"], before["A_value"])
                    self.assertEqual(after["A_baseline"], before["A_baseline"])
                    self.assertEqual(after["Sum0"], before["Sum0"])
                    self.assertEqual(after["SumEnd"], before["SumEnd"])
        for size, attempts, matches in ((7, 15, 34), (11, 45, 76)):
            rows = list(lab.table_rows(size, ghosting="wrong-sum-a", ghost_step="descents"))
            self.assertEqual(sum(r["A_retry"] is not None for r in rows), attempts)
            self.assertEqual(sum(r["SumEnd"] for r in rows), matches)
        # A already ghosted in the first pass must not restart at root reps.
        example = next(r for r in lab.table_rows(7, ghosting="wrong-sum-a", ghost_step="descents")
                       if (r["x"], r["k"]) == (3, 4))
        self.assertEqual(example["A_retry"]["initial_lifts"], (0, 6, 10, 16))

    def test_explicit_retry_preserves_zero_and_singleton_information(self):
        for size in range(1, 9):
            for row in lab.table_rows(size, ghosting="when-needed", ghost_step="descents"):
                if row["x"] == 0 or row["k"] == 1:
                    details = row["A_details"]
                    retry = lab.ghost_reconstruction(lab.canonical_cycles(size, row["x"]),
                              row["A_singlets"], details["baseline"], details["lifts"])
                    self.assertEqual(retry["value"], 0)
                    self.assertEqual(retry["status"], "unchanged")
                    self.assertEqual(retry["moves"], ())

    def test_command_line_and_incompatible_mode(self):
        script = str(Path(lab.__file__).resolve())
        run = subprocess.run([sys.executable, script, "7", "--ghosting", "when-needed", "--ghost-step", "descents", "--csv"],
                             capture_output=True, text=True, check=True)
        self.assertEqual(len(run.stdout.splitlines()), 50)
        self.assertIn("A ghosts", run.stdout.splitlines()[0])
        retry = subprocess.run([sys.executable, script, "7", "--ghosting", "wrong-sum-a", "--ghost-step", "descents", "--csv"],
                               capture_output=True, text=True, check=True)
        self.assertEqual(len(retry.stdout.splitlines()), 50)
        self.assertIn("A retry", retry.stdout.splitlines()[0])
        self.assertEqual(sum(line.endswith(",unchanged") for line in retry.stdout.splitlines()), 15)
        case = subprocess.run([sys.executable, script, "7", "--ghosting", "wrong-sum-a", "--ghost-step", "descents", "--case", "4", "3"],
                              capture_output=True, text=True, check=True)
        self.assertIn("A retry for wrong sums: unchanged", case.stdout)
        self.assertIn("Additional ghost moves: none", case.stdout)
        incompatible = subprocess.run([sys.executable, script, "7", "--closure", "inherited",
                                       "--ghosting", "when-needed"], capture_output=True, text=True)
        self.assertEqual(incompatible.returncode, 2)
        self.assertIn("ghosting requires --closure canonical", incompatible.stderr)
        with self.assertRaisesRegex(ValueError, "ghosting requires canonical closure"):
            list(lab.table_rows(7, closure_mode="inherited", ghosting="when-needed", ghost_step="descents"))


class ParentInferredGhostTests(unittest.TestCase):
    def test_parent_units_reduce_to_multiplication_units(self):
        for size in range(1, 33):
            for value in range(size):
                cycles = lab.canonical_cycles(size, value)
                for child_size in range(1, size + 1):
                    if size % child_size:
                        continue
                    opposing = size // child_size
                    evidence = lab.infer_parent_step(cycles, child_size)
                    self.assertEqual(evidence["step"], value // opposing)
                    for _, _, distance, sample in evidence["measures"]:
                        self.assertEqual(sample, distance // opposing)

    def test_multiplication_example_and_already_canonical_additive_A(self):
        cycles = lab.canonical_cycles(12, 7)
        selected = (0, 7, 2)
        result = lab.analyze_child(cycles, selected, ghosting="when-needed", ghost_step="parent")
        self.assertEqual((result["value_before"], result["value"]), (2, 1))
        self.assertEqual(result["baseline"], (0, 7, 2))
        self.assertEqual(result["lifts"], (0, 7, 14))
        self.assertEqual(result["step_inference"]["step"], 1)
        row = next(r for r in lab.table_rows(7, ghosting="when-needed")
                   if (r["x"], r["k"]) == (4, 3))
        self.assertEqual((row["A_details"]["value_before"], row["A_value"]), (2, 1))
        self.assertEqual(row["A_baseline"], (0, 4, 1))
        self.assertEqual(row["A_details"]["lifts"], (0, 4, 8))
        self.assertEqual(row["Bend_value"], 3)
        self.assertTrue(row["SumEnd"])

    def test_parent_inference_can_supply_zero_cosets(self):
        row = next(r for r in lab.table_rows(7, ghosting="when-needed")
                   if (r["x"], r["k"]) == (1, 2))
        self.assertEqual(row["A_details"]["value_before"], 1)
        self.assertEqual(row["A_value"], 0)
        self.assertEqual(row["A_details"]["status"], "value changed")
        self.assertEqual(row["A_details"]["moves"], ())
        self.assertEqual(lab.canonical_cycles(2, row["A_value"]), ((0,), (1,)))
        self.assertTrue(row["SumEnd"])

    def test_inference_is_independent_of_child_baseline_and_preserves_B(self):
        for size in range(1, 17):
            for mode in lab.BASELINE_MODES:
                control = list(lab.table_rows(size, mode, ghosting="when-needed", ghost_step="descents"))
                parent = list(lab.table_rows(size, mode, ghosting="when-needed"))
                for old, row in zip(control, parent):
                    self.assertEqual(row["A_value"], row["x"] * row["k"] // size)
                    self.assertEqual(row["A_singlets"], old["A_singlets"])
                    for child in ("B0", "Bend"):
                        self.assertEqual(row[child + "_details"], old[child + "_details"])
                    details = row["A_details"]
                    self.assertEqual(details["step_source"], "parent")
                    self.assertEqual(details["step_inference"],
                                     lab.infer_parent_step(lab.canonical_cycles(size, row["x"]), row["k"]))
                    self.assertEqual(tuple(lift % size for lift in details["lifts"]), row["A_baseline"])
                    self.assertEqual(tuple(row["A_baseline"][i] for i in
                                           walked_traversal(row["k"], row["A_value"])), row["A_singlets"])
                    sources = dict(zip(details["initial_baseline"], details["initial_lifts"]))
                    previous = -1
                    for singlet, lift in zip(details["baseline"], details["lifts"]):
                        self.assertGreater(lift, previous)
                        self.assertGreaterEqual(lift, sources[singlet])
                        self.assertTrue(lift - size < sources[singlet] or lift - size <= previous)
                        previous = lift

    def test_parent_retry_retains_prior_ghosts_and_counts_new_moves(self):
        row = next(r for r in lab.table_rows(7, ghosting="wrong-sum-a")
                   if (r["x"], r["k"]) == (3, 4))
        retry = row["A_retry"]
        self.assertEqual(retry["initial_lifts"], (0, 6, 10, 16))
        self.assertEqual(retry["lifts"], (0, 10, 13, 16))
        self.assertEqual((retry["value_before"], retry["value"]), (2, 1))
        self.assertEqual(retry["moves"], ((6, 6, 13),))
        self.assertTrue(row["SumEnd"])
        script = str(Path(lab.__file__).resolve())
        run = subprocess.run([sys.executable, script, "7", "--ghosting", "wrong-sum-a", "--csv"],
                             capture_output=True, text=True, check=True)
        rows = list(csv.DictReader(io.StringIO(run.stdout)))
        self.assertEqual(len(rows), 49)
        displayed = next(r for r in rows if (r["Parent"], r["k"]) == ("*7+4", "3"))
        self.assertEqual(displayed["A ghosts"], "1")
        self.assertEqual(displayed["A retry"], "applied")
        self.assertEqual(displayed["A"], "*3+1")

    def test_prime_results_and_composite_boundary(self):
        for size in (7, 11):
            for mode in ("when-needed", "wrong-sum-a"):
                rows = list(lab.table_rows(size, ghosting=mode))
                self.assertEqual(sum(r["SumEnd"] for r in rows), size * size)
                self.assertEqual(sum(r["Sum0"] for r in rows), size * size)
        # The A-only experimental inference has not resolved all composite
        # end-overlap cases. Keep these failures visible rather than force B.
        for size, expected in ((6, 32), (12, 127)):
            self.assertEqual(sum(r["SumEnd"] for r in lab.table_rows(size, ghosting="when-needed")), expected)
        script = str(Path(lab.__file__).resolve())
        run = subprocess.run([sys.executable, script, "7", "--ghosting", "when-needed", "--case", "4", "3"],
                             capture_output=True, text=True, check=True)
        self.assertIn("A step rule: parent-scaled-spans (3/7)", run.stdout)
        self.assertIn("Inferred parent step:", run.stdout)


if __name__ == "__main__":
    unittest.main(verbosity=2)
