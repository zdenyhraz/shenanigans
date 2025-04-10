import os
from . import utils
from script.log import log


opencv_install_name = 'opencv_install'
opencv_install_dir = os.path.join(utils.get_root_directory(), 'libs', opencv_install_name)
opencv_cmake_args = {
    'CMAKE_BUILD_TYPE': 'Release',
    'BUILD_SHARED_LIBS': 'OFF',
    'OPENCV_EXTRA_MODULES_PATH': '../opencv_contrib/modules',
    'OPENCV_ENABLE_NONFREE': 'ON',
    'BUILD_TESTS': 'OFF',
    'BUILD_PERF_TESTS': 'OFF',
    'BUILD_OPENCV_WECHAT_QRCODE': 'OFF',
    'BUILD_OPENCV_JAVA': 'OFF',
    'BUILD_OPENCV_APPS': 'OFF',
    'BUILD_OPENCV_PYTHON': 'OFF',
    'OPENCV_PYTHON_SKIP': 'ON',
    'BUILD_OPENCV_DNN': 'OFF',
    'BUILD_OPENCV_ML': 'OFF',
    'BUILD_OPENCV_OBJDETECT': 'OFF',
    'BUILD_OPENCV_PHOTO': 'OFF',
    'BUILD_OPENCV_VIDEO': 'OFF',
    'BUILD_OPENCV_VIDEOIO': 'ON',
    'BUILD_OPENCV_STITCHING': 'OFF',
    'BUILD_OPENCV_CALIB3D': 'OFF',
    'BUILD_OPENCV_SHAPE': 'OFF',
    'BUILD_OPENCV_SUPERRES': 'OFF',
    'BUILD_OPENCV_FLANN': 'OFF',
    'BUILD_OPENCV_HIGHGUI': 'ON',
    'BUILD_OPENCV_IMGCODECS': 'ON',
    'BUILD_OPENCV_IMGPROC': 'ON',
    'BUILD_OPENCV_CORE': 'ON',
    'BUILD_OPENCV_CONTRIB': 'OFF',
    'BUILD_OPENCV_TS': 'OFF',
}


def opencv_find_root():
    candidates = []
    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, _, files in os.walk(opencv_install_dir):
            for file in files:
                if file == 'OpenCVConfig.cmake':
                    candidates.append(root)
    if False and len(candidates) > 1:
        log.debug(f'Found {len(candidates)} opencv candidates: {candidates}')
    return '' if len(candidates) == 0 else max(candidates, key=len)  # return the deepest if multiple found


def opencv_install():
    log.debug(f"Installing opencv to {opencv_install_dir}")
    cwd = os.path.join(utils.get_root_directory(), 'libs/opencv')
    os.makedirs(os.path.join(cwd, 'build'), exist_ok=True)
    utils.run(f'cmake -B ./build {utils.generate_configure_args(opencv_cmake_args)}', cwd)
    utils.run(f'cmake --build ./build --config Release --parallel', cwd)
    utils.run(f'cmake --install ./build --prefix ../{opencv_install_name}', cwd)


def opencv_installed():
    return os.path.isdir(opencv_install_dir) and any(os.scandir(opencv_install_dir))


def setup(build_type):
    if not opencv_installed():
        opencv_install()

    opencv_dir = opencv_find_root()
    log.debug(f'opencv directory: {opencv_dir}')
    return opencv_dir
