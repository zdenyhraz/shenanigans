#!/bin/bash

cd build
valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./shenanigans #--dump-before=DifferentialRotation::Calculate
kcachegrind