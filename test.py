import subprocess
import os


def run(command, cwd=None):
    print(f"Running command{f' in subdirectory {cwd}' if cwd else ''}: ", command)
    subprocess.run(command, shell=True, check=True, cwd=cwd if cwd else None)


def check_build_dir(build_dir):
    if os.path.exists(build_dir):
        print(f'Build directory: {build_dir}')
    else:
        raise RuntimeError(f'Cannot find build directory {build_dir}')


def check_test_data_dir(test_dir):
    if os.path.exists(test_dir) and os.path.isdir(test_dir):
        for root, dirs, files in os.walk(test_dir):
            for file in files:
                if file == 'baboon.png':
                    print(f'Test data directory: {root}')
                    return

    raise RuntimeError('Cannot not find test data directory')


if __name__ == '__main__':
    root_dir = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.join(root_dir, 'build')
    test_dir = os.path.join(root_dir, 'test')

    check_build_dir(build_dir)
    check_test_data_dir(test_dir)

    run('ctest --output-on-failure', build_dir)
