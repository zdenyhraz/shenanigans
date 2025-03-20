import subprocess
import os
import shutil
import platform


def run(command, cwd=None):
    print(f"Running command '{command}'{f' in subdirectory {cwd}' if cwd else ''}")
    try:
        result = subprocess.run(command, shell=True, check=True, cwd=cwd if cwd else None, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        print(result.stdout)
    except subprocess.CalledProcessError as e:
        print(f"Command '{command}' failed with exit code {e.returncode}")
        print(f"Error output: {e.stderr}")
        raise


def linux():
    return platform.system() == 'Linux'


def windows():
    return platform.system() == 'Windows'


def generate_configure_args(configure_args):
    return ' '.join([f"-D{key}={value}" for key, value in configure_args.items()])


def copy_files_to_directory(file_paths, destination_directory):
    os.makedirs(destination_directory, exist_ok=True)
    for file_path in file_paths:
        file_name = os.path.basename(file_path)
        destination_path = os.path.join(destination_directory, file_name)
        shutil.copy(file_path, destination_path)
        print(f"File '{file_path}' copied to '{destination_directory}'")


def find_binaries(dir):
    binaries = []
    target_extension = '.so' if linux() else '.dll' if windows() else None
    for root, _, files in os.walk(dir):
        for file in files:
            filename, extension = os.path.splitext(file)
            if extension == target_extension:
                binaries.append(os.path.join(root, file))
    return binaries


def get_root_directory(start_dir=os.getcwd()):
    if 'build.py' in os.listdir(start_dir):
        return start_dir
    parent_dir = os.path.dirname(start_dir)
    if parent_dir == start_dir:
        return None
    return get_root_directory(parent_dir)


def get_runtime_directory(build_type):
    if linux():
        return os.path.join(get_root_directory(), 'build')
    if windows():
        return os.path.join(get_root_directory(), 'build', build_type)


def is_linux_executable(file):
    return os.path.splitext(file)[1] == '' and os.access(file, os.X_OK)


def is_windows_executable(file):
    return os.path.splitext(file)[1] == ".exe"


def is_executable(file):
    if not os.path.isfile(file):
        return False
    if linux():
        return is_linux_executable(file)
    if windows():
        return is_windows_executable(file)
