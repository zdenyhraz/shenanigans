#!/bin/bash

cd build
perf record -a --call-graph fp ./shenanigans
perf script > out.perf
../external/flamegraph/stackcollapse-perf.pl out.perf > out.folded
../external/flamegraph/flamegraph.pl out.folded > flamegraph.svg