#!/usr/bin/env python3
"""Measure editor selection using a real offscreen project runtime.

Run from any directory after building ORCA:
    python3 tools/benchmark_editor_selection.py [preview|tree|first]

Preview calls Preview.pick/hitTest using rectangles from a real runtime snapshot;
tree sends TreeView.SelectItem. Both alternate two elements, discard two warmups,
and report five wall-clock samples. First launches three fresh processes and measures
the first preview selection after the initial document is open. Each sample ends
after queue processing and layout, not physical frame presentation. Setup, snapshot,
and teardown are excluded. CPU samples cover the editor process, not its server.
Set ORCA_BENCH_RUNS to change the number of fresh-process first-selection runs.
"""

import os
from pathlib import Path
import statistics
import subprocess
import sys
import tempfile
import time

root = Path(__file__).resolve().parent.parent
mode = sys.argv[1] if len(sys.argv) == 2 else 'preview'
if len(sys.argv) > 2 or mode not in ('preview', 'tree', 'first'):
    raise SystemExit('usage: benchmark_editor_selection.py [preview|tree|first]')

samples, cpu_samples = [], []
for run in range(int(os.getenv('ORCA_BENCH_RUNS', '3')) if mode == 'first' else 1):
    with tempfile.TemporaryDirectory(prefix='orca-editor-benchmark-') as fixture:
        env = os.environ.copy()
        env.update(ORCA_BENCH_FIXTURE=fixture, ORCA_BENCH_MODE='preview' if mode == 'first' else mode,
                   ORCA_BENCH_ITERATIONS='1' if mode == 'first' else '7', TMPDIR=fixture)
        process = subprocess.Popen(
            [str(root / 'build/bin/orca'), '-test=tests/benchmark_editor_selection.lua'],
            cwd=root, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, bufsize=1,
        )
        starts, run_samples, run_cpu = {}, [], []
        for line in process.stdout:
            now = time.monotonic_ns()
            words = line.strip().split()
            if len(words) == 2 and words[0] == 'BEGIN':
                starts[words[1]] = now
            elif len(words) == 3 and words[0] == 'CPU':
                run_cpu.append(float(words[2]))
            elif len(words) == 2 and words[0] == 'END':
                sample = (now - starts.pop(words[1])) / 1e6
                run_samples.append(sample)
                print(f'run {run + 1}, sample {words[1]}: {sample:.2f} ms', flush=True)
            else:
                print(line, end='', flush=True)
        status = process.wait()
        expected = 1 if mode == 'first' else 7
        if status or len(run_samples) != expected or len(run_cpu) != expected or starts:
            raise SystemExit(f'benchmark failed: exit={status}, samples={len(run_samples)}')
        samples.extend(run_samples if mode == 'first' else run_samples[2:])
        cpu_samples.extend(run_cpu if mode == 'first' else run_cpu[2:])
print(f'{mode}: wall ms, {len(samples)} measured; '
      f'median={statistics.median(samples):.2f} '
      f'min={min(samples):.2f} max={max(samples):.2f}')
print('samples_ms=' + ','.join(f'{value:.2f}' for value in samples))
print(f'editor CPU ms: median={statistics.median(cpu_samples):.2f} min={min(cpu_samples):.2f} max={max(cpu_samples):.2f}')
