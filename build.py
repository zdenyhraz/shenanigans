import subprocess
import os
import platform
import multiprocessing
import argparse
import shutil


def run(command, cwd=None):
    print(f"Running command{f' in subdirectory {cwd}' if cwd else ''}: {command}")
    subprocess.run(command, shell=True, check=True, cwd=cwd if cwd else None)


def copy_files_to_directory(file_paths, destination_directory):
    os.makedirs(destination_directory, exist_ok=True)
    for file_path in file_paths:
        file_name = os.path.basename(file_path)
        destination_path = os.path.join(destination_directory, file_name)
        shutil.copy2(file_path, destination_path)
        print(f"File '{file_path}' copied to '{destination_directory}'")


def get_runtime_directory(build_type):
    return os.path.join(os.path.dirname(os.path.abspath(__file__)), 'build',  build_type)


def opencv_find_cmake_dir(opencv_install_dir):
    candidates = []
    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, dirs, files in os.walk(opencv_install_dir):
            for file in files:
                if file == 'OpenCVConfig.cmake':
                    candidates.append(root)

    if len(candidates) > 1:
        print(f'Found {len(candidates)} OpenCV candidates: {candidates}')
    return '' if len(candidates) == 0 else max(candidates, key=len)  # return the deepest if multiple found


def opencv_find_bin_dir(opencv_install_dir):
    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, dirs, files in os.walk(opencv_install_dir):
            for file in files:
                if 'opencv_core' in file and (file.endswith('.so') or file.endswith('.dll')):
                    return root

    print('Cannot find OpenCV binary directory - opencv_install_dir:')
    if platform.system() == 'Linux':
        run(f'ls -R {opencv_install_dir}')
    if platform.system() == 'Windows':
        run(f'dir /s {opencv_install_dir}')
    return ''


def opencv_find_binaries(opencv_install_bin_dir):
    bin_files = []
    for root, dirs, files in os.walk(opencv_install_bin_dir):
        for file in files:
            if '.so' in file or '.dll' in file:
                bin_files.append(os.path.join(root, file))

    return bin_files


def opencv_install_linux(opencv_configure_args, jobs, opencv_install_name):
    cwd = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'libs/opencv')
    if not os.path.exists(os.path.join(cwd, 'build')):
        run('mkdir build', cwd)
    run(f'cmake -B ./build {opencv_configure_args}', cwd)
    run(f'cmake --build ./build --config Release -j {jobs}', cwd)
    run(f'sudo cmake --install ./build --prefix ../{opencv_install_name}', cwd)


def opencv_install_windows(opencv_configure_args, jobs, opencv_install_name):
    cwd = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'libs/opencv')
    if not os.path.exists(os.path.join(cwd, 'build')):
        run('mkdir build', cwd)
    run(f'cmake -B ./build {opencv_configure_args}', cwd)
    run(f'cmake --build ./build --config Release -j {jobs}', cwd)
    run(f'cmake --install ./build --prefix ../{opencv_install_name}', cwd)


def opencv_install(opencv_configure_args, jobs, opencv_install_name):
    print(f'Installing OpenCV: {platform.system()}/-j{jobs}/opencv_configure_args={opencv_configure_args}')
    if platform.system() == 'Linux':
        opencv_install_linux(opencv_configure_args, jobs, opencv_install_name)
    if platform.system() == 'Windows':
        opencv_install_windows(opencv_configure_args, jobs, opencv_install_name)

    return opencv_find_cmake_dir(opencv_install_name)


def setup_opencv(opencv_configure_args, jobs, opencv_install_name, opencv_install_dir, build_type):
    # install and get opencv cmake directory
    opencv_install_cmake_dir = opencv_find_cmake_dir(opencv_install_dir)
    if not opencv_install_cmake_dir:
        opencv_install_cmake_dir = opencv_install(opencv_configure_args, jobs, opencv_install_name)
    if not opencv_install_cmake_dir:
        raise RuntimeError("Unable to find installed OpenCV CMake directory")
    print('OpenCV cmake directory: ', opencv_install_cmake_dir)

    # copy opencv binary files to runtime directory
    opencv_install_bin_dir = opencv_find_bin_dir(opencv_install_dir)
    if not opencv_install_bin_dir:
        raise RuntimeError('Cannot find OpenCV binary directory')
    print('OpenCV binary directory: ', opencv_install_bin_dir)
    opencv_binaries = opencv_find_binaries(opencv_install_bin_dir)
    if not opencv_binaries:
        print('Warning: Could not find any OpenCV binaries')
    copy_files_to_directory(opencv_binaries, get_runtime_directory(build_type))
    return opencv_install_cmake_dir


def gcc_install():
    run('sudo add-apt-repository ppa:ubuntu-toolchain-r/test && sudo apt-get update && sudo apt-get install -y gcc-13 g++-13')
    run('sudo update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-13 100 && sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-13 100')


def clang_install():
    run('wget https://apt.llvm.org/llvm.sh')
    run('sudo chmod u+x llvm.sh')
    run('sudo ./llvm.sh 17')
    run('clang-17 --version')
    run('sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang-17 100 && sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-17 100')


def compiler_install(compiler):
    if compiler == 'gcc':
        return gcc_install()
    elif compiler == 'clang':
        return clang_install()
    else:
        raise RuntimeError(f'Unsupported compiler {compiler}')


def cmake_install():
    run('sudo apt-get install -y cmake')


def ninja_install():
    run('sudo apt install ninja-build')


def generator_install(generator):
    if generator == None:
        return
    elif generator == 'Ninja':
        return ninja_install()
    else:
        raise RuntimeError(f'Unsupported generator {generator}')


def opengl_install():
    run('sudo apt install libglu1-mesa-dev mesa-common-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev')


def setup_linux(compiler, generator, opengl):
    cmake_install()
    generator_install(generator)
    if opengl:
        opengl_install()
    compiler_install(compiler)


def find_sanitizer_binaries_windows():
    search_paths = [
        'C:/Program Files/Microsoft Visual Studio',
        'C:/Program Files/Windows Kits',
        'C:/Program Files (x86)/Microsoft Visual Studio',
        'C:/Program Files (x86)/Windows Kits',
    ]

    binaries = []
    for path in search_paths:
        for root, dirs, files in os.walk(path):
            for file in files:
                if 'asan' in file and file.endswith('.dll'):
                    binaries.append(os.path.join(root, file))

    return binaries


def setup_sanitizer_windows(build_type):
    sanitizer_binaries = find_sanitizer_binaries_windows()
    if not sanitizer_binaries:
        print('Warning: Could not find sanitizer binaries')
    print('Sanitizer binaries: ', sanitizer_binaries)
    copy_files_to_directory(sanitizer_binaries, get_runtime_directory(build_type))


def setup_windows(build_type, sanitizer):
    if sanitizer:
        setup_sanitizer_windows(build_type)


def setup_os(compiler, generator, opengl, build_type, sanitizer):
    if platform.system() == 'Linux':
        setup_linux(compiler, generator, opengl)
    if platform.system() == 'Windows':
        setup_windows(build_type, sanitizer)


def build(build_dir, generator, build_type, targets, jobs, ci, sanitizer, opencv_install_cmake_dir):
    print(f"Building: {f'{generator}/' if generator else ''}{build_type}/-j{jobs}/ci={ci}/opencv_install_cmake_dir={opencv_install_cmake_dir}/targets={targets}")
    if not os.path.exists('build'):
        run('mkdir build')
    run(f"cmake -B {build_dir} {f'-G {generator}' if generator else ''} -DCMAKE_BUILD_TYPE={build_type} {'-DCI=ON' if ci else ''} {'-DENABLE_SANITIZER=ON' if sanitizer else ''} -DOpenCV_DIR={opencv_install_cmake_dir}")
    run(f'cmake --build {build_dir} --config {build_type} --target {targets} -j {jobs}')
    print(f'Targets {targets} built successfully')


def check_args(args):
    platforms = ['Linux', 'Windows']
    if not platform.system() in platforms:
        raise RuntimeError(f'Platform {platform.system()} not supported - supported platforms: {platforms}')

    compilers_windows = ['msvc']
    compilers_linux = ['gcc', 'clang']
    compilers = compilers_windows if platform.system() == 'Windows' else compilers_linux
    if args.compiler and args.compiler not in compilers:
        raise RuntimeError(f'Compiler {args.compiler} on {platform.system()} not supported - supported compilers: {compilers}')

    generators_windows = []
    generators_linux = ['Ninja']
    generators = generators_windows if platform.system() == 'Windows' else generators_linux
    if args.generator and args.generator not in generators:
        raise RuntimeError(f'Generator {args.generator} on {platform.system()} not supported - supported generators: {generators}')

    build_types = ['Debug', 'Release']
    if args.build_type not in build_types:
        raise RuntimeError(f'Build type {args.build_type} not supported - supported build types: {build_types}')


if __name__ == '__main__':
    current_dir = os.path.dirname(os.path.abspath(__file__))
    parser = argparse.ArgumentParser(description='Build script')
    parser.add_argument('--compiler', help='compiler', type=str, required=False, default='gcc' if platform.system() == 'Linux' else 'msvc')
    parser.add_argument('--generator', help='generator', type=str, required=False, default='Ninja' if platform.system() == 'Linux' else None)
    parser.add_argument('--build_type', help='build_type', type=str, required=False, default='Release')
    parser.add_argument('--targets', help='targets', type=str, required=False, default='shenanigans shenanigans_test clarity clarity_test')
    parser.add_argument('--build_dir', help='build_dir', type=str, required=False, default='./build')
    parser.add_argument('--jobs', help='jobs', type=int, required=False, default=multiprocessing.cpu_count())
    parser.add_argument('--ci', help='ci', required=False, action='store_true', default='CI' in os.environ)
    parser.add_argument('--sanitizer', help='sanitizer', required=False, action='store_true', default=False)
    args = parser.parse_args()
    check_args(args)

    opengl = 'shenanigans ' in args.targets
    opencv_configure_args = '-DCMAKE_BUILD_TYPE=Release -DOPENCV_EXTRA_MODULES_PATH="../opencv_contrib/modules" -DOPENCV_ENABLE_NONFREE=ON -DBUILD_TESTS=OFF -DBUILD_opencv_python=OFF -DBUILD_opencv_java=OFF -DBUILD_opencv_apps=OFF'
    opencv_install_name = 'opencv-install'
    opencv_install_dir = os.path.join(current_dir, 'libs', opencv_install_name)
    print('platform: ', platform.system())
    print('compiler: ', args.compiler)
    print('generator: ', args.generator)
    print('build_type: ', args.build_type)
    print('targets: ', args.targets)
    print('build_dir', args.build_dir)
    print('jobs: ', args.jobs)
    print('ci: ', args.ci)
    print('sanitizer: ', args.sanitizer)
    print('opengl: ', opengl)
    print('opencv_configure_args: ', opencv_configure_args)
    print('opencv_install_dir: ', opencv_install_dir)

    setup_os(args.compiler, args.generator, opengl, args.build_type, args.sanitizer)
    opencv_install_cmake_dir = setup_opencv(opencv_configure_args, args.jobs, opencv_install_name, opencv_install_dir, args.build_type)

    build(args.build_dir, args.generator, args.build_type, args.targets, args.jobs, args.ci, args.sanitizer, opencv_install_cmake_dir)
