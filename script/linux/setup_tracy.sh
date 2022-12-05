#!/bin/bash

sudo apt update
sudo apt install libcapstone-dev
cd libs/tracy/profiler/build/unix && make
