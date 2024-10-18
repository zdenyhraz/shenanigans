import os
import platform
from . import utils

opencv_install_name = 'opencv_install'
opencv_install_dir = os.path.join(utils.get_root_directory(), 'libs', opencv_install_name)
opencv_args = {
    'CMAKE_BUILD_TYPE': 'Release',
    'OPENCV_EXTRA_MODULES_PATH': '../opencv_contrib/modules',
    'OPENCV_ENABLE_NONFREE': 'ON',
    'BUILD_TESTS': 'OFF',
    'BUILD_opencv_python': 'OFF',
    'BUILD_opencv_java': 'OFF',
    'BUILD_opencv_apps': 'OFF'
}


def opencv_find_root(opencv_install_dir):
    candidates = []
    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, _, files in os.walk(opencv_install_dir):
            for file in files:
                if file == 'OpenCVConfig.cmake':
                    candidates.append(root)
    if len(candidates) > 1:
        print(f'Found {len(candidates)} OpenCV candidates: {candidates}')
    return '' if len(candidates) == 0 else max(candidates, key=len)  # return the deepest if multiple found


def opencv_find_binaries(opencv_install_dir):
    opencv_install_bin_dir = ''
    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, _, files in os.walk(opencv_install_dir):
            for file in files:
                if 'opencv_core' in file and (file.endswith('.so') or file.endswith('.dll')):
                    opencv_install_bin_dir = root
    if not opencv_install_bin_dir:
        raise RuntimeError('Cannot find OpenCV binary directory')
    print('OpenCV binary directory: ', opencv_install_bin_dir)
    bin_files = []
    for root, _, files in os.walk(opencv_install_bin_dir):
        for file in files:
            if '.so' in file or '.dll' in file:
                bin_files.append(os.path.join(root, file))
    return bin_files


def opencv_install(jobs, opencv_install_name, opencv_install_dir):
    print(f'Installing OpenCV: {platform.system()}/-j{jobs}/opencv_configure_args={utils.generate_configure_args(opencv_args)}')
    cwd = os.path.join(utils.get_root_directory(), 'libs/opencv')
    os.makedirs(os.path.join(cwd, 'build'), exist_ok=True)
    utils.run(f'cmake -B ./build {utils.generate_configure_args(opencv_args)}', cwd)
    utils.run(f'cmake --build ./build --config Release -j {jobs}', cwd)
    utils.run(f'{"sudo " if utils.linux() else ""}cmake --install ./build --prefix ../{opencv_install_name}', cwd)
    return opencv_find_root(opencv_install_dir)


def setup(jobs, build_type):
    opencv_dir = opencv_find_root(opencv_install_dir)
    if not opencv_dir:
        opencv_dir = opencv_install(jobs, opencv_install_name, opencv_install_dir)
    if not opencv_dir:
        raise RuntimeError("Unable to find installed OpenCV CMake directory")
    print('OpenCV cmake directory: ', opencv_dir)

    opencv_binaries = opencv_find_binaries(opencv_install_dir)
    if opencv_binaries:
        utils.copy_files_to_directory(opencv_binaries, utils.get_runtime_directory(build_type))
    else:
        print('Warning: Could not find any OpenCV binaries')
    return opencv_dir
