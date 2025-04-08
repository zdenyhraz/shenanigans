import os
import argparse
from script.setup import utils
from script.log import log


def find_test_executables(build_dir):
    if not os.path.exists(build_dir) or not os.path.isdir(build_dir):
        raise RuntimeError(f'Cannot find test build directory {build_dir}')
    test_executables = []
    for root, _, files in os.walk(build_dir):
        for file in files:
            if 'test' in file:
                executable = os.path.join(root, file)
                if utils.is_executable(executable):
                    test_executables.append(executable)
    if not test_executables:
        raise RuntimeError(f'Cannot find any test executables in {build_dir}')
    return test_executables


def run_cpp_tests():
    log.info('Running C++ tests')
    build_dir = 'build'
    log.debug(f'Build directory: {build_dir}')

    # run('ctest --output-on-failure', build_dir)
    test_executables = find_test_executables(build_dir)
    log.debug(f'Test executables: {test_executables}')
    for test in test_executables:
        utils.run(test)


def run_python_tests():
    log.info('Running Python tests')
    utils.run(f'pip install -r requirements.txt')
    test_dir = 'script/test'
    log.debug(f'Test directory: {test_dir}')
    utils.run(f'pytest -p no:warnings {test_dir}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Test script')
    parser.add_argument('--cpp', help='run C++ tests', required=False, action='store_true', default=True)
    parser.add_argument('--python', help='run Python tests', required=False, action='store_true', default=True)
    args = parser.parse_args()

    log.info('Running tests')
    for arg, val in vars(args).items():
        log.debug(f'{arg}: {val}')

    args.cpp and run_cpp_tests()
    args.python and run_python_tests()

    log.info('All tests passed')
