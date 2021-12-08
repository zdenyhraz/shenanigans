cmake_minimum_required(VERSION 3.0)
project(shenanigans)

# cpp
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp") # OpenMP support
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra -Wpedantic") # warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter") # excluded warnings

# qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(APP_ICON_RESOURCE_WINDOWS "src/Resources/shenanigans.rc")

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport REQUIRED PATHS C:\\Qt\\5.12.2\\mingw73_64)

# fmt
add_subdirectory(external/fmt)
include_directories("external/fmt/include")

# opencv
find_package(OpenCV REQUIRED PATHS D:\\include\\opencv\\build\\install-mingw)
include_directories(${OpenCV_INCLUDE_DIRS})


# the shenanigans executable
include_directories("src")
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.h src/*.cpp src/*.ui src/*.qrc)
add_executable(shenanigans ${SRC} ${APP_ICON_RESOURCE_WINDOWS})
target_link_libraries(shenanigans PRIVATE ${OpenCV_LIBS} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport fmt::fmt-header-only)
target_precompile_headers(shenanigans PRIVATE src/Application/Precompiled.h)



