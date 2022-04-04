#!/bin/bash

# update packages
sudo apt-get update

# gcc
sudo apt install gcc-11 g++-11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100 --slave /usr/bin/g++ g++ /usr/bin/g++-11 --slave /usr/bin/gcov gcov /usr/bin/gcov-11
sudo update-alternatives --set gcc /usr/bin/gcc-11

# ninja
sudo apt-get install ninja-build

# opengl
sudo apt-get install libglu1-mesa-dev mesa-common-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev

# python
sudo apt-get install libpython-dev

# torch
wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.11.0%2Bcpu.zip --no-check-certificate
sudo chmod 755 libtorch-cxx11-abi-shared-with-deps-1.11.0+cpu.zip
unzip libtorch-cxx11-abi-shared-with-deps-1.11.0+cpu.zip
