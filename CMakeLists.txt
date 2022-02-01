cmake_minimum_required(VERSION 3.7)
project(shenanigans)

# cpp
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /favor:INTEL64 /arch:SSE /arch:SSE2 /arch:AVX /arch:AVX2 /openmp") # optimizations
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /WX") # warnings
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -fopenmp") # optimizations
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wfatal-errors -Wextra -Wpedantic -Wshadow") # warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-deprecated-enum-enum-conversion -Wno-attributes") # excluded warnings
endif()

# fmt
add_subdirectory(external/fmt)
include_directories("external/fmt/include")

# scnlib
add_subdirectory(external/scnlib)
include_directories("external/scnlib/include")

# gtest
add_subdirectory(external/gtest)
include_directories("external/gtest/googletest/include")
include_directories("external/gtest/googlemock/include")
enable_testing()
include(GoogleTest)

# json
add_subdirectory(external/json)
include_directories("external/json/include")

# matplotlib
find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)
include_directories("external/matplotlib")

# opencv
find_package(OpenCV REQUIRED PATHS E:\\Repos\\opencv-4.5.5\\build\\install)
include_directories(${OpenCV_INCLUDE_DIRS})

# qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport REQUIRED PATHS C:\\Qt\\5.12.2\\mingw73_64)

# custom
include_directories("src")

# sources
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
src/*.ui src/*.rc src/*.qrc
src/Astrophysics/*.h src/Astrophysics/*.cpp
src/ComplexityEstimation/*.h src/ComplexityEstimation/*.cpp
src/Core/*.h src/Core/*.cpp
src/Draw/*.h src/Draw/*.cpp
src/Features/*.h src/Features/*.cpp
src/Filtering/*.h src/Filtering/*.cpp
src/Fit/*.h src/Fit/*.cpp
src/Fourier/*.h src/Fourier/*.cpp
src/Fractal/*.h src/Fractal/*.cpp
src/InverseMap/*.h src/InverseMap/*.cpp
src/IPC/*.h src/IPC/*.cpp
src/Log/*.h src/Log/*.cpp
src/Optimization/*.h src/Optimization/*.cpp
src/Plot/*.h src/Plot/*.cpp
src/Precompiled/*.h src/Precompiled/*.cpp
src/Procedural/*.h src/Procedural/*.cpp
src/Sasko/*.h src/Sasko/*.cpp
src/Snake/*.h src/Snake/*.cpp
src/Utils/*.h src/Utils/*.cpp
)
file(GLOB_RECURSE SRC_APP CONFIGURE_DEPENDS src/Application/*.h src/Application/*.cpp )
file(GLOB_RECURSE SRC_TEST CONFIGURE_DEPENDS test/*.h test/*.cpp)

# shenanigans
add_executable(shenanigans ${SRC} ${SRC_APP})
target_link_libraries(shenanigans PRIVATE ${OpenCV_LIBS} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport fmt::fmt-header-only scn::scn nlohmann_json::nlohmann_json Python3::Python Python3::Module Python3::NumPy)
target_precompile_headers(shenanigans PRIVATE src/Precompiled.h)

# shenanigans_test
add_executable(shenanigans_test ${SRC} ${SRC_TEST})
target_link_libraries(shenanigans_test PRIVATE ${OpenCV_LIBS} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport fmt::fmt-header-only scn::scn nlohmann_json::nlohmann_json Python3::Python Python3::Module Python3::NumPy GTest::gtest_main GTest::gmock)
target_precompile_headers(shenanigans_test PRIVATE src/Precompiled.h)
gtest_discover_tests(shenanigans_test)