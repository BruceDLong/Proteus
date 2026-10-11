#!/usr/bin/env bash
set -euo pipefail
report_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
source_dir=${1:-/home/bruce/devl/Proteus4}
stage_dir=$(mktemp -d /tmp/proteus4-trace.XXXXXX)
mkdir "$stage_dir/LocalBuild"
cp -a "$source_dir/graph" "$stage_dir/graph"
cp "$source_dir/LocalBuild/TestProteus.cpp" "$stage_dir/LocalBuild/"
cp "$report_dir/trace-probe.cpp" "$report_dir/trace.scons" "$stage_dir/LocalBuild/"
python3 "$report_dir/instrument.py" "$stage_dir"
python3 /home/bruce/devl/CodeDog/Scons/scons.py -Q -C "$stage_dir/LocalBuild" -f trace.scons > "$stage_dir/build.log" 2>&1
(
    cd "$stage_dir/LocalBuild"
    ./trace-probe > "$stage_dir/probe.log" 2>&1
)
cat "$stage_dir/probe.log"
printf 'Trace artifacts: %s\n' "$stage_dir"
