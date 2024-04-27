import subprocess
import os
import platform
import multiprocessing


def run(command, cwd=None):
    print(f"Running command{f' in subdirectory {cwd}' if cwd else ''}: ", command)
    try:
        subprocess.run(command, shell=True, check=True, cwd=cwd if cwd else None)
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Error running command {command}: {e}")


def opencv_find_lib_dir(opencv_install_name):
    current_dir = os.path.dirname(os.path.abspath(__file__))
    opencv_install_dir = os.path.join(current_dir, 'libs', opencv_install_name)
    candidates = []

    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, dirs, files in os.walk(opencv_install_dir):
            for file in files:
                if file == 'OpenCVConfig.cmake':
                    candidates.append(root)

    print(f'Found {len(candidates)} candidates: {candidates}')
    return '' if len(candidates) == 0 else max(candidates, key=len)  # return the deepest if multiple found


def opencv_install_linux(generator, opencv_configure_args, jobs, opencv_install_prefix):
    cwd = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'libs/opencv')
    run('mkdir build', cwd)
    run(f'cmake -B ./build -G {generator} {opencv_configure_args}', cwd)
    run(f'cmake --build ./build --config Release -j {jobs}', cwd)
    run(f'sudo cmake --install ./build --prefix {opencv_install_prefix}', cwd)


def opencv_install_windows(generator, opencv_configure_args, jobs, opencv_install_prefix):
    cwd = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'libs/opencv')
    run('mkdir build', cwd)
    run(f'cmake -B ./build {opencv_configure_args}', cwd)
    run(f'cmake --build ./build --config Release -j {jobs}', cwd)
    run(f'cmake --install ./build --prefix {opencv_install_prefix}', cwd)


def opencv_install(os_name, generator, opencv_configure_args, jobs, opencv_install_prefix, opencv_install_name):
    print(f'Installing OpenCV: {os_name}/{generator}/-j{jobs}/prefix={opencv_install_prefix}/opencv_configure_args={opencv_configure_args}')

    if os_name == 'Linux':
        opencv_install_linux(generator, opencv_configure_args, jobs, opencv_install_prefix)
    elif os_name == 'Windows':
        opencv_install_windows(generator, opencv_configure_args, jobs, opencv_install_prefix)
    else:
        raise RuntimeError(f'Unsupported os: {os_name}')

    return opencv_find_lib_dir(opencv_install_name)


def gcc_install():
    run('sudo add-apt-repository ppa:ubuntu-toolchain-r/test && sudo apt-get update && sudo apt-get install -y gcc-13 g++-13')
    run('sudo update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-13 100 && sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-13 100')


def clang_install():
    run('wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -')
    run('sudo add-apt-repository "deb https://apt.llvm.org/$(lsb_release -sc)/ llvm-toolchain-$(lsb_release -sc)-17 main"')
    run('sudo apt update')
    run('sudo apt install clang-17')
    run('sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100 && sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100')


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


def build(build_dir, generator, build_type, jobs, ci, opencv_install_cmake_dir):
    print(f'Building: {generator}/{build_type}/-j{jobs}/ci={ci}/opencv_install_cmake_dir={opencv_install_cmake_dir}')
    run('mkdir build')
    run(f"cmake -B {build_dir} {f'-G {generator}' if generator else ''} -DCMAKE_BUILD_TYPE={build_type} -DCI={ci} -DOpenCV_DIR={opencv_install_cmake_dir}")
    run(f'cmake --build {build_dir} --config {build_type} -j {jobs}')


def test():
    pass


if __name__ == '__main__':
    os_name = platform.system()
    generator = 'Ninja' if os_name == 'Linux' else None
    compiler = 'gcc' if os_name == 'Linux' else 'msvc'
    build_type = 'Release'
    build_dir = './build'
    jobs = multiprocessing.cpu_count() + 1
    opencv_configure_args = '-DCMAKE_BUILD_TYPE=Release -DOPENCV_EXTRA_MODULES_PATH="../opencv_contrib/modules" -DOPENCV_ENABLE_NONFREE=ON -DBUILD_TESTS=OFF -DBUILD_opencv_python=OFF -DBUILD_opencv_java=OFF -DBUILD_opencv_apps=OFF'
    opencv_install_name = 'opencv-install'
    opencv_install_prefix = f'../{opencv_install_name}'
    ci = 'ON'

    print('os_name: ', os_name)
    print('generator: ', generator)
    print('compiler: ', compiler)
    print('build_type: ', build_type)
    print('build_dir', build_dir)
    print('jobs: ', jobs)
    print('opencv_configure_args: ', opencv_configure_args)
    print('opencv_install_name: ', opencv_install_name)
    print('opencv_install_prefix: ', opencv_install_prefix)
    print('ci: ', ci)

    if os_name == 'Linux':
        cmake_install()
        generator_install(generator)
        opengl_install()
        compiler_install(compiler)

    opencv_install_cmake_dir = opencv_find_lib_dir(opencv_install_name)
    if not opencv_install_cmake_dir:
        opencv_install_cmake_dir = opencv_install(os_name, generator, opencv_configure_args, jobs, opencv_install_prefix, opencv_install_name)
    if not opencv_install_cmake_dir:
        raise RuntimeError("Unable to find installed OpenCV CMake directory")
    print('opencv_install_cmake_dir: ', opencv_install_cmake_dir)

    build(build_dir, generator, build_type, jobs, ci, opencv_install_cmake_dir)
    test()
