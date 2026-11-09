#!/usr/bin/env python3
"""Print the n-squared-row additive prefix compatibility table.

Reconstructed on 2026-11-01 from the specification and 49 reference rows in
the ChatGPT conversation "Codex Additive Decomposition". The original attachment
could not be downloaded. This is a verified replacement, not a byte-for-byte recovery.

For every parent *n+x, 0 <= x < n, and prefix length 1 <= k <= n:
  A    = the first k singlets in the parent's canonical coset traversal;
  B0   = singlet 0 followed by the unselected suffix;
  Bend = A's last singlet followed by the unselected suffix.

Baseline choices (--baseline):
  inherited: restrict the parent's baseline to the selected singlets (default);
  traversal: use the child's selected traversal as its local baseline.
Both baselines start at the child's first selected singlet.

Closure choices (--closure), independently of baseline choice:
  inherited: skip excluded singlets WITHIN each parent coset;
  canonical: permit the child's own canonical coset closures (default), as in the later
             selection-rule experiment, matching the whole flattened traversal.

Optional ghosting requires canonical closure. A's inference rule is selected
with --ghost-step:
  parent (default): infer A's step from the PARENT's baseline transitions,
      taking the modal floor(distance * A_size / parent_size). When the parent
      size is A_size * opposing_size, this is exactly multiplication's modal
      floor(distance / opposing_size). Extending it to additive cuts is an
      explicit experimental hypothesis, not a derived addition theorem.
  descents: retain the previous inference from A's current cyclic baseline.

--ghosting when-needed with parent inference determines A's step BEFORE its
local canonical test can freeze a value. It constructs the canonical baseline
for that inferred step, then realizes it with minimal forward lifts identity +
turns * PARENT size. An inferred step of zero supplies explicit zero-coset
information, including for a positive-valued parent.
B0 and Bend retain the previous canonical/descent calculation so this experiment
isolates the change to A. With descents, canonical children stay unchanged.

--ghosting wrong-sum-a first runs the old when-needed pass, then retries A on
either failed sum using the selected inference rule and its retained lifts.
Only the decision to retry uses the sum; neither inference rule receives B's
value or the desired total. Neither searches values for a matching sum.
Ghost counts report additional shifts beyond the initial cyclic baseline;
an ordinary wrap at the baseline anchor is not an additional ghost move.
Both inference rules are experimental. Forward ghost lifts
preserve identities but do not by themselves establish a unique local value.
Coset closures are checked, and zero is distinguished from one.
Canonicality and additive value agreement are separate tests. These checks
are experiments, not proofs that a decomposition is infonically permitted.

Python 3.8+; standard library only. Examples:
    python3 infon_additive_prefix_table.py 7
    python3 infon_additive_prefix_table.py 7 --csv
    python3 infon_additive_prefix_table.py 7 --baseline traversal --case 4 4
    python3 infon_additive_prefix_table.py 7 --closure canonical --case 4 4
    python3 infon_additive_prefix_table.py 7 --ghosting when-needed
    python3 infon_additive_prefix_table.py 7 --ghosting when-needed --case 5 5
    python3 infon_additive_prefix_table.py 7 --ghosting when-needed --case 4 3
    python3 infon_additive_prefix_table.py 7 --ghosting when-needed --ghost-step descents
    python3 infon_additive_prefix_table.py 7 --ghosting wrong-sum-a
    python3 infon_additive_prefix_table.py 7 --ghosting wrong-sum-a --case 4 3
    python3 infon_additive_prefix_table.py 7 --closure inherited
    python3 infon_additive_prefix_table.py 7 --show-baselines
    python3 infon_additive_prefix_table.py --self-test
"""

import argparse
import csv
from math import gcd
import sys


BASELINE_MODES = ("inherited", "traversal")
CLOSURE_MODES = ("inherited", "canonical")
GHOSTING_MODES = ("off", "when-needed", "wrong-sum-a")
GHOST_STEP_RULES = ("parent", "descents")


def child_baseline(selected, baseline_mode="inherited"):
    """Construct a baseline without using the desired additive value."""
    selected = tuple(selected)
    if not selected or len(set(selected)) != len(selected):
        raise ValueError("a child must contain distinct singlets and be nonempty")
    if baseline_mode == "traversal":
        return selected
    if baseline_mode != "inherited":
        raise ValueError("unknown baseline mode: " + baseline_mode)
    baseline = sorted(selected)
    anchor = baseline.index(selected[0])
    return tuple(baseline[anchor:] + baseline[:anchor])


def canonical_cycles(size, value):
    """Return the separate canonical cycles, starting with baseline 0."""
    if size < 1 or not 0 <= value < size:
        raise ValueError("require size >= 1 and 0 <= value < size")
    count = gcd(size, value)
    return tuple(
        tuple((start + j * value) % size for j in range(size // count))
        for start in range(count)
    )


def canonical_traversal(size, value):
    return tuple(s for cycle in canonical_cycles(size, value) for s in cycle)


def induced_successors(parent_cycles, selected):
    """First return to selected singlets along each original parent cycle."""
    chosen = set(selected)
    successors = {}
    for cycle in parent_cycles:
        kept = [s for s in cycle if s in chosen]
        for i, singlet in enumerate(kept):
            successors[singlet] = kept[(i + 1) % len(kept)]
    if set(successors) != chosen:
        raise ValueError("selected singlet is absent from the parent")
    return successors


def value_on_baseline(parent_cycles, selected, baseline, closure_mode="canonical"):
    """Classify a specified baseline without changing identities or its order."""
    selected = tuple(selected)
    if not selected or len(set(selected)) != len(selected) or set(baseline) != set(selected):
        raise ValueError("baseline must contain each selected singlet exactly once")
    if len(baseline) != len(selected):
        raise ValueError("baseline must contain each selected singlet exactly once")
    rank = {s: i for i, s in enumerate(baseline)}
    if closure_mode == "canonical":
        if len(selected) == 1 or all(len(cycle) == 1 for cycle in parent_cycles):
            return 0
        normalized = tuple(rank[s] for s in selected)
        return next((v for v in range(1, len(selected))
                     if canonical_traversal(len(selected), v) == normalized), None)
    if closure_mode != "inherited":
        raise ValueError("unknown closure mode: " + closure_mode)
    successors = induced_successors(parent_cycles, selected)
    steps = {
        (rank[successors[s]] - rank[s]) % len(selected) for s in selected
    }
    return steps.pop() if len(steps) == 1 else None


def canonical_child_value(parent_cycles, selected, baseline_mode="inherited",
                          closure_mode="canonical"):
    """Classify the child under independently selected baseline/closure rules."""
    return value_on_baseline(parent_cycles, selected,
                             child_baseline(selected, baseline_mode), closure_mode)


def forward_lifts(parent_size, baseline, source_lifts=None):
    """Minimal strictly increasing lifts, never moving a source occurrence back.

    With no supplied occurrences, use each identity's root representative as
    its lower bound. A supplied mapping retains any already accumulated turns.
    Every output is congruent to its identity modulo the PARENT size.
    """
    if parent_size < 1:
        raise ValueError("parent size must be positive")
    previous = -1
    lifts = []
    for singlet in baseline:
        if not 0 <= singlet < parent_size:
            raise ValueError("singlet is outside the parent")
        lift = singlet if source_lifts is None else source_lifts[singlet]
        if lift < 0 or lift % parent_size != singlet:
            raise ValueError("source lift does not preserve its parent identity")
        if lift <= previous:
            lift += ((previous - lift) // parent_size + 1) * parent_size
        lifts.append(lift)
        previous = lift
    return tuple(lifts)


def infer_parent_step(parent_cycles, child_size):
    """Infer a child-scale phase step without looking at a child baseline.

    Samples and the running modal vote follow the multiplication applet's
    parentTransitionMeasures/modeOf pattern. Scaling by child_size/parent_size
    extends its quotient-span units to sizes that do not divide the parent.
    Known zero-coset parents retain zero; their flattened inter-coset ordering
    does not represent a phase advance.
    """
    parent = tuple(s for cycle in parent_cycles for s in cycle)
    parent_size = len(parent)
    if not 1 <= child_size <= parent_size:
        raise ValueError("child size must be between one and parent size")
    zero_parent = all(len(cycle) == 1 for cycle in parent_cycles)
    measures = () if zero_parent else tuple(
        (a, b, (b - a) % parent_size, ((b - a) % parent_size) * child_size // parent_size)
        for a, b in zip(parent, parent[1:]))
    counts = {}
    step, best_count = 0, 0
    for _, _, _, sample in measures:
        counts[sample] = counts.get(sample, 0) + 1
        # This reproduces modeOf's first-to-reach-the-winning-count tie rule.
        if counts[sample] > best_count:
            step, best_count = sample, counts[sample]
    return {"rule": "parent-scaled-spans", "parent_size": parent_size,
            "child_size": child_size, "zero_parent": zero_parent,
            "measures": measures, "counts": tuple(counts.items()), "step": step}


def analyze_child(parent_cycles, selected, baseline_mode="inherited",
                   closure_mode="canonical", ghosting="off", ghost_step="descents"):
    """Reconstruct from an independently inferred step, or use legacy descents."""
    if ghosting not in GHOSTING_MODES:
        raise ValueError("unknown ghosting mode: " + ghosting)
    if ghosting != "off" and closure_mode != "canonical":
        raise ValueError("ghosting requires canonical closure")
    if ghost_step not in GHOST_STEP_RULES:
        raise ValueError("unknown ghost step rule: " + ghost_step)
    selected = tuple(selected)
    # Parent inference happens before choosing/classifying any child baseline.
    inference = infer_parent_step(parent_cycles, len(selected)) if (
        ghosting != "off" and ghost_step == "parent") else None
    parent_size = sum(map(len, parent_cycles))
    initial = child_baseline(selected, baseline_mode)
    before = value_on_baseline(parent_cycles, selected, initial, closure_mode)
    initial_lifts = forward_lifts(parent_size, initial) if ghosting != "off" else ()
    if inference is not None:
        return place_child_for_step(parent_cycles, selected, initial, initial_lifts,
                                    inference["step"], "parent", inference)
    result = {
        "initial_baseline": initial, "baseline": initial,
        "value_before": before, "value": before, "inferred_value": None,
        "status": "off" if ghosting == "off" else "not needed",
        "initial_lifts": initial_lifts, "lifts": initial_lifts, "moves": (),
    }
    if ghosting == "off" or before is not None:
        return result
    return ghost_reconstruction(parent_cycles, selected, initial, initial_lifts)


def ghost_reconstruction(parent_cycles, selected, initial, initial_lifts):
    """Try reconstruction, including for canonical children, retaining lifts.

    This function receives neither the opposing child's value nor the desired
    sum. Callers can use it for a failed-sum retry without selecting a value
    to force that sum to pass.
    """
    selected, initial, initial_lifts = tuple(selected), tuple(initial), tuple(initial_lifts)
    if len(initial_lifts) != len(initial):
        raise ValueError("each baseline identity needs a retained source lift")
    # Infer locally, before constructing a new baseline. No opposing child or
    # desired total is available to this function.
    ranks = {s: i for i, s in enumerate(initial)}
    if len(selected) == 1 or all(len(cycle) == 1 for cycle in parent_cycles):
        candidate = 0
    else:
        candidate = sum(ranks[selected[i]] > ranks[selected[(i + 1) % len(selected)]]
                        for i in range(len(selected)))
    return place_child_for_step(parent_cycles, selected, initial, initial_lifts,
                                candidate, "descents")


def place_child_for_step(parent_cycles, selected, initial, initial_lifts,
                         candidate, step_source, inference=None):
    """Construct baseline/cosets, then ghost lifts, for an already inferred step.

    In particular, step zero carries zero-coset information from its inference
    source; a flattened traversal alone cannot distinguish zero from one.
    """
    selected, initial, initial_lifts = tuple(selected), tuple(initial), tuple(initial_lifts)
    if not 0 <= candidate < len(selected):
        raise ValueError("inferred step must be within the child's size")
    if len(initial_lifts) != len(initial):
        raise ValueError("each baseline identity needs a retained source lift")
    parent_size = sum(map(len, parent_cycles))
    before = value_on_baseline(parent_cycles, selected, initial, "canonical")
    baseline = [None] * len(selected)
    for singlet, slot in zip(selected, canonical_traversal(len(selected), candidate)):
        baseline[slot] = singlet
    baseline = tuple(baseline)
    sources = dict(zip(initial, initial_lifts))
    lifts = forward_lifts(parent_size, baseline, sources)
    generated = tuple(baseline[i] for i in canonical_traversal(len(selected), candidate))
    if generated != selected:
        raise RuntimeError("ghost reconstruction did not reproduce the selected traversal")
    moves = tuple((s, sources[s], lift) for s, lift in zip(baseline, lifts)
                  if lift != sources[s])
    return {
        "initial_baseline": initial, "baseline": baseline,
        "value_before": before, "value": candidate, "inferred_value": candidate,
        "step_source": step_source, "step_inference": inference,
        "status": "applied" if moves else ("value changed" if before != candidate else "unchanged"),
        "initial_lifts": initial_lifts, "lifts": lifts, "moves": moves,
    }


def table_rows(size, baseline_mode="inherited", closure_mode="canonical", ghosting="off",
               ghost_step="parent"):
    if size < 1:
        raise ValueError("size must be a positive integer")
    if baseline_mode not in BASELINE_MODES or closure_mode not in CLOSURE_MODES:
        raise ValueError("unknown baseline or closure mode")
    if ghosting not in GHOSTING_MODES:
        raise ValueError("unknown ghosting mode: " + ghosting)
    if ghosting != "off" and closure_mode != "canonical":
        raise ValueError("ghosting requires canonical closure")
    if ghost_step not in GHOST_STEP_RULES:
        raise ValueError("unknown ghost step rule: " + ghost_step)
    for value in range(size):
        cycles = canonical_cycles(size, value)
        parent = tuple(s for cycle in cycles for s in cycle)
        for k in range(1, size + 1):
            a = parent[:k]
            b0 = (parent[0],) + parent[k:]
            bend = parent[k - 1:]
            # B is the control. In wrong-sum-a mode, the initial A pass is
            # also the legacy pass; only its retry uses the requested rule.
            children = {child: analyze_child(cycles, selected, baseline_mode, closure_mode, ghosting,
                         ghost_step if child == "A" and ghosting != "wrong-sum-a" else "descents")
                        for child, selected in (("A", a), ("B0", b0), ("Bend", bend))}
            av, b0v, bev = (children[c]["value"] for c in ("A", "B0", "Bend"))
            sum0_before = av is not None and b0v is not None and av + b0v == value
            sumend_before = av is not None and bev is not None and av + bev == value
            retry = None
            retry_sums = ()
            if ghosting == "wrong-sum-a" and not (sum0_before and sumend_before):
                retry_sums = tuple(name for name, correct in
                                   (("Sum0", sum0_before), ("SumEnd", sumend_before)) if not correct)
                if ghost_step == "parent":
                    inference = infer_parent_step(cycles, len(a))
                    retry = place_child_for_step(cycles, a, children["A"]["baseline"],
                              children["A"]["lifts"], inference["step"], "parent", inference)
                else:
                    retry = ghost_reconstruction(cycles, a, children["A"]["baseline"], children["A"]["lifts"])
                av = retry["value"]
            row = {
                "n": size, "x": value, "k": k,
                "baseline_mode": baseline_mode, "closure_mode": closure_mode,
                "ghosting_mode": ghosting,
                "ghost_step": ghost_step,
                "A_singlets": a, "B0_singlets": b0, "Bend_singlets": bend,
                "A_value": av, "B0_value": b0v, "Bend_value": bev,
                "A_retry": retry, "A_retry_sums": retry_sums,
                "Sum0_before_A_retry": sum0_before, "SumEnd_before_A_retry": sumend_before,
                "Sum0": av is not None and b0v is not None and av + b0v == value,
                "SumEnd": av is not None and bev is not None and av + bev == value,
            }
            for child, details in children.items():
                row[child + "_baseline"] = details["baseline"]
                row[child + "_details"] = details
            if retry is not None:
                row["A_baseline"] = retry["baseline"]
            yield row


def infon(size, value):
    return "-" if value is None else "*{}+{}".format(size, value)


def display_rows(size, baseline_mode="inherited", closure_mode="canonical",
                 show_baselines=False, ghosting="off", ghost_step="parent"):
    for row in table_rows(size, baseline_mode, closure_mode, ghosting, ghost_step):
        cells = [
            infon(size, row["x"]), str(row["k"]),
            ",".join(map(str, row["A_singlets"])),
            infon(row["k"], row["A_value"]),
            infon(size - row["k"] + 1, row["B0_value"]),
            infon(size - row["k"] + 1, row["Bend_value"]),
            "yes" if row["Sum0"] else "no",
            "yes" if row["SumEnd"] else "no",
        ]
        if ghosting != "off":
            for child in ("A", "B0", "Bend"):
                moved = {move[0] for move in row[child + "_details"]["moves"]}
                if child == "A" and row["A_retry"]:
                    moved.update(move[0] for move in row["A_retry"]["moves"])
                cells.append(str(len(moved)))
        if ghosting == "wrong-sum-a":
            cells.append(row["A_retry"]["status"] if row["A_retry"] else "not needed")
        if show_baselines:
            cells.extend([baseline_mode, closure_mode] + [
                ",".join(map(str, row[child + "_baseline"]))
                for child in ("A", "B0", "Bend")])
            if ghosting != "off":
                cells.extend(",".join(map(str, row[child + "_details"]["initial_baseline"]))
                             for child in ("A", "B0", "Bend"))
        yield cells


HEADERS = ["Parent", "k", "A singlets", "A", "B0", "Bend", "Sum0", "SumEnd"]


def print_table(size, as_csv=False, baseline_mode="inherited",
                closure_mode="canonical", show_baselines=False, ghosting="off", ghost_step="parent"):
    rows = list(display_rows(size, baseline_mode, closure_mode, show_baselines, ghosting, ghost_step))
    headers = HEADERS + (["A ghosts", "B0 ghosts", "Bend ghosts"] if ghosting != "off" else [])
    if ghosting == "wrong-sum-a":
        headers.append("A retry")
    headers += (["Baseline mode", "Closure mode", "A baseline",
                          "B0 baseline", "Bend baseline"] if show_baselines else [])
    if show_baselines and ghosting != "off":
        headers += ["A initial baseline", "B0 initial baseline", "Bend initial baseline"]
    if as_csv:
        writer = csv.writer(sys.stdout)
        writer.writerow(headers)
        writer.writerows(rows)
        return
    widths = [max(len(r[i]) for r in [headers] + rows) for i in range(len(headers))]
    for row in [headers] + rows:
        print("  ".join(s.ljust(w) for s, w in zip(row, widths)).rstrip())


def print_parent_inference(details, prefix="  "):
    evidence = details.get("step_inference")
    if evidence is None:
        return
    print(prefix + "A step rule: parent-scaled-spans ({}/{})".format(
        evidence["child_size"], evidence["parent_size"]))
    if evidence["zero_parent"]:
        print(prefix + "Parent has known zero cosets; step = 0.")
    else:
        print(prefix + "Parent baseline distances:", tuple(m[2] for m in evidence["measures"]))
        print(prefix + "Scaled step samples:      ", tuple(m[3] for m in evidence["measures"]))
        print(prefix + "Inferred parent step:     ", evidence["step"])
    if evidence["step"] == 0:
        print(prefix + "The inferred step supplies zero-coset information.")


def print_case(size, value, k, baseline_mode="inherited", closure_mode="canonical", ghosting="off",
                ghost_step="parent"):
    row = next(r for r in table_rows(size, baseline_mode, closure_mode, ghosting, ghost_step)
               if r["x"] == value and r["k"] == k)
    cycles = canonical_cycles(size, value)
    print("Parent *{}+{}, A size {}".format(size, value, k))
    print("Parent traversal:", canonical_traversal(size, value))
    print("Baseline rule: {}; closure rule: {}; ghosting: {}".format(
        baseline_mode, closure_mode, ghosting))
    if ghosting != "off" and ghost_step == "parent":
        print("A inference: parent transitions; B inference: canonical/descents (control).")
    for child in ("A", "B0", "Bend"):
        selected, baseline = row[child + "_singlets"], row[child + "_baseline"]
        result = row[child + "_value"]
        print("{}: {}".format(child, infon(len(selected), result)))
        print("  Selected singlets:", selected)
        print("  Local baseline:   ", baseline)
        if ghosting != "off":
            details = row[child + "_details"]
            print("  Ghosting:         ", details["status"])
            print("  Value before:     ", infon(len(selected), details["value_before"]))
            if details["inferred_value"] is not None:
                if details.get("step_source") == "parent":
                    print_parent_inference(details)
                else:
                    print("  Descent value:    ", details["inferred_value"])
            print("  Initial baseline: ", details["initial_baseline"])
            print("  Initial lifts:    ", details["initial_lifts"])
            print("  Final lifts:      ", details["lifts"])
            print("  Root identities:  ", tuple(lift % size for lift in details["lifts"]))
            for singlet, source, target in details["moves"]:
                print("  Ghost {}: {} -> {} (+{} parent cycles)".format(
                    singlet, source, target, (target - source) // size))
            if child == "A" and ghosting == "wrong-sum-a":
                retry = row["A_retry"]
                print("  A retry for wrong sums:", retry["status"] if retry else "not needed")
                if retry is not None:
                    print_parent_inference(retry, "  Retry ")
                    print("  Triggered by:     ", ", ".join(row["A_retry_sums"]))
                    print("  Retry value:      ", "{} -> {}".format(
                        infon(len(selected), retry["value_before"]), infon(len(selected), retry["value"])))
                    print("  Retry baseline:   ", retry["initial_baseline"], "->", retry["baseline"])
                    print("  Retry lifts:      ", retry["initial_lifts"], "->", retry["lifts"])
                    if not retry["moves"]:
                        print("  Additional ghost moves: none")
                    for singlet, source, target in retry["moves"]:
                        print("  Retry ghost {}: {} -> {} (+{} parent cycles)".format(
                            singlet, source, target, (target - source) // size))
        if closure_mode == "inherited":
            successors = induced_successors(cycles, selected)
        elif result is not None:
            successors = {s: baseline[(i + result) % len(baseline)]
                          for i, s in enumerate(baseline)}
        else:
            print("  No canonical traversal matches this baseline.")
            continue
        rank = {s: i for i, s in enumerate(baseline)}
        print("  Phase edges:      ", ", ".join("{}->{}".format(s, successors[s])
                                                        for s in selected))
        print("  Local advances:   ", tuple((rank[successors[s]] - rank[s]) % len(selected)
                                              for s in selected))
    print("Values add: B0={}; Bend={}".format(
        "yes" if row["Sum0"] else "no", "yes" if row["SumEnd"] else "no"))


def self_test():
    # Frozen from the original conversation's 49-row table, not generated here.
    # Each token is A/B0/Bend; '-' means the original table's dash.
    reference = (
        "000 000 000 000 000 000 000",
        "011 111 111 111 111 111 100",
        "022 1-- 1-- 111 -11 -11 200",
        "033 1-- 122 --- 211 -11 300",
        "044 1-- 233 --- 322 -11 400",
        "055 1-- 2-- 333 -22 -11 500",
        "066 155 244 333 422 511 600",
    )
    rows = list(table_rows(7, closure_mode="inherited"))
    expected = [tuple(None if c == "-" else int(c) for c in token)
                for line in reference for token in line.split()]
    assert len(rows) == 49
    for row, triple in zip(rows, expected):
        actual = tuple(row[key] for key in ("A_value", "B0_value", "Bend_value"))
        assert actual == triple, (row["x"], row["k"], actual, triple)
    assert sum(row["Sum0"] for row in rows) == 27
    assert sum(row["SumEnd"] for row in rows) == 27
    assert sum(all(row[key] is not None for key in
                   ("A_value", "B0_value", "Bend_value")) for row in rows) == 35

    # Independent successor walking checks the optimized cycle restriction.
    for size in range(1, 35):
        count = 0
        for row in table_rows(size, closure_mode="inherited"):
            count += 1
            value = row["x"]
            cycles = canonical_cycles(size, value)
            for field, value_field in (("A_singlets", "A_value"),
                                       ("B0_singlets", "B0_value"),
                                       ("Bend_singlets", "Bend_value")):
                selected = row[field]
                chosen = set(selected)
                walked = {}
                for s in selected:
                    target = (s + value) % size
                    while target not in chosen:
                        target = (target + value) % size
                    walked[s] = target
                assert induced_successors(cycles, selected) == walked
                baseline = sorted(selected)
                candidates = [v for v in range(len(selected)) if all(
                    walked[s] == baseline[(i + v) % len(selected)]
                    for i, s in enumerate(baseline))]
                assert candidates == ([] if row[value_field] is None else [row[value_field]])
                assert row["k"] + len(row["B0_singlets"]) - 1 == size
                assert len(set(row["A_singlets"]) & set(row["B0_singlets"])) == 1
                assert len(set(row["A_singlets"]) & set(row["Bend_singlets"])) == 1
        assert count == size * size
    # The motivating example distinguishes baseline choice from coset closure.
    for baseline_mode, closure_mode, expected in (
            ("inherited", "inherited", None), ("traversal", "inherited", 1),
            ("inherited", "canonical", 2), ("traversal", "canonical", 1)):
        row = next(r for r in table_rows(7, baseline_mode, closure_mode)
                   if r["x"] == 4 and r["k"] == 4)
        assert row["A_value"] == row["Bend_value"] == expected
        assert row["SumEnd"] == (expected == 2)
    assert child_baseline((5, 2, 6, 3)) == (5, 6, 2, 3)
    assert child_baseline((5, 2, 6, 3), "traversal") == (5, 2, 6, 3)
    # Traversal baselines preserve the distinction between zero and one, and
    # changing a baseline does not silently merge inherited parent cosets.
    for baseline_mode in BASELINE_MODES:
        for closure_mode in CLOSURE_MODES:
            assert all(r["A_value"] == r["Bend_value"] == 0
                       for r in table_rows(7, baseline_mode, closure_mode) if r["x"] == 0)
    composite = next(r for r in table_rows(6, "traversal", "inherited")
                     if r["x"] == 2 and r["k"] == 6)
    assert composite["A_value"] is None
    # Canonical is the new default. Ghosting keeps the known *7+4 case intact,
    # repairs the *7+5 five-state child, and exposes the *7+3 sum mismatch.
    canonical = list(table_rows(7))
    assert sum(r["SumEnd"] for r in canonical) == 32
    ghosted = list(table_rows(7, ghosting="when-needed", ghost_step="descents"))
    assert len(ghosted) == 49
    assert sum(r["SumEnd"] for r in ghosted) == 34
    repaired = next(r for r in ghosted if (r["x"], r["k"]) == (5, 5))
    assert repaired["A_value"] == 3 and repaired["Bend_value"] == 2 and repaired["SumEnd"]
    assert repaired["A_baseline"] == (0, 3, 6, 5, 1)
    assert repaired["A_details"]["lifts"] == (0, 3, 6, 12, 15)
    unchanged = next(r for r in ghosted if (r["x"], r["k"]) == (4, 4))
    assert unchanged["A_value"] == unchanged["Bend_value"] == 2
    assert unchanged["A_details"]["status"] == unchanged["Bend_details"]["status"] == "not needed"
    mismatch = next(r for r in ghosted if (r["x"], r["k"]) == (3, 4))
    assert mismatch["A_value"] == mismatch["Bend_value"] == 2 and not mismatch["SumEnd"]
    retried = list(table_rows(7, ghosting="wrong-sum-a", ghost_step="descents"))
    assert sum(r["A_retry"] is not None for r in retried) == 15
    assert all(r["A_retry"]["status"] == "unchanged" for r in retried if r["A_retry"])
    assert sum(r["SumEnd"] for r in retried) == 34
    parent_rows = list(table_rows(7, ghosting="when-needed"))
    assert sum(r["SumEnd"] for r in parent_rows) == 49
    changed = next(r for r in parent_rows if (r["x"], r["k"]) == (4, 3))
    assert changed["A_details"]["value_before"] == 2 and changed["A_value"] == 1
    assert changed["A_baseline"] == (0, 4, 1)
    assert changed["A_details"]["lifts"] == (0, 4, 8)
    parent_retry = list(table_rows(7, ghosting="wrong-sum-a"))
    assert sum(r["SumEnd"] for r in parent_retry) == 49
    assert sum(r["A_retry"] is not None for r in parent_retry) == 15
    print("PASS: legacy sizes 1..34 and descents; parent-inferred A ghosting and retries.")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("n", type=int, nargs="?", default=7, help="parent size (default: 7)")
    parser.add_argument("--csv", action="store_true", help="write CSV instead of aligned text")
    parser.add_argument("--baseline", choices=BASELINE_MODES, default="inherited",
                        help="local baseline: parent order or selected traversal (default: inherited)")
    parser.add_argument("--closure", choices=CLOSURE_MODES, default="canonical",
                        help="preserve parent cosets or allow child canonical closures (default: canonical)")
    parser.add_argument("--ghosting", choices=GHOSTING_MODES, default="off",
                        help="ghost noncanonical children; wrong-sum-a also retries A on failed sums (default: off)")
    parser.add_argument("--ghost-step", choices=GHOST_STEP_RULES, default="parent",
                        help="A inference: parent-scaled transitions (default) or legacy descents; B keeps its previous rule")
    parser.add_argument("--show-baselines", action="store_true",
                        help="include baseline/closure modes and all three local baselines in the table")
    parser.add_argument("--case", nargs=2, type=int, metavar=("X", "K"),
                        help="inspect one parent value X and prefix length K, including baselines and edges")
    parser.add_argument("--self-test", action="store_true", help="verify the reference results")
    args = parser.parse_args()
    if args.n < 1:
        parser.error("n must be a positive integer")
    if args.case and not (0 <= args.case[0] < args.n and 1 <= args.case[1] <= args.n):
        parser.error("require 0 <= X < n and 1 <= K <= n")
    if args.ghosting != "off" and args.closure != "canonical":
        parser.error("ghosting requires --closure canonical")
    if args.self_test:
        self_test()
    elif args.case:
        print_case(args.n, *args.case, args.baseline, args.closure, args.ghosting, args.ghost_step)
    else:
        print_table(args.n, args.csv, args.baseline, args.closure, args.show_baselines, args.ghosting, args.ghost_step)


if __name__ == "__main__":
    main()
