cd build
valgrind --tool=callgrind ./shenanigans #--dump-before=DifferentialRotation::Calculate
kcachegrind