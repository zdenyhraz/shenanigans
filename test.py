import subprocess
import os
import platform


def run(command, cwd=None):
    print(f"Running command{f' in subdirectory {cwd}' if cwd else ''}: {command}")
    subprocess.run(command, shell=True, check=True, cwd=cwd if cwd else None)


def is_linux_executable(file):
    return os.access(file, os.X_OK) and os.path.isfile(file)


def is_windows_executable(file):
    return os.path.splitext(file)[1].lower() == ".exe"


def is_executable(file):
    if platform.system() == 'Linux':
        return is_linux_executable(file)
    if platform.system() == 'Windows':
        return is_windows_executable(file)


def check_build_dir(build_dir):
    if os.path.exists(build_dir):
        return

    raise RuntimeError(f'Cannot find build directory {build_dir}')


def check_test_data_dir(test_dir):
    if os.path.exists(test_dir) and os.path.isdir(test_dir):
        for root, dirs, files in os.walk(test_dir):
            for file in files:
                if file == 'baboon.png':
                    print(f'Test data directory: {root}')
                    return

    raise RuntimeError('Cannot not find test data directory')


def find_test_executables(build_dir):
    test_executables = []
    if os.path.exists(build_dir) and os.path.isdir(build_dir):
        for root, dirs, files in os.walk(build_dir):
            for file in files:
                if '_test' in file and is_executable(file):
                    test_executables.append(os.path.join(root, file))

    return test_executables


if __name__ == '__main__':
    root_dir = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(root_dir, 'build')
    test_dir = os.path.join(root_dir, 'test')

    check_build_dir(build_dir)
    check_test_data_dir(test_dir)

    # run('ctest --output-on-failure', build_dir)
    test_executables = find_test_executables(build_dir)
    for test in test_executables:
        run(f'{test}')
