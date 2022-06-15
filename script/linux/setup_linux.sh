#!/bin/bash

# update packages
sudo apt update

# gcc
sudo apt install gcc-11 g++-11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100

# ninja
sudo apt install ninja-build

# opengl
sudo apt install libglu1-mesa-dev mesa-common-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev

# torch
(cd libs && wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.11.0%2Bcpu.zip --no-check-certificate)
sudo chmod 755 libs/libtorch-cxx11-abi-shared-with-deps-1.11.0+cpu.zip
(cd libs && unzip libtorch-cxx11-abi-shared-with-deps-1.11.0+cpu.zip)
