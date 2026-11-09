#!/usr/bin/env python3
"""Reproduce the later additive selection-rule experiments.

Reconstructed on 2026-11-01 from the recorded algorithms, example cases, and
result counts in "Codex Additive Decomposition". The original attachment was unavailable;
this is a verified replacement rather than a byte-for-byte recovered file.

This LATER model checks the child's entire canonical flattened traversal in
its inherited cyclic baseline, allowing its own canonical coset closures.
The original infon_additive_prefix_table.py instead preserves the parent's
induced successor map. Consequently their success counts differ: at n=7,
the original strict test accepts 27 sums, and the later fixed-prefix test 32.
Neither model is a proof that a decomposition is infonically permitted.

All choices retain parent traversal order. A includes position 0; B shares
A's last position unless the rule explicitly says to share position 0.
B's remaining singlets follow cyclic parent traversal from the shared one.
This reconstruction defines minimum displacement as sum(position_i - i),
breaking ties lexicographically by A's position tuple, without consulting the
desired sum. The original implementation's exact tie-breaking was not
available; this explicit convention reproduces all published reference totals.

Python 3.8+; standard library only. Examples:
    python3 infon_additive_rule_trials.py 7
    python3 infon_additive_rule_trials.py 7 --case 5 3
    python3 infon_additive_rule_trials.py 7 --case 5 5
    python3 infon_additive_rule_trials.py --self-test

The subset searches grow exponentially with n; the original reference runs
use n=7 and n=11.
"""

import argparse
from functools import lru_cache
from itertools import combinations
from math import gcd


@lru_cache(maxsize=None)
def canonical_traversal(size, value):
    count = gcd(size, value)
    return tuple((start + j * value) % size
                 for start in range(count) for j in range(size // count))


def local_value(selected, parent_is_zero):
    """Identify the whole flattened canonical pattern, not only descents."""
    size = len(selected)
    if parent_is_zero or size == 1:
        return 0
    baseline = sorted(selected)
    anchor = baseline.index(selected[0])
    ranks = {s: (i - anchor) % size for i, s in enumerate(baseline)}
    normalized = tuple(ranks[s] for s in selected)
    # Zero and one share a flattened pattern. Only zero-valued parents retain
    # zero-coset information here; positive nonsingleton children use v > 0.
    return next((v for v in range(1, size)
                 if canonical_traversal(size, v) == normalized), None)


def evaluate(size, value, positions, shared=None):
    parent = canonical_traversal(size, value)
    if shared is None:
        shared = positions[-1]
    chosen = set(positions)
    a = tuple(parent[i] for i in positions)
    b = (parent[shared],) + tuple(
        parent[i % size] for i in range(shared + 1, shared + size)
        if i % size not in chosen)
    av, bv = local_value(a, value == 0), local_value(b, value == 0)
    canonical = av is not None and bv is not None
    return {"positions": positions, "A": a, "B": b, "av": av, "bv": bv,
            "canonical": canonical, "success": canonical and av + bv == value,
            "displacement": sum(pos - i for i, pos in enumerate(positions))}


RULES = ("Prefix", "Share0", "Baseline", "Even", "Stride", "Any", "Minimum")


def trial(size, value, k):
    parent = canonical_traversal(size, value)
    prefix = tuple(range(k))
    baseline = tuple(sorted(parent.index(s) for s in range(k)))
    even = tuple(j * (size - 1) // (k - 1) for j in range(k)) if k > 1 else (0,)
    stride_value = (size - k + 1) % size
    stride = tuple(sorted(canonical_traversal(size, stride_value)[:k]))
    results = {
        "Prefix": evaluate(size, value, prefix),
        "Share0": evaluate(size, value, prefix, shared=0),
        "Baseline": evaluate(size, value, baseline),
        "Even": evaluate(size, value, even),
        "Stride": evaluate(size, value, stride),
    }
    candidates = []
    for suffix in combinations(range(1, size), k - 1):
        result = evaluate(size, value, (0,) + suffix)
        if result["canonical"]:
            candidates.append(result)
    candidates.sort(key=lambda r: (r["displacement"], r["positions"]))
    # 'Any' is an existence test using the desired sum, not a selection rule.
    results["Any"] = next((r for r in candidates if r["success"]), None)
    results["Minimum"] = candidates[0] if candidates else None
    return results, candidates


def success(result):
    return bool(result and result["success"])


def describe(result):
    if result is None:
        return "no canonical pair"
    av = "-" if result["av"] is None else "*{}+{}".format(len(result["A"]), result["av"])
    bv = "-" if result["bv"] is None else "*{}+{}".format(len(result["B"]), result["bv"])
    return "A={} {}  B={} {}  sum={}  positions={}  displacement={}".format(
        result["A"], av, result["B"], bv,
        "yes" if result["success"] else "no", result["positions"], result["displacement"])


def counts(size):
    totals = {rule: 0 for rule in RULES}
    for value in range(size):
        for k in range(1, size + 1):
            results, _ = trial(size, value, k)
            for rule in RULES:
                totals[rule] += success(results[rule])
    return totals


def self_test():
    # Independent reference totals from the conversation's published tables.
    for size, reference in ((7, (32, 29, 24, 24, 25, 37, 33)),
                            (11, (60, 50, 40, 40, 42, 74, 69))):
        actual = counts(size)
        assert tuple(actual[r] for r in RULES) == reference, actual
    results, candidates = trial(7, 5, 3)
    best = results["Minimum"]
    assert (best["av"], best["bv"], best["success"]) == (1, 3, False)
    # The prose's equally small changes replace the same number of singlets;
    # their summed position displacement can differ.
    replacements = sum(p >= 3 for p in best["positions"])
    assert any(sum(p >= 3 for p in r["positions"]) == replacements and r["success"]
               and (r["av"], r["bv"]) == (2, 3) for r in candidates)
    results, _ = trial(7, 5, 5)
    best = results["Minimum"]
    assert best["A"] == (0, 5, 3, 6, 4)
    assert (best["av"], best["bv"], best["success"]) == (3, 2, True)
    print("PASS: all seven n=7 and n=11 reference totals; both *7+5 examples.")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("n", nargs="?", type=int, default=7)
    parser.add_argument("--case", nargs=2, type=int, metavar=("X", "K"))
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.n < 1:
        parser.error("n must be a positive integer")
    if args.self_test:
        self_test()
        return
    if args.case:
        value, k = args.case
        if not (0 <= value < args.n and 1 <= k <= args.n):
            parser.error("require 0 <= X < n and 1 <= K <= n")
        results, candidates = trial(args.n, value, k)
        print("Parent *{}+{}, A size {}".format(args.n, value, k))
        print("Parent traversal:", canonical_traversal(args.n, value))
        for rule in RULES:
            print("{}: {}".format(rule, describe(results[rule])))
        if candidates:
            minimum = candidates[0]["displacement"]
            print("All canonical pairs tied at the minimum displacement:")
            for candidate in candidates:
                if candidate["displacement"] == minimum:
                    print(" ", describe(candidate))
        return
    totals = {rule: 0 for rule in RULES}
    print("Parent k " + " ".join(RULES))
    for value in range(args.n):
        for k in range(1, args.n + 1):
            results, _ = trial(args.n, value, k)
            flags = [success(results[rule]) for rule in RULES]
            print("*{}+{} {} {}".format(args.n, value, k,
                  " ".join("yes" if flag else "no" for flag in flags)))
            for rule, flag in zip(RULES, flags):
                totals[rule] += flag
    print("Successful sums out of {} rows:".format(args.n * args.n))
    for rule in RULES:
        print("{}: {}".format(rule, totals[rule]))


if __name__ == "__main__":
    main()
