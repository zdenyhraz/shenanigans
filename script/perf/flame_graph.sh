#!/bin/bash

cd build
perf record -a --call-graph fp ./shenanigans
perf script > out.perf
../libs/flamegraph/stackcollapse-perf.pl out.perf > out.folded
../libs/flamegraph/flamegraph.pl out.folded > flamegraph.svg
