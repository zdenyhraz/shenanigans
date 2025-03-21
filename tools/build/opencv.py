import os
import platform
from . import utils

opencv_install_name = 'opencv_install'
opencv_install_dir = os.path.join(utils.get_root_directory(), 'libs', opencv_install_name)
opencv_cmake_args = {
    'CMAKE_BUILD_TYPE': 'Release',
    'OPENCV_EXTRA_MODULES_PATH': '../opencv_contrib/modules',
    'OPENCV_ENABLE_NONFREE': 'ON',
    'BUILD_TESTS': 'OFF',
    'BUILD_opencv_python': 'OFF',
    'BUILD_opencv_java': 'OFF',
    'BUILD_opencv_apps': 'OFF'
}


def opencv_find_root():
    candidates = []
    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, _, files in os.walk(opencv_install_dir):
            for file in files:
                if file == 'OpenCVConfig.cmake':
                    candidates.append(root)
    if len(candidates) > 1:
        print(f'Found {len(candidates)} OpenCV candidates: {candidates}')
    return '' if len(candidates) == 0 else max(candidates, key=len)  # return the deepest if multiple found


def opencv_install(jobs):
    print(f'Installing OpenCV: {platform.system()}/-j{jobs}/opencv_configure_args={utils.generate_configure_args(opencv_cmake_args)}')
    cwd = os.path.join(utils.get_root_directory(), 'libs/opencv')
    os.makedirs(os.path.join(cwd, 'build'), exist_ok=True)
    utils.run(f'cmake -B ./build {utils.generate_configure_args(opencv_cmake_args)}', cwd)
    utils.run(f'cmake --build ./build --config Release -j {jobs}', cwd)
    utils.run(f'cmake --install ./build --prefix ../{opencv_install_name}', cwd)
    return opencv_find_root()


def setup(jobs, build_type):
    opencv_dir = opencv_find_root()
    if not opencv_dir:
        opencv_dir = opencv_install(jobs)
    if not opencv_dir:
        raise RuntimeError("Unable to find installed OpenCV CMake directory")
    print('OpenCV cmake directory: ', opencv_dir)

    binaries = utils.find_binaries(opencv_install_dir)
    utils.copy_files_to_directory(binaries, utils.get_runtime_directory(build_type))
    return opencv_dir
