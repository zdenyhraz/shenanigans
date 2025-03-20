import zipfile
import tarfile
import os
import platform
import urllib.request
from . import utils

onnxruntime_install_name = "onnxruntime"
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

    os.makedirs(onnxruntime_install_dir, exist_ok=True)
    print(f"Downloading onnxruntime from {url}")
    urllib.request.urlretrieve(url, archive_file)

    print(f"Extracting onnxruntime to {onnxruntime_install_dir}")
    if extension == ".tgz":
        with tarfile.open(archive_file, 'r:gz') as tar_ref:
            tar_ref.extractall(onnxruntime_install_dir)
    elif extension == ".zip":
        with zipfile.ZipFile(archive_file, 'r') as zip_ref:
            zip_ref.extractall(onnxruntime_install_dir)

    print(f"Cleaning up onnxruntime archive {archive_file}")
    os.remove(archive_file)


def onnxruntime_installed():
    return os.path.isdir(onnxruntime_install_dir) and any(os.scandir(onnxruntime_install_dir))


def setup(build_type):
    if not onnxruntime_installed():
        onnxruntime_download()

    binaries = utils.find_binaries(onnxruntime_install_dir)
    utils.copy_files_to_directory(binaries, utils.get_runtime_directory(build_type))
    return onnxruntime_install_dir
