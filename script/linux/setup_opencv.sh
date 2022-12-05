#!/bin/bash

wget https://github.com/opencv/opencv/archive/refs/tags/4.5.5.tar.gz
sudo chmod 755 4.5.5.tar.gz
tar -xf 4.5.5.tar.gz && mv opencv-4.5.5 opencv && cd opencv
git clone https://github.com/opencv/opencv_contrib.git
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DOPENCV_ENABLE_NONFREE=ON -DOPENCV_EXTRA_MODULES_PATH=../opencv_contrib/modules -DBUILD_TESTS=OFF -DBUILD_opencv_python=OFF -DBUILD_opencv_java=OFF -DBUILD_opencv_apps=OFF
cmake --build . --config Release
sudo ninja install
