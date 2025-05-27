import zipfile
import tarfile
import os
import platform
import urllib.request
import shutil
from . import utils
from script.log import log


onnxruntime_install_name = "onnxruntime_install"
onnxruntime_install_dir = os.path.join(utils.get_root_directory(), 'libs', onnxruntime_install_name)
onnxruntime_build = True


def onnxruntime_get_url():
    if utils.linux():
        return "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-x64-gpu-1.20.1.tgz"
    elif utils.windows():
        return "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-win-x64-gpu-1.20.1.zip"
    elif utils.macos():
        return "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-osx-universal2-1.20.1.tgz"
    raise RuntimeError(f"Unsupported onnxruntime system/machine: {platform.system()}/{platform.machine()}")


def onnxruntime_download():
    url = onnxruntime_get_url()
    filename, extension = os.path.splitext(url.split('/')[-1])
    archive_file = f"onnxruntime{extension}"
    temp_extract_dir = "temp_onnxruntime_extract"

    os.makedirs(onnxruntime_install_dir, exist_ok=True)
    log.debug(f"Downloading onnxruntime from {url}")
    urllib.request.urlretrieve(url, archive_file)

    log.debug(f"Extracting onnxruntime to temporary directory {temp_extract_dir}")
    os.makedirs(temp_extract_dir, exist_ok=True)
    if extension == ".tgz":
        with tarfile.open(archive_file, 'r:gz') as tar_ref:
            tar_ref.extractall(temp_extract_dir)
    elif extension == ".zip":
        with zipfile.ZipFile(archive_file, 'r') as zip_ref:
            zip_ref.extractall(temp_extract_dir)

    extracted_dir = os.path.join(temp_extract_dir, os.listdir(temp_extract_dir)[0])
    for item in os.listdir(extracted_dir):
        src = os.path.join(extracted_dir, item)
        dst = os.path.join(onnxruntime_install_dir, item)
        log.debug(f"Moving {src} to {dst}")
        shutil.move(src, dst)

    log.debug(f"Cleaning up temporary directory {temp_extract_dir}")
    shutil.rmtree(temp_extract_dir)
    log.debug(f"Cleaning up onnxruntime archive {archive_file}")
    os.remove(archive_file)


def onnxruntime_install(jobs):
    log.debug(f"Installing onnxruntime to {onnxruntime_install_dir}")
    cwd = os.path.join(utils.get_root_directory(), 'libs', 'onnxruntime')
    os_args = '--enable_msvc_static_runtime' * utils.windows() + '--allow_running_as_root' * utils.linux()
    cuda_args = f'--use_cuda --cuda_home "{os.getenv("CUDA_PATH")}"' * utils.cuda_available()
    utils.run(
        f"python tools/ci_build/build.py --config Release --build_shared_lib --build_dir build --parallel {jobs} --update --build --skip_tests --compile_no_warning_as_error {os_args} {cuda_args}", cwd)
    utils.run(f'cmake --install ./build/Release --prefix ../{onnxruntime_install_name}', cwd)


def onnxruntime_installed():
    return os.path.isdir(onnxruntime_install_dir) and any(os.scandir(onnxruntime_install_dir))


def setup(build_type, jobs):
    if not onnxruntime_installed():
        onnxruntime_install(jobs) if onnxruntime_build else onnxruntime_download()

    utils.copy_files_to_directory(utils.find_binaries(onnxruntime_install_dir), utils.get_runtime_directory(build_type))
    log.debug(f'onnxruntime directory: {onnxruntime_install_dir}')
    return onnxruntime_install_dir
