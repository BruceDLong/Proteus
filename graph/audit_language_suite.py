#!/usr/bin/env python3
"""Run original language fixtures, unchanged, in isolated bounded processes.

Preserves parse/norm/multi/world modes and expected outputs. Normalization uses
Agent's graph entry point. JSON records and individual logs distinguish semantic
failures from crashes/timeouts; test-family grouping is not causal diagnosis.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import re
import subprocess


def main():
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--suite', type=Path, default=root/'defaultTests.tst')
    parser.add_argument('--timeout', type=float, default=5)
    parser.add_argument('--select', default='')
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error('--timeout must be positive')
    suite = args.suite.resolve()
    fixtures = []
    for line in suite.read_text().splitlines():
        if not line.strip() or line.lstrip().startswith('#'):
            continue
        columns = re.split(r'(?<!\\),', line)
        if len(columns) >= 5 and columns[0].strip().startswith(args.select):
            fixtures.append((columns[0].strip(), columns[4].strip(), line))
    if not fixtures:
        parser.error('no matching language fixtures')
    args.output.mkdir(parents=True, exist_ok=True)
    records = []
    for index, (name, mode, row) in enumerate(fixtures, 1):
        log = args.output / f'{index:03d}.log'
        # One-row files avoid the harness treating a name as a prefix selector.
        fixture = args.output / 'current.tst'
        fixture.write_text(row + '\n')
        with log.open('w') as output:
            try:
                run = subprocess.run([str(root/'LocalBuild/TestProteus'), '-t', str(fixture.resolve())+':'],
                                     cwd=root/'LocalBuild', stdout=output, stderr=subprocess.STDOUT,
                                     timeout=args.timeout)
                code = run.returncode
            except subprocess.TimeoutExpired:
                code = None
        text = log.read_text(errors='replace')
        summary = re.search(r'Total Failures/Tests:\s*(\d+)/(\d+)', text)
        if code is None:
            status = 'timeout'
        elif code != 0:
            status = 'process_error'
        elif not summary or int(summary[2]) != 1:
            status = 'incomplete'
        else:
            status = 'fail' if int(summary[1]) else 'pass'
        records.append(dict(name=name, mode=mode, status=status, returncode=code, log=log.name))
        (args.output/'results.json').write_text(json.dumps(records, indent=2)+'\n')
        if status in ('timeout', 'process_error', 'incomplete') or index % 25 == 0:
            print(f'{index}/{len(fixtures)} {name}: {status}', flush=True)
    fixture.unlink(missing_ok=True)
    print(json.dumps(dict(Counter(r['status'] for r in records)), sort_keys=True), flush=True)
    return int(any(r['status'] != 'pass' for r in records))


if __name__ == '__main__':
    raise SystemExit(main())
