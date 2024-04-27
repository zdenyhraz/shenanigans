import subprocess
import os
import platform
import multiprocessing
import argparse


def run(command, cwd=None):
    print(f"Running command{f' in subdirectory {cwd}' if cwd else ''}: ", command)
    try:
        subprocess.run(command, shell=True, check=True, cwd=cwd if cwd else None)
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Error running command {command}: {e}")


def opencv_find_cmake_dir(opencv_install_dir):
    candidates = []
    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, dirs, files in os.walk(opencv_install_dir):
            for file in files:
                if file == 'OpenCVConfig.cmake':
                    candidates.append(root)

    print(f'Found {len(candidates)} OpenCV candidates: {candidates}')
    return '' if len(candidates) == 0 else max(candidates, key=len)  # return the deepest if multiple found


def opencv_find_bin_dir(opencv_install_dir):
    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, dirs, files in os.walk(opencv_install_dir):
            for file in files:
                if file.startswith('opencv_core') and (file.endswith('.dll') or file.endswith('.a')):
                    print(f'OpenCV binary directory: {root}')
                    return root

    raise RuntimeError('Cannot find opencv binary directory')


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


def opencv_install(os_name,  opencv_configure_args, jobs, opencv_install_name):
    print(f'Installing OpenCV: {os_name}/-j{jobs}/opencv_configure_args={opencv_configure_args}')

    if os_name == 'linux':
        opencv_install_linux(opencv_configure_args, jobs, opencv_install_name)
    elif os_name == 'windows':
        opencv_install_windows(opencv_configure_args, jobs, opencv_install_name)
    else:
        raise RuntimeError(f'Unsupported os: {os_name}')

    return opencv_find_cmake_dir(opencv_install_name)


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


def check_args(args, os_name):
    compilers_windows = ['msvc']
    compilers_linux = ['gcc', 'clang']
    compilers = compilers_windows if os_name == 'windows' else compilers_linux
    if args.compiler and args.compiler not in compilers:
        raise RuntimeError(f'Compiler {args.compiler} on {os_name} not supported - supported compilers: {compilers}')

    generators_windows = []
    generators_linux = ['Ninja']
    generators = generators_windows if os_name == 'windows' else generators_linux
    if args.generator and args.generator not in generators:
        raise RuntimeError(f'Generator {args.generator} on {os_name} not supported - supported generators: {generators}')

    build_types = ['Debug', 'Release']
    if args.build_type not in build_types:
        raise RuntimeError(f'Build type {args.build_type} not supported - supported build types: {build_types}')


def setup_opencv(os_name, opencv_configure_args, jobs, opencv_install_name, opencv_install_dir):
    opencv_install_cmake_dir = opencv_find_cmake_dir(opencv_install_dir)
    if not opencv_install_cmake_dir:
        opencv_install_cmake_dir = opencv_install(os_name, opencv_configure_args, jobs, opencv_install_name)
    if not opencv_install_cmake_dir:
        raise RuntimeError("Unable to find installed OpenCV CMake directory")
    print('opencv_install_cmake_dir: ', opencv_install_cmake_dir)

    opencv_install_bin_dir = opencv_find_bin_dir(opencv_install_dir)
    print('opencv_install_bin_dir: ', opencv_install_bin_dir)
    env_current_path = os.environ.get('PATH', '')
    os.environ['PATH'] = f"{opencv_install_bin_dir}:{env_current_path}"

    return opencv_install_cmake_dir


def setup_linux(compiler, generator, opengl):
    cmake_install()
    generator_install(generator)
    if opengl:
        opengl_install()
    compiler_install(compiler)


def setup_windows():
    pass


def setup_os(os_name, compiler, generator, opengl):
    if os_name == 'linux':
        setup_linux(compiler, generator, opengl)
    if os_name == 'windows':
        setup_windows()


def build(build_dir, generator, build_type, targets, jobs, ci, opencv_install_cmake_dir):
    print(f"Building: {f'{generator}/' if generator else ''}{build_type}/-j{jobs}/ci={ci}/opencv_install_cmake_dir={opencv_install_cmake_dir}/targets={targets}")
    if not os.path.exists('build'):
        run('mkdir build')
    run(f"cmake -B {build_dir} {f'-G {generator}' if generator else ''} -DCMAKE_BUILD_TYPE={build_type} {'-DCI=ON' if ci else ''} -DOpenCV_DIR={opencv_install_cmake_dir}")
    run(f'cmake --build {build_dir} --config {build_type} --target {targets} -j {jobs}')
    print(f'Targets {targets} built successfully')


if __name__ == '__main__':
    os_name = platform.system().lower()
    current_dir = os.path.dirname(os.path.abspath(__file__))
    parser = argparse.ArgumentParser(description='Build script')
    parser.add_argument('--compiler', help='compiler', type=str, required=False, default='gcc' if os_name == 'linux' else 'msvc')
    parser.add_argument('--generator', help='generator', type=str, required=False, default='Ninja' if os_name == 'linux' else None)
    parser.add_argument('--build_type', help='build_type', type=str, required=False, default='Release')
    parser.add_argument('--targets', help='targets', type=str, required=False, default='shenanigans shenanigans_test clarity clarity_test')
    parser.add_argument('--build_dir', help='build_dir', type=str, required=False, default='./build')
    parser.add_argument('--jobs', help='jobs', type=int, required=False, default=multiprocessing.cpu_count())
    parser.add_argument('--ci', help='ci', required=False, action='store_true', default='CI' in os.environ)
    args = parser.parse_args()
    check_args(args, os_name)

    compiler = args.compiler
    generator = args.generator
    build_type = args.build_type
    targets = args.targets
    build_dir = args.build_dir
    jobs = args.jobs
    ci = args.ci
    opengl = 'shenanigans ' in targets
    opencv_configure_args = '-DCMAKE_BUILD_TYPE=Release -DOPENCV_EXTRA_MODULES_PATH="../opencv_contrib/modules" -DOPENCV_ENABLE_NONFREE=ON -DBUILD_TESTS=OFF -DBUILD_opencv_python=OFF -DBUILD_opencv_java=OFF -DBUILD_opencv_apps=OFF'
    opencv_install_name = 'opencv-install'
    opencv_install_dir = os.path.join(current_dir, 'libs', opencv_install_name)

    print('os_name: ', os_name)
    print('compiler: ', compiler)
    print('generator: ', generator)
    print('build_type: ', build_type)
    print('targets: ', targets)
    print('build_dir', build_dir)
    print('jobs: ', jobs)
    print('ci: ', ci)
    print('opengl: ', opengl)
    print('opencv_configure_args: ', opencv_configure_args)
    print('opencv_install_name: ', opencv_install_name)

    setup_os(os_name, compiler, generator, opengl)
    opencv_install_cmake_dir = setup_opencv(os_name, opencv_configure_args, jobs, opencv_install_name, opencv_install_dir)

    build(build_dir, generator, build_type, targets, jobs, ci, opencv_install_cmake_dir)
