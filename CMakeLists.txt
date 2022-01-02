cmake_minimum_required(VERSION 3.7)
project(shenanigans)

# cpp
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -fopenmp") # optimizations
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wfatal-errors -Wall -Wextra -Wpedantic -Wshadow") # warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter") # excluded warnings

# fmt
add_subdirectory(external/fmt)
include_directories("external/fmt/include")

# gtest
add_subdirectory(external/gtest)
include_directories("external/gtest/googletest/include")
include_directories("external/gtest/googlemock/include")
enable_testing()
include(GoogleTest)

# opencv
find_package(OpenCV REQUIRED PATHS D:\\include\\opencv\\build\\install-mingw)
include_directories(${OpenCV_INCLUDE_DIRS})

# qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(APP_ICON_RESOURCE_WINDOWS "resources/shenanigans.rc")
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()
find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport REQUIRED PATHS C:\\Qt\\5.12.2\\mingw73_64)

# custom
include_directories("src")
include_directories("resources")

# the shenanigans executable
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.h src/*.cpp src/*.ui src/*.qrc)
add_executable(shenanigans ${SRC} ${SRC_TEST} ${APP_ICON_RESOURCE_WINDOWS})
target_link_libraries(shenanigans PRIVATE ${OpenCV_LIBS} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport fmt::fmt-header-only)
target_precompile_headers(shenanigans PRIVATE src/Precompiled/Precompiled.h)

# the shenanigans_test executable
file(GLOB_RECURSE SRC_TEST CONFIGURE_DEPENDS test/*.h test/*.cpp)
add_executable(shenanigans_test ${SRC_TEST})
target_link_libraries(shenanigans_test PRIVATE ${OpenCV_LIBS} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport fmt::fmt-header-only GTest::gtest_main GTest::gmock)
gtest_discover_tests(shenanigans_test)






