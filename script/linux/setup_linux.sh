#!/bin/bash

sudo apt update
sudo apt install gcc-12 g++-12
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100
sudo apt install ninja-build

# opengl
sudo apt install libglu1-mesa-dev mesa-common-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev

# torch
(cd libs && wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.11.0%2Bcpu.zip --no-check-certificate)
sudo chmod 755 libs/libtorch-cxx11-abi-shared-with-deps-1.11.0+cpu.zip
(cd libs && unzip libtorch-cxx11-abi-shared-with-deps-1.11.0+cpu.zip)
