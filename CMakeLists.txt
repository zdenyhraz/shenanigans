cmake_minimum_required(VERSION 3.22)
project(shenanigans LANGUAGES CXX)

# variables
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ENABLE_PCH ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(ENABLE_PROFILING "Enable performance profiling" OFF)
option(ENABLE_PROFILING_MEMORY "Enable memory profiling" OFF)
option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
option(ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
option(ENABLE_NDA "Use NDA submodules" ON)
option(ENABLE_COVERAGE "Test coverage" OFF)
option(CI "Continuous integration build" OFF)
option(DEVELOP "Development build" ON)

# ci variables
if(CI OR DEFINED ENV{CI})
  set(ENABLE_PROFILING OFF)
  set(DEVELOP ON)
endif()

# compiler flags
if (MSVC)
    add_compile_options(/W1 /favor:AMD64 /arch:AVX2 /openmp /MT)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -march=native)
    find_package(OpenMP)
    if (OpenMP_FOUND)
        add_compile_options(-fopenmp)
    endif()
else()
    add_compile_options(-Wall -march=native -fopenmp)
endif()

if(ENABLE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(COVERAGE_FLAGS "-O0" "-g" "--coverage" "-fprofile-arcs" "-ftest-coverage")
    add_compile_options(${COVERAGE_FLAGS})
    add_link_options("--coverage")
  endif()
endif()

# definitions
if (DEVELOP)
  add_compile_definitions(PROJECT_DIRECTORY="${CMAKE_SOURCE_DIR}" DEVELOP)
else()
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  add_compile_definitions(PROJECT_DIRECTORY="")
endif()

# ignore warnings from external sources
file(GLOB_RECURSE SRC_LIBS CONFIGURE_DEPENDS libs/*.c libs/*.cpp)
if(MSVC)
  set_source_files_properties(${SRC_LIBS} PROPERTIES COMPILE_OPTIONS /w)
else()
  set_source_files_properties(${SRC_LIBS} PROPERTIES COMPILE_OPTIONS -w)
endif()

# ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

# sanitizers
if(ENABLE_SANITIZER_ADDRESS)
  if(MSVC)
    add_compile_options(/fsanitize=address /Zi /EHsc /Oy-)
    add_link_options(/debug)
  else()
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
  endif()
endif()

if(ENABLE_SANITIZER_UNDEFINED AND NOT MSVC)
  add_compile_options(-fsanitize=undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=undefined)
endif()

if(ENABLE_SANITIZER_THREAD AND NOT MSVC)
  add_compile_options(-fsanitize=thread -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=thread)
endif()

if(ENABLE_SANITIZER_MEMORY AND NOT MSVC)
  add_compile_options(-fsanitize=memory -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=thread)
endif()

# status messages
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})
message(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER_ID} " " ${CMAKE_C_COMPILER_VERSION})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})
message(STATUS "CMAKE_CXX_COMPILER_LAUNCHER: " ${CMAKE_CXX_COMPILER_LAUNCHER})
message(STATUS "ENABLE_PROFILING: " ${ENABLE_PROFILING})
message(STATUS "ENABLE_SANITIZER_ADDRESS: " ${ENABLE_SANITIZER_ADDRESS})
message(STATUS "ENABLE_SANITIZER_UNDEFINED: " ${ENABLE_SANITIZER_UNDEFINED})
message(STATUS "ENABLE_SANITIZER_THREAD: " ${ENABLE_SANITIZER_THREAD})
message(STATUS "ENABLE_SANITIZER_MEMORY: " ${ENABLE_SANITIZER_MEMORY})
message(STATUS "ENABLE_PRIVATE: " ${ENABLE_PRIVATE})
message(STATUS "OPENCV_DIR: " ${OpenCV_DIR})
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message(STATUS "CMAKE_EXPORT_COMPILE_COMMANDS: " ${CMAKE_EXPORT_COMPILE_COMMANDS})
message(STATUS "ENABLE_COVERAGE: " ${ENABLE_COVERAGE})
message(STATUS "CI: " ${CI})
message(STATUS "DEVELOP: " ${DEVELOP})

# targets
add_executable(shenanigans)
add_executable(shenanigans_test)

# precompiled headers
if (ENABLE_PCH)
  set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
  set(SRC_PRECOMPILED_SHENANIGANS ${CMAKE_SOURCE_DIR}/apps/shenanigans/Precompiled.hpp)
  set(SRC_PRECOMPILED_GUI ${CMAKE_SOURCE_DIR}/src/PrecompiledGui.hpp)
  target_precompile_headers(shenanigans PRIVATE ${SRC_PRECOMPILED_GUI} ${SRC_PRECOMPILED_SHENANIGANS})
  target_precompile_headers(shenanigans_test PRIVATE ${SRC_PRECOMPILED_SHENANIGANS})
endif()

#openmp
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(shenanigans PRIVATE OpenMP::OpenMP_CXX)
    target_link_libraries(shenanigans_test PRIVATE OpenMP::OpenMP_CXX)
endif()

# pybind11
add_subdirectory(libs/pybind11)
include_directories(libs/pybind11/include)
target_link_libraries(shenanigans PRIVATE pybind11::embed)
target_link_libraries(shenanigans_test PRIVATE pybind11::embed)

# fmt
add_subdirectory(libs/fmt)
include_directories(libs/fmt/include)
target_link_libraries(shenanigans PRIVATE fmt::fmt)
target_link_libraries(shenanigans_test PRIVATE fmt::fmt)

# gtest
add_subdirectory(libs/gtest)
target_include_directories(shenanigans_test PRIVATE libs/gtest/googletest/include)
enable_testing()
include(GoogleTest)
target_link_libraries(shenanigans_test PRIVATE GTest::gtest)

# json
add_subdirectory(libs/json)
include_directories(libs/json/include)
target_link_libraries(shenanigans PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(shenanigans_test PRIVATE nlohmann_json::nlohmann_json)

# tracy
if(ENABLE_PROFILING)
  include_directories(libs/tracy/public)
  add_compile_definitions(TRACY_ENABLE) # for tracy macros
  add_compile_definitions(ENABLE_PROFILING) # for custom macros
  add_compile_definitions(TRACY_NO_FRAME_IMAGE) # not using frame images
  if(ENABLE_PROFILING_MEMORY)
    add_compile_definitions(ENABLE_PROFILING_MEMORY)
  endif()
  set(SRC_TRACY ${CMAKE_SOURCE_DIR}/libs/tracy/public/TracyClient.cpp)
  target_sources(shenanigans PRIVATE ${SRC_TRACY})
  target_link_libraries(shenanigans PRIVATE ${CMAKE_DL_LIBS})
endif()

# opencv
set(OpenCV_STATIC ON)
find_package(OpenCV REQUIRED HINTS ${OPENCV_DIR} ${CMAKE_SOURCE_DIR}/libs/opencv_install)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(shenanigans PRIVATE ${OpenCV_LIBS})
target_link_libraries(shenanigans_test PRIVATE ${OpenCV_LIBS})

# glad
add_subdirectory(libs/glad)
target_link_libraries(shenanigans PRIVATE glad)
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLAD)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)
target_link_libraries(shenanigans PRIVATE glfw)

# imgui
include_directories(libs/imgui)
include_directories(libs/imgui/misc/cpp)
include_directories(libs/imgui/backends)
file(GLOB SRC_IMGUI CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/libs/imgui/*.cpp ${CMAKE_SOURCE_DIR}/libs/imgui/misc/cpp/imgui_stdlib.cpp)
set(SRC_IMGUI ${SRC_IMGUI} CACHE INTERNAL "ImGui source files")
set(SRC_IMGUI_BACKEND ${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_glfw.cpp ${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl3.cpp CACHE INTERNAL "ImGui backend source files")
target_sources(shenanigans PRIVATE ${SRC_IMGUI} ${SRC_IMGUI_BACKEND})

# implot
include_directories(libs/implot)
file(GLOB SRC_IMPLOT CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/libs/implot/*.cpp)
target_sources(shenanigans PRIVATE ${SRC_IMPLOT})

# imgui-nodes
include_directories(libs/imgui-nodes)
file(GLOB SRC_IMGUI_NODES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/libs/imgui-nodes/*.cpp)
set(SRC_IMGUI_NODES ${SRC_IMGUI_NODES} CACHE INTERNAL "ImGui Nodes source files")
file(GLOB SRC_IMGUI_NODES_EXTRA CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/libs/imgui-nodes/examples/blueprints-example/utilities/drawing.cpp ${CMAKE_SOURCE_DIR}/libs/imgui-nodes/examples/blueprints-example/utilities/widgets.cpp)
set(SRC_IMGUI_NODES_EXTRA ${SRC_IMGUI_NODES_EXTRA} CACHE INTERNAL "ImGui Nodes extra source files")
target_sources(shenanigans PRIVATE ${SRC_IMGUI_NODES} ${SRC_IMGUI_NODES_EXTRA})

# range-v3
add_subdirectory(libs/range-v3)
target_link_libraries(shenanigans PRIVATE range-v3)
target_link_libraries(shenanigans_test PRIVATE range-v3)

# onnxruntime
set(ONNXRUNTIME_DIR "${CMAKE_SOURCE_DIR}/libs/onnxruntime_install" CACHE INTERNAL "ONNX runtime dir")
if (MSVC)
    set(ONNXRUNTIME_LIBS ${ONNXRUNTIME_DIR}/lib/onnxruntime.lib CACHE INTERNAL "ONNX runtime lib")
else()
    set(ONNXRUNTIME_LIBS ${ONNXRUNTIME_DIR}/lib/libonnxruntime.so CACHE INTERNAL "ONNX runtime lib")
endif()
include_directories(${ONNXRUNTIME_DIR}/include)
target_link_libraries(shenanigans PRIVATE ${ONNXRUNTIME_LIBS})
target_link_libraries(shenanigans_test PRIVATE ${ONNXRUNTIME_LIBS})

# custom
include_directories(libs)
include_directories(src)

# shenanigans sources
file(GLOB_RECURSE SRC_SHENANIGANS CONFIGURE_DEPENDS
 src/Astrophysics/*.cpp
 src/ImageProcessing/*.cpp
 src/ImageRegistration/*.cpp
 src/Log/*.cpp
 src/Math/*.cpp
 src/Microservice/*.cpp
 src/ONNX/*.cpp
 src/Optimization/*.cpp
 src/Plot/*.cpp
 src/Python/*.cpp
 src/Random/*.cpp
 src/Utils/*.cpp
)

# gui soruces
file(GLOB_RECURSE SRC_GUI CONFIGURE_DEPENDS src/Gui/*.cpp)

# shenanigans app sources
file(GLOB_RECURSE SRC_SHENANIGANS_APP CONFIGURE_DEPENDS apps/shenanigans/*.cpp apps/shenanigans/Windows/*.cpp)
target_sources(shenanigans PRIVATE ${SRC_SHENANIGANS} ${SRC_GUI} ${SRC_SHENANIGANS_APP})

# shenanigans_test app sources
file(GLOB_RECURSE SRC_SHENANIGANS_TEST CONFIGURE_DEPENDS apps/shenanigans_test/*.cpp src/Test/*.cpp)
target_sources(shenanigans_test PRIVATE ${SRC_SHENANIGANS} ${SRC_SHENANIGANS_TEST})
gtest_discover_tests(shenanigans_test)

if (ENABLE_NDA)
  add_subdirectory(src/NDA)
  add_compile_definitions(ENABLE_NDA)
  file(GLOB_RECURSE SRC_SHENANIGANS_NDA CONFIGURE_DEPENDS src/NDA/SAS/*.cpp)
  file(GLOB_RECURSE SRC_SHENANIGANS_APP_NDA CONFIGURE_DEPENDS src/NDA/Windows/*.cpp)
  target_sources(shenanigans PRIVATE ${SRC_SHENANIGANS_NDA} ${SRC_SHENANIGANS_APP_NDA})
endif()
