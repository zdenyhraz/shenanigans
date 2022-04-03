cmake_minimum_required(VERSION 3.7)
project(shenanigans)

# variables
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(ENABLE_PROFILING OFF)
set(ENABLE_SANITIZER OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ENABLE_PCH ON)

# ci variables
if(DEFINED ENV{CI})
  set(ENABLE_PROFILING OFF)
  set(ENABLE_SANITIZER OFF)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  set(CMAKE_UNITY_BUILD OFF)
endif()

# compiler
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /favor:INTEL64 /arch:SSE /arch:SSE2 /arch:AVX /arch:AVX2 /openmp")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /WX")
else()
  set(CXX_FLAGS "-march=native -fopenmp -Wall -Werror -Wfatal-errors -Wextra -Wpedantic -Wshadow -Wno-unused-parameter -Wno-missing-field-initializers")
  set(CXX_FLAGS_DEBUG "-g -Og")
  set(CXX_FLAGS_RELEASE "-O3")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CXX_FLAGS_DEBUG}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CXX_FLAGS_RELEASE}")

  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -w")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -w")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -w")
endif()

# ignore warnings from libs sources
file(GLOB_RECURSE SRC_LIBS CONFIGURE_DEPENDS libs/*.h libs/*.hpp libs/*.c libs/*.cpp)
set_source_files_properties(${SRC_LIBS} PROPERTIES COMPILE_FLAGS "-w")

# ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
endif()

# sanitizer
if(ENABLE_SANITIZER)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address -fno-omit-frame-pointer)
endif()

# status messages
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})
message(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER_ID} " " ${CMAKE_C_COMPILER_VERSION})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE})
message(STATUS "ENABLE_PROFILING: " ${ENABLE_PROFILING})
message(STATUS "ENABLE_SANITIZER: " ${ENABLE_SANITIZER})

# targets
add_executable(shenanigans)
add_executable(shenanigans_test)

# pybind11
find_package(PythonLibs REQUIRED)
add_subdirectory(libs/pybind11)
include_directories(${PYTHON_INCLUDE_DIRS})
include_directories(libs/pybind11/include)
target_link_libraries(shenanigans PRIVATE pybind11::embed)
target_link_libraries(shenanigans_test PRIVATE pybind11::embed)

# fmt
add_subdirectory(libs/fmt)
include_directories(libs/fmt/include)
target_link_libraries(shenanigans PRIVATE fmt::fmt-header-only)
target_link_libraries(shenanigans_test PRIVATE fmt::fmt-header-only)

# gtest
add_subdirectory(libs/gtest)
include_directories(libs/gtest/googletest/include)
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
  include_directories(libs/tracy)
  add_compile_definitions(TRACY_ENABLE) # for tracy macros
  add_compile_definitions(ENABLE_PROFILING) # for custom macros
  add_compile_definitions(TRACY_NO_FRAME_IMAGE) # not using frame images
  set(SRC_TRACY libs/tracy/Tracy.hpp libs/tracy/TracyClient.cpp)
  target_sources(shenanigans PRIVATE ${SRC_TRACY})
  target_sources(shenanigans_test PRIVATE ${SRC_TRACY})
  target_link_libraries(shenanigans PRIVATE ${CMAKE_DL_LIBS})
  target_link_libraries(shenanigans_test PRIVATE ${CMAKE_DL_LIBS})
endif()

# opencv
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(shenanigans PRIVATE ${OpenCV_LIBS})
target_link_libraries(shenanigans_test PRIVATE ${OpenCV_LIBS})

# torch
find_package(Torch REQUIRED HINTS libs/libtorch)
include_directories(libs/libtorch/include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
target_link_libraries(shenanigans PRIVATE ${TORCH_LIBRARIES})
target_link_libraries(shenanigans_test PRIVATE ${TORCH_LIBRARIES})

# imgui
include_directories(libs/imgui)
include_directories(libs/imgui/misc/cpp)
include_directories(libs/imgui/backends)
file(GLOB SRC_IMGUI CONFIGURE_DEPENDS libs/imgui/*.cpp libs/imgui/misc/cpp/imgui_stdlib.cpp)
set(SRC_IMGUI_BACKEND libs/imgui/backends/imgui_impl_glfw.cpp libs/imgui/backends/imgui_impl_opengl3.cpp)
target_sources(shenanigans PRIVATE ${SRC_IMGUI} ${SRC_IMGUI_BACKEND})
target_sources(shenanigans_test PRIVATE ${SRC_IMGUI} ${SRC_IMGUI_BACKEND})

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw)
target_link_libraries(shenanigans PRIVATE glfw)
target_link_libraries(shenanigans_test PRIVATE glfw)

# glad
add_subdirectory(libs/glad)
target_link_libraries(shenanigans PRIVATE glad)
target_link_libraries(shenanigans_test PRIVATE glad)

# custom
include_directories(src)

# sources
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.hpp src/*.cpp)
target_sources(shenanigans PRIVATE ${SRC})
target_sources(shenanigans_test PRIVATE ${SRC})

# shenanigans app sources
file(GLOB SRC_SHENANIGANS_APP CONFIGURE_DEPENDS apps/shenanigans/*.cpp)
target_sources(shenanigans PRIVATE ${SRC_SHENANIGANS_APP})

# shenanigans_test app sources
file(GLOB SRC_SHENANIGANS_TEST_APP CONFIGURE_DEPENDS apps/shenanigans_test/*.cpp test/*.hpp test/*.cpp)
target_sources(shenanigans_test PRIVATE ${SRC_SHENANIGANS_TEST_APP})
gtest_discover_tests(shenanigans_test)

# precompiled headers
if (ENABLE_PCH)
  set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)
  set(SRC_PRECOMPILED src/Precompiled.hpp)
  target_precompile_headers(shenanigans PRIVATE ${SRC_PRECOMPILED})
  target_precompile_headers(shenanigans_test PRIVATE ${SRC_PRECOMPILED})
endif()
