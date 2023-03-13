cd libs
wget https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-1.13.0%2Bcpu.zip -O libtorch.zip
tar -xf libtorch.zip
del libtorch.zip

:: delete lazy_init_num_threads() function definition @libs\libtorch\include\ATen\Parallel.h
