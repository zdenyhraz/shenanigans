import os
import tools.build as build


def check_data_dir(test_dir):
    if os.path.exists(test_dir) and os.path.isdir(test_dir):
        for root, _, files in os.walk(test_dir):
            for file in files:
                if file == 'baboon.png':
                    print(f'Test data directory: {root}')
                    return
    raise RuntimeError('Cannot find test data directory')


def find_test_executables(build_dir):
    if not os.path.exists(build_dir) or not os.path.isdir(build_dir):
        raise RuntimeError(f'Cannot find test build directory {build_dir}')
    test_executables = []
    for root, _, files in os.walk(build_dir):
        for file in files:
            if '_test' in file:
                executable = os.path.join(root, file)
                if build.utils.is_executable(executable):
                    test_executables.append(executable)
    if not test_executables:
        raise RuntimeError(f'Cannot find any test executables in {build_dir}')
    return test_executables


if __name__ == '__main__':
    root_dir = build.utils.get_root_directory()
    build_dir = os.path.join(root_dir, 'build')
    test_dir = os.path.join(root_dir, 'test')

    check_data_dir(test_dir)

    # run('ctest --output-on-failure', build_dir)
    test_executables = find_test_executables(build_dir)
    print('Test executables: ', test_executables)
    for test in test_executables:
        build.utils.run(test)
