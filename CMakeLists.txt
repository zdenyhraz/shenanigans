cmake_minimum_required(VERSION 3.7)
project(shenanigans)

# cpp
set(CMAKE_CXX_STANDARD 20)

# compiler
if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /favor:INTEL64 /arch:SSE /arch:SSE2 /arch:AVX /arch:AVX2 /openmp") # optimizations
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /WX") # warnings
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -fopenmp") # optimizations
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wfatal-errors -Wextra -Wpedantic -Wshadow") # warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-deprecated-enum-enum-conversion -Wno-attributes") # excluded warnings
endif()

# ignore warnings from external sources
file(GLOB_RECURSE SRC_EXTERNAL "external/*.h" "external/*.hpp" "external/*.c" "external/*.cpp")
set_source_files_properties(${SRC_EXTERNAL} PROPERTIES COMPILE_FLAGS "-w")

# variables
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(ENABLE_MATPLOTLIB OFF)
set(ENABLE_PROFILING ON)
set(ENABLE_TRACY ON)
set(ENABLE_OPTICK OFF)
set(ENABLE_SANITIZER OFF)

if(CI)
  set(ENABLE_PROFILING OFF)
endif()

if(ENABLE_SANITIZER)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address -fno-omit-frame-pointer)
endif()

# status messages
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "ENABLE_PROFILING: " ${ENABLE_PROFILING})
message(STATUS "ENABLE_MATPLOTLIB: " ${ENABLE_MATPLOTLIB})

# targets
add_executable(shenanigans)
add_executable(shenanigans_test)

# fmt
add_subdirectory(external/fmt)
include_directories("external/fmt/include")
target_link_libraries(shenanigans PRIVATE fmt::fmt-header-only)
target_link_libraries(shenanigans_test PRIVATE fmt::fmt-header-only)

# scnlib
add_subdirectory(external/scnlib)
include_directories("external/scnlib/include")
target_link_libraries(shenanigans PRIVATE scn::scn)
target_link_libraries(shenanigans_test PRIVATE scn::scn)

# gtest
add_subdirectory(external/gtest)
include_directories("external/gtest/googletest/include")
include_directories("external/gtest/googlemock/include")
enable_testing()
include(GoogleTest)
target_link_libraries(shenanigans_test PRIVATE GTest::gtest_main GTest::gmock)

# json
add_subdirectory(external/json)
include_directories("external/json/include")
target_link_libraries(shenanigans PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(shenanigans_test PRIVATE nlohmann_json::nlohmann_json)

if(ENABLE_PROFILING)
  if(ENABLE_OPTICK)
    # optick 
    include_directories("external/optick/src")
    file(GLOB_RECURSE SRC_OPTICK CONFIGURE_DEPENDS external/optick/src/*.h external/optick/src/*.cpp)
    target_sources(shenanigans PRIVATE ${SRC_OPTICK})
    target_sources(shenanigans_test PRIVATE ${SRC_OPTICK})
  endif()
  if(ENABLE_TRACY)
    # tracy
    include_directories("external/tracy")
    add_compile_definitions(TRACY_ENABLE)
    add_compile_definitions(TRACY_TIMER_FALLBACK) # needed for WSL
    add_compile_definitions(TRACY_NO_FRAME_IMAGE) # not using frame images
    file(GLOB_RECURSE SRC_TRACY CONFIGURE_DEPENDS external/tracy/Tracy.hpp external/tracy/TracyClient.cpp)
    target_sources(shenanigans PRIVATE ${SRC_TRACY})
    target_sources(shenanigans_test PRIVATE ${SRC_TRACY})
    target_link_libraries(shenanigans PRIVATE ${CMAKE_DL_LIBS})
    target_link_libraries(shenanigans_test PRIVATE ${CMAKE_DL_LIBS})
  endif()
  add_compile_definitions(ENABLE_PROFILING)
endif()

# matplotlib
if(ENABLE_MATPLOTLIB)
  find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)
  include_directories("external/matplotlib")
  target_link_libraries(shenanigans PRIVATE Python3::Python Python3::Module Python3::NumPy)
  target_link_libraries(shenanigans_test PRIVATE Python3::Python Python3::Module Python3::NumPy)
endif()

# opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(shenanigans PRIVATE ${OpenCV_LIBS})
target_link_libraries(shenanigans_test PRIVATE ${OpenCV_LIBS})

# qt
find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)
target_link_libraries(shenanigans PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport)
target_link_libraries(shenanigans_test PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport)

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
src/Logger/*.h src/Logger/*.cpp
src/Optimization/*.h src/Optimization/*.cpp
src/Plot/*.h src/Plot/*.cpp
src/Precompiled/*.h src/Precompiled/*.cpp
src/Procedural/*.h src/Procedural/*.cpp
src/Sasko/*.h src/Sasko/*.cpp
src/Snake/*.h src/Snake/*.cpp
src/Utils/*.h src/Utils/*.cpp
)
target_sources(shenanigans PRIVATE ${SRC})
target_sources(shenanigans_test PRIVATE ${SRC})

# app sources
file(GLOB_RECURSE SRC_APP CONFIGURE_DEPENDS src/Application/*.h src/Application/*.cpp)
target_sources(shenanigans PRIVATE ${SRC_APP})

# test sources
file(GLOB_RECURSE SRC_TEST CONFIGURE_DEPENDS test/*.h test/*.cpp)
target_sources(shenanigans_test PRIVATE ${SRC_TEST})
gtest_discover_tests(shenanigans_test)

# precompiled headers
target_precompile_headers(shenanigans PRIVATE src/Precompiled.h)
target_precompile_headers(shenanigans_test PRIVATE src/Precompiled.h)
