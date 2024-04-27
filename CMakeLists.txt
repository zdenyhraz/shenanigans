cmake_minimum_required(VERSION 3.22)
project(shenanigans LANGUAGES CXX)

# variables
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(ENABLE_PROFILING OFF)
set(ENABLE_SANITIZER OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ENABLE_PCH ON)
set(ENABLE_PRIVATE ON)
set(ENABLE_COVERAGE OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ci variables
if(CI OR DEFINED ENV{CI})
  message(STATUS "CI: ON")
  set(ENABLE_COVERAGE ON)
  #set(ENABLE_SANITIZER ON)
  set(ENABLE_PROFILING OFF)
  set(CMAKE_UNITY_BUILD OFF)
endif()

# compiler flags
if(MSVC)
  add_compile_options(/favor:AMD64 /arch:SSE /arch:SSE2 /arch:AVX /arch:AVX2 /openmp /MT)
  #add_compile_options(/Wall /WX)
else()
  add_compile_options(-march=native -fopenmp -pthread -lstdc++_libbacktrace)
  add_compile_options(-Wall -Wfatal-errors -Wextra -Wpedantic -Wshadow)#-Werror
  add_compile_options(-Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -Wno-sign-compare)
  add_compile_options("$<$<CONFIG:DEBUG>:-g;-O0>")
  add_compile_options("$<$<CONFIG:RELEASE>:-O3>")
  add_compile_options("$<$<COMPILE_LANGUAGE:C>:-w>")
  add_link_options(-fopenmp -pthread)
endif()

# ignore warnings from external sources
file(GLOB_RECURSE SRC_LIBS CONFIGURE_DEPENDS libs/*.h libs/*.hpp libs/*.c libs/*.cpp)
if(MSVC)
  set_source_files_properties(${SRC_LIBS} PROPERTIES COMPILE_FLAGS /w)
else()
  set_source_files_properties(${SRC_LIBS} PROPERTIES COMPILE_FLAGS -w)
endif()

# ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

# sanitizer
if(ENABLE_SANITIZER)
  if(MSVC)
    add_compile_options(/fsanitize=address /Oy-)
  else()
    add_compile_options(-fsanitize=address,undefined,leak -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined,leak -fno-omit-frame-pointer)
  endif()
endif()

# status messages
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})
message(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER_ID} " " ${CMAKE_C_COMPILER_VERSION})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})
message(STATUS "CMAKE_CXX_COMPILER_LAUNCHER: " ${CMAKE_CXX_COMPILER_LAUNCHER})
message(STATUS "ENABLE_PROFILING: " ${ENABLE_PROFILING})
message(STATUS "ENABLE_SANITIZER: " ${ENABLE_SANITIZER})
message(STATUS "ENABLE_PRIVATE: " ${ENABLE_PRIVATE})
message(STATUS "OPENCV_DIR: " ${OpenCV_DIR})
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# targets
add_executable(shenanigans)
add_executable(shenanigans_test)
add_executable(shenanigans_terminal)
add_executable(shenanigans_benchmark)
add_executable(shenanigans_icsw)

# precompiled headers
if (ENABLE_PCH)
  set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
  set(SRC_PRECOMPILED src/Precompiled.hpp)
  set(SRC_PRECOMPILED_GUI src/PrecompiledGui.hpp)
  target_precompile_headers(shenanigans PRIVATE ${SRC_PRECOMPILED_GUI} ${SRC_PRECOMPILED})
  target_precompile_headers(shenanigans_test PRIVATE ${SRC_PRECOMPILED})
  target_precompile_headers(shenanigans_terminal PRIVATE ${SRC_PRECOMPILED})
  target_precompile_headers(shenanigans_benchmark PRIVATE ${SRC_PRECOMPILED})
endif()

# pybind11
add_subdirectory(libs/pybind11)
include_directories(libs/pybind11/include)
target_link_libraries(shenanigans PRIVATE pybind11::embed)
target_link_libraries(shenanigans_test PRIVATE pybind11::embed)
target_link_libraries(shenanigans_terminal PRIVATE pybind11::embed)
target_link_libraries(shenanigans_benchmark PRIVATE pybind11::embed)

# fmt
add_subdirectory(libs/fmt)
include_directories(libs/fmt/include)
target_link_libraries(shenanigans PRIVATE fmt::fmt)
target_link_libraries(shenanigans_test PRIVATE fmt::fmt)
target_link_libraries(shenanigans_terminal PRIVATE fmt::fmt)
target_link_libraries(shenanigans_benchmark PRIVATE fmt::fmt)
target_link_libraries(shenanigans_icsw PRIVATE fmt::fmt)

# gtest
add_subdirectory(libs/gtest)
target_include_directories(shenanigans_test PRIVATE libs/gtest/googletest/include)
enable_testing()
include(GoogleTest)
target_link_libraries(shenanigans_test PRIVATE GTest::gtest)

# benchmark
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/benchmark)
target_link_libraries(shenanigans_benchmark PRIVATE benchmark::benchmark)

# json
add_subdirectory(libs/json)
include_directories(libs/json/include)
target_link_libraries(shenanigans PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(shenanigans_test PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(shenanigans_terminal PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(shenanigans_benchmark PRIVATE nlohmann_json::nlohmann_json)

# tracy
if(ENABLE_PROFILING)
  include_directories(libs/tracy)
  add_compile_definitions(TRACY_ENABLE) # for tracy macros
  add_compile_definitions(ENABLE_PROFILING) # for custom macros
  add_compile_definitions(TRACY_NO_FRAME_IMAGE) # not using frame images
  set(SRC_TRACY libs/tracy/Tracy.hpp libs/tracy/TracyClient.cpp)
  target_sources(shenanigans PRIVATE ${SRC_TRACY})
  target_link_libraries(shenanigans PRIVATE ${CMAKE_DL_LIBS})
endif()

# opencv
find_package(OpenCV REQUIRED HINTS ${OpenCV_DIR} libs/opencv-install)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(shenanigans PRIVATE ${OpenCV_LIBS})
target_link_libraries(shenanigans_test PRIVATE ${OpenCV_LIBS})
target_link_libraries(shenanigans_terminal PRIVATE ${OpenCV_LIBS})
target_link_libraries(shenanigans_benchmark PRIVATE ${OpenCV_LIBS})

# implot
include_directories(libs/implot)
file(GLOB SRC_IMPLOT CONFIGURE_DEPENDS libs/implot/*.cpp)
target_sources(shenanigans PRIVATE ${SRC_IMPLOT})

# imgui
include_directories(libs/imgui)
include_directories(libs/imgui/misc/cpp)
include_directories(libs/imgui/backends)
file(GLOB SRC_IMGUI CONFIGURE_DEPENDS libs/imgui/*.cpp libs/imgui/misc/cpp/imgui_stdlib.cpp)
set(SRC_IMGUI_BACKEND libs/imgui/backends/imgui_impl_glfw.cpp libs/imgui/backends/imgui_impl_opengl3.cpp)
target_sources(shenanigans PRIVATE ${SRC_IMGUI} ${SRC_IMGUI_BACKEND})

# imgui-nodes
include_directories(libs/imgui-nodes)
file(GLOB SRC_IMGUI_NODES CONFIGURE_DEPENDS libs/imgui-nodes/*.cpp)
target_sources(shenanigans PRIVATE ${SRC_IMGUI_NODES})

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)
target_link_libraries(shenanigans PRIVATE glfw)

# glad
add_subdirectory(libs/glad)
target_link_libraries(shenanigans PRIVATE glad)

# zmq
set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/libzmq)

# cppzmq
set(CPPZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/cppzmq)

# boost
set(BOOST_ENABLE_CMAKE ON CACHE BOOL "" FORCE)
set(BOOST_INCLUDE_LIBRARIES interprocess)
add_subdirectory(libs/boost EXCLUDE_FROM_ALL)
include_directories(${Boost_INCLUDE_DIRS})

# shenanigans_private
get_filename_component(PATH_PRIVATE "${src/NDA}" REALPATH)
if (ENABLE_PRIVATE AND EXISTS "${PATH_PRIVATE}/LICENSE.md")
  add_compile_definitions(ENABLE_PRIVATE)
  include_directories(src/NDA)
  file(GLOB_RECURSE SRC_NDA CONFIGURE_DEPENDS src/NDA/Windows/*.cpp)
  target_sources(shenanigans PRIVATE ${SRC_NDA})
  add_subdirectory(src/NDA/Clarity)
endif()

# custom
include_directories(src)

# sources
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
 src/Astrophysics/*.cpp
 src/ImageProcessing/*.cpp
 src/ImageRegistration/*.cpp
 src/Log/*.cpp
 src/Math/*.cpp
 src/Microservice/*.cpp
 src/Optimization/*.cpp
 src/Plot/*.cpp
 src/Python/*.cpp
 src/Random/*.cpp
 src/Utils/*.cpp
 )
file(GLOB_RECURSE SRC_GUI CONFIGURE_DEPENDS src/Gui/*.cpp)

target_sources(shenanigans PRIVATE ${SRC} ${SRC_GUI})
target_sources(shenanigans_test PRIVATE ${SRC})
target_sources(shenanigans_terminal PRIVATE ${SRC})
target_sources(shenanigans_benchmark PRIVATE ${SRC})

# shenanigans app sources
file(GLOB SRC_SHENANIGANS_APP CONFIGURE_DEPENDS apps/shenanigans/*.cpp)
target_sources(shenanigans PRIVATE ${SRC_SHENANIGANS_APP})

# shenanigans_test app sources
file(GLOB SRC_SHENANIGANS_TEST CONFIGURE_DEPENDS apps/shenanigans_test/*.cpp test/shenanigans/*.cpp)
target_sources(shenanigans_test PRIVATE ${SRC_SHENANIGANS_TEST})
gtest_discover_tests(shenanigans_test)

# shenanigans_terminal app sources
file(GLOB SRC_SHENANIGANS_TERMINAL_APP CONFIGURE_DEPENDS apps/shenanigans_terminal/*.cpp)
target_sources(shenanigans_terminal PRIVATE ${SRC_SHENANIGANS_TERMINAL_APP})

# shenanigans_benchmark app sources
file(GLOB SRC_SHENANIGANS_BENCHMARK_APP CONFIGURE_DEPENDS apps/shenanigans_benchmark/*.cpp)
target_sources(shenanigans_benchmark PRIVATE ${SRC_SHENANIGANS_BENCHMARK_APP})

# shenanigans_icsw app sources
file(GLOB SRC_SHENANIGANS_ICSW_APP CONFIGURE_DEPENDS apps/shenanigans_icsw/*.cpp)
target_sources(shenanigans_icsw PRIVATE ${SRC_SHENANIGANS_ICSW_APP})
