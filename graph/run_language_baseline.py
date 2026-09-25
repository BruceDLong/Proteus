#!/usr/bin/env python3
"""Run selected original language fixtures through the public Agent graph entry point.

Inputs and expected strings come from defaultTests.tst on each run. The only
changed column is the execution mode, including for selected parser fixtures.
The legacy engine is never a fallback. Nonzero exits record outstanding gaps;
this exploratory baseline is not a requirement that every case pass yet.
"""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile


def main():
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--select", default="", help="only names beginning with this prefix")
    parser.add_argument("--timeout", type=float, default=60, help="whole-run timeout in seconds")
    parser.add_argument("--fixtures", type=Path, default=root / "graph/language-baseline.txt",
                        help="file containing original fixture names")
    args = parser.parse_args()
    names = [line.strip() for line in args.fixtures.read_text().splitlines()
             if line.strip() and not line.lstrip().startswith("#")]
    names = [name for name in names if name.startswith(args.select)]
    if not names:
        parser.error("selection matched no baseline fixtures")
    rows = {}
    for line in (root / "defaultTests.tst").read_text().splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        columns = re.split(r"(?<!\\),", line)
        if len(columns) >= 5:
            rows[columns[0].strip()] = columns
    selected = []
    for name in names:
        if name not in rows:
            parser.error(f"fixture no longer exists: {name}")
        columns = rows[name][:]
        if columns[4].strip() not in ("norm", "parse"):
            parser.error(f"unsupported original test mode: {name}")
        columns[4] = " graph"
        selected.append(",".join(columns))
    executable = root / "LocalBuild/TestProteus"
    if not executable.is_file():
        parser.error("build first with codeDog Proteus.Lib.dog")
    with tempfile.TemporaryDirectory(prefix="proteus4-language-") as temporary:
        suite = Path(temporary) / "baseline.tst"
        suite.write_text("\n".join(selected) + "\n")
        try:
            run = subprocess.run([str(executable), "-t", str(suite) + ":"],
                                 cwd=executable.parent, text=True, capture_output=True,
                                 timeout=args.timeout)
        except subprocess.TimeoutExpired:
            print(f"Graph baseline exceeded {args.timeout}s", flush=True)
            return 2
    print(run.stdout, end="")
    print(run.stderr, end="")
    cases = re.findall(r"GRAPH_CASE ([^\r\n]+)", run.stdout)
    summary = re.search(r"Total Failures/Tests:\s*(\d+)/(\d+)", run.stdout)
    if run.returncode or sorted(cases) != sorted(names) or not summary:
        print("Incomplete graph baseline: missing cases, summary, or process failure")
        return 2
    failures, total = map(int, summary.groups())
    if total != len(names):
        print(f"Unexpected test count: {total}, expected {len(names)}")
        return 2
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
