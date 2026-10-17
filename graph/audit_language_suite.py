#!/usr/bin/env python3
"""Run original language fixtures or registered compiled tests in bounded processes.

Preserves parse/norm/multi/world modes and expected outputs. Normalization uses
Agent's graph entry point. JSON records and individual logs distinguish semantic
failures from crashes/timeouts; test-family grouping is not causal diagnosis.
"""
import argparse
import hashlib
from collections import Counter
import json
from pathlib import Path
import re
import subprocess


def main():
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    source = parser.add_mutually_exclusive_group()
    source.add_argument('--suite', type=Path, default=root/'Tests/defaultTests.tst')
    source.add_argument('--compiled', action='store_true', help='Read registered tests from ProteusTests.dog')
    parser.add_argument('--timeout', type=float, default=5)
    parser.add_argument('--select', default='')
    parser.add_argument('--exclude', action='append', default=[], help='Exclude a test-name prefix')
    parser.add_argument('--include-ambiguous', action='store_true', help='Run documented ambiguous fixtures instead of reporting skips')
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error('--timeout must be positive')
    fixtures = []
    if args.compiled:
        block = (root/'ProteusTests.dog').read_text().split('TestsToRun = `', 1)[1].split('`', 1)[0]
        fixtures = [(name, 'compiled', None) for line in block.splitlines()
                    if (name := line.split('//', 1)[0].strip())]
    else:
        for line in args.suite.resolve().read_text().splitlines():
            if not line.strip() or line.lstrip().startswith('#'):
                continue
            columns = re.split(r'(?<!\\),', line)
            if len(columns) >= 5:
                fixtures.append((columns[0].strip(), columns[4].strip(), line))
    fixtures = [f for f in fixtures if f[0].startswith(args.select) and
                not any(f[0].startswith(prefix) for prefix in args.exclude)]
    if not fixtures:
        parser.error('no matching tests')
    args.output.mkdir(parents=True, exist_ok=True)
    exceptions = {} if args.include_ambiguous else {
        (entry['suite'], entry['name']): entry
        for entry in json.loads((root/'graph/test-ambiguities.json').read_text())
    }
    records = []
    for index, (name, mode, row) in enumerate(fixtures, 1):
        exception = exceptions.get(('compiled' if args.compiled else args.suite.name, name))
        if exception:
            digest = hashlib.sha256((row or name).encode()).hexdigest()
            if digest != exception['fixture_sha256']:
                parser.error(f'ambiguity entry is stale for {name}; review it before skipping')
            records.append(dict(name=name, mode=mode, status='skipped_ambiguous',
                                returncode=None, log=None, reason=exception['reason']))
            (args.output/'results.json').write_text(json.dumps(records, indent=2)+'\n')
            print(f'{index}/{len(fixtures)} {name}: skipped_ambiguous', flush=True)
            continue
        log = args.output / f'{index:03d}.log'
        # A compiled name is exact; file-based selectors can be prefixes, so
        # isolate each language fixture in a one-row file.
        fixture = args.output / 'current.tst'
        selector = name
        if row is not None:
            fixture.write_text(row + '\n')
            selector = str(fixture.resolve()) + ':'
        with log.open('w') as output:
            try:
                run = subprocess.run([str(root/'LocalBuild/TestProteus'), '-t', selector],
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
    (args.output/'current.tst').unlink(missing_ok=True)
    print(json.dumps(dict(Counter(r['status'] for r in records)), sort_keys=True), flush=True)
    return int(any(r['status'] not in ('pass', 'skipped_ambiguous') for r in records))


if __name__ == '__main__':
    raise SystemExit(main())
