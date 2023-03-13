git clone https://github.com/opencv/opencv.git --branch 4.x --single-branch --depth 1
git clone https://github.com/opencv/opencv_contrib.git --branch 4.x --single-branch --depth 1
cd opencv && mkdir build && cd build

cmake .. -DCMAKE_BUILD_TYPE=Debug -DOPENCV_ENABLE_NONFREE=ON -DOPENCV_EXTRA_MODULES_PATH="../../opencv_contrib/modules" -DBUILD_TESTS=OFF -DBUILD_opencv_python=OFF -DBUILD_opencv_java=OFF -DBUILD_opencv_apps=OFF
cmake --build . --config Debug -j 8
cmake --install . --prefix "../install/Debug"

cmake .. -DCMAKE_BUILD_TYPE=Release -DOPENCV_ENABLE_NONFREE=ON -DOPENCV_EXTRA_MODULES_PATH="../../opencv_contrib/modules" -DBUILD_TESTS=OFF -DBUILD_opencv_python=OFF -DBUILD_opencv_java=OFF -DBUILD_opencv_apps=OFF
cmake --build . --config Release -j 8
cmake --install . --prefix "../install/Release"

:: -DENABLE_FAST_MATH=ON -DWITH_CUDA=ON -DCUDA_FAST_MATH=ON -DCUDA_ARCH_BIN="8.9"
