cd libs
wget https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip -O libtorch.zip

tar -xf libtorch.zip
del libtorch.zip

:: delete lazy_init_num_threads() function definition @libs\libtorch\include\ATen\Parallel.h
