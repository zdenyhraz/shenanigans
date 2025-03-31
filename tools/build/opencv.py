import os
import platform
from . import utils

opencv_install_name = 'opencv_install'
opencv_install_dir = os.path.join(utils.get_root_directory(), 'libs', opencv_install_name)
opencv_cmake_args = {
    'CMAKE_BUILD_TYPE': 'Release',
    'BUILD_SHARED_LIBS': 'OFF',
    'OPENCV_EXTRA_MODULES_PATH': '../opencv_contrib/modules',
    'OPENCV_ENABLE_NONFREE': 'ON',
    'BUILD_TESTS': 'OFF',
    'BUILD_PERF_TESTS': 'OFF',
    'BUILD_opencv_wechat_qrcode': 'OFF',
    'BUILD_opencv_java': 'OFF',
    'BUILD_opencv_apps': 'OFF',
    'BUILD_opencv_python': 'OFF',
    'BUILD_opencv_python': 'OFF',
    'BUILD_opencv_python2': 'OFF',
    'BUILD_opencv_python3': 'OFF',
    'OPENCV_PYTHON_SKIP': 'ON',
    'PYTHON_EXECUTABLE': '',
    'PYTHON_LIBRARY': ''
}


def opencv_find_root():
    candidates = []
    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, _, files in os.walk(opencv_install_dir):
            for file in files:
                if file == 'OpenCVConfig.cmake':
                    candidates.append(root)
    if len(candidates) > 1:
        print(f'Found {len(candidates)} opencv candidates: {candidates}')
    return '' if len(candidates) == 0 else max(candidates, key=len)  # return the deepest if multiple found


def opencv_install():
    print(f"Installing opencv to {opencv_install_dir}")
    cwd = os.path.join(utils.get_root_directory(), 'libs/opencv')
    os.makedirs(os.path.join(cwd, 'build'), exist_ok=True)
    utils.run(f'cmake -B ./build {utils.generate_configure_args(opencv_cmake_args)}', cwd)
    utils.run(f'cmake --build ./build --config Release --parallel', cwd)
    utils.run(f'cmake --install ./build --prefix ../{opencv_install_name}', cwd)
    return opencv_find_root()


def opencv_installed():
    return os.path.isdir(opencv_install_dir) and any(os.scandir(opencv_install_dir))


def setup(build_type):
    if not opencv_installed():
        opencv_install()

    opencv_dir = opencv_find_root()
    print('opencv cmake directory: ', opencv_dir)
    return opencv_dir
