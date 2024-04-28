import subprocess
import os
import platform


def run(command, cwd=None):
    print(f"Running command{f' in subdirectory {cwd}' if cwd else ''}: {command}")
    subprocess.run(command, shell=True, check=True, cwd=cwd if cwd else None)


def is_linux_executable(file):
    return os.path.splitext(file)[1] == '' and os.access(file, os.X_OK)


def is_windows_executable(file):
    return os.path.splitext(file)[1] == ".exe"


def is_executable(file):
    if not os.path.isfile(file):
        return False
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
                if '_test' in file:
                    executable = os.path.join(root, file)
                    if is_executable(executable):
                        test_executables.append(executable)

    return test_executables


def find_library_binaries(build_dir):
    library_binaries = []
    if os.path.exists(build_dir) and os.path.isdir(build_dir):
        for root, dirs, files in os.walk(build_dir):
            for file in files:
                if '.so' in file or '.dll' in file:
                    library_binaries.append(os.path.join(root, file))

    return library_binaries


if __name__ == '__main__':
    root_dir = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(root_dir, 'build')
    test_dir = os.path.join(root_dir, 'test')

    check_build_dir(build_dir)
    check_test_data_dir(test_dir)

    # run('ctest --output-on-failure', build_dir)
    test_executables = find_test_executables(build_dir)
    print('Library binaries: ', find_library_binaries(build_dir))
    print('Test executables: ', test_executables)
    for test in test_executables:
        run(test)
