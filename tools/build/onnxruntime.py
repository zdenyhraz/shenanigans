import zipfile
import tarfile
import os
import platform
import urllib.request
import shutil
from . import utils

onnxruntime_install_name = "onnxruntime_install"
onnxruntime_install_dir = os.path.join(utils.get_root_directory(), 'libs', onnxruntime_install_name)


def onnxruntime_get_url():
    if utils.linux():
        return "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-linux-x64-gpu-1.20.1.tgz"
    elif utils.windows():
        return "https://github.com/microsoft/onnxruntime/releases/download/v1.20.1/onnxruntime-win-x64-gpu-1.20.1.zip"
    raise RuntimeError(f"Unsupported onnxruntime platform: {platform.system()}")


def onnxruntime_download():
    url = onnxruntime_get_url()
    filename, extension = os.path.splitext(url.split('/')[-1])
    archive_file = f"onnxruntime{extension}"
    temp_extract_dir = "temp_onnxruntime_extract"

    os.makedirs(onnxruntime_install_dir, exist_ok=True)
    print(f"Downloading onnxruntime from {url}")
    urllib.request.urlretrieve(url, archive_file)

    print(f"Extracting onnxruntime to temporary directory {temp_extract_dir}")
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
        print(f"Moving {src} to {dst}")
        shutil.move(src, dst)

    print(f"Cleaning up temporary directory {temp_extract_dir}")
    shutil.rmtree(temp_extract_dir)
    print(f"Cleaning up onnxruntime archive {archive_file}")
    os.remove(archive_file)


def onnxruntime_install():
    print(f'Installing onnxruntime')
    cwd = os.path.join(utils.get_root_directory(), 'libs/onnxruntime')
    os.makedirs(os.path.join(cwd, 'build'), exist_ok=True)
    utils.run(f'python tools\ci_build\build.py --build_dir {onnxruntime_install_dir} --parallel --config Release --enable_msvc_static_runtime', cwd)


def onnxruntime_installed():
    return os.path.isdir(onnxruntime_install_dir) and any(os.scandir(onnxruntime_install_dir))


def setup(build_type):
    if not onnxruntime_installed():
        onnxruntime_install()  # onnxruntime_download()

    return onnxruntime_install_dir
