import subprocess
import os
import platform
import multiprocessing


def run_command(command, cwd=None):
    print("Running command: ", command)
    try:
        subprocess.run(command, shell=True, check=True, cwd=cwd if cwd else None)
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Error running command {command}: {e}")
    return True


def opencv_find_lib_dir(os_name, opencv_install_name):
    filename = 'opencv_core'
    extension = '.lib' if os_name == 'Windows' else '.a'
    current_dir = os.path.dirname(os.path.abspath(__file__))
    opencv_install_dir = os.path.join(current_dir, 'libs', opencv_install_name)

    if os.path.exists(opencv_install_dir) and os.path.isdir(opencv_install_dir):
        for root, dirs, files in os.walk(opencv_install_dir):
            for file in files:
                if file.startswith(filename) and file.endswith(extension):
                    return root

    return ''


def opencv_install_linux(generator, opencv_configure_args, jobs, opencv_install_prefix):
    cwd = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'libs/opencv')
    run_command('mkdir build', cwd)
    run_command(f'cmake -B ./build -G {generator} {opencv_configure_args}', cwd)
    run_command(f'cmake --build ./build --config Release -j {jobs}', cwd)
    run_command(f'sudo cmake --install ./build --prefix {opencv_install_prefix}', cwd)
    run_command('sudo cmake --install ./build', cwd)
    return opencv_find_lib_dir()


def opencv_install_windows(generator, opencv_configure_args, jobs, opencv_install_prefix):
    cwd = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'libs/opencv')
    run_command('mkdir build', cwd)
    run_command(f'cmake -B ./build {opencv_configure_args}', cwd)
    run_command(f'cmake --build ./build --config Release -j {jobs}', cwd)
    run_command(f'cmake --install ./build --prefix {opencv_install_prefix}', cwd)
    return opencv_find_lib_dir()


def opencv_install(os_name, generator, opencv_configure_args, jobs, opencv_install_prefix):
    print(f'Installing OpenCV: {os_name}/{generator}/j{jobs}/prefix={opencv_install_prefix}/opencv_configure_args={opencv_configure_args}')
    if os_name == 'Linux':
        return opencv_install_linux(generator, opencv_configure_args, jobs, opencv_install_prefix)
    elif os_name == 'Windows':
        return opencv_install_windows(generator, opencv_configure_args, jobs, opencv_install_prefix)
    else:
        raise RuntimeError(f'Unsupported os: {os_name}')


def gcc_install():
    run_command('sudo add-apt-repository ppa:ubuntu-toolchain-r/test && sudo apt-get update && sudo apt-get install -y gcc-13 g++-13')
    run_command('sudo update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-13 100 && sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-13 100')


def clang_install():
    run_command('wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -')
    run_command('sudo add-apt-repository "deb https://apt.llvm.org/$(lsb_release -sc)/ llvm-toolchain-$(lsb_release -sc)-17 main"')
    run_command('sudo apt update')
    run_command('sudo apt install clang-17')
    run_command('sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100 && sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100')


def compiler_install(compiler):
    if compiler == 'gcc':
        return gcc_install()
    elif compiler == 'clang':
        return clang_install()
    else:
        raise RuntimeError(f'Unsupported compiler {compiler}')


def cmake_install():
    run_command('sudo apt-get install -y cmake')


def ninja_install():
    run_command('sudo apt install ninja-build')


def generator_install(generator):
    if generator == 'Ninja':
        return ninja_install()
    else:
        raise RuntimeError(f'Unsupported generator {generator}')


def opengl_install():
    run_command('sudo apt install libglu1-mesa-dev mesa-common-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev')


def build_windows(generator, build_type, jobs, ci, opencv_install_lib_dir):
    run_command('mkdir build')
    run_command(f'cmake -B ./build -DCMAKE_BUILD_TYPE={build_type} -DCI={ci} -DOPENCV_DIR={opencv_install_lib_dir}')
    run_command(f'cmake --build ./build --config {build_type} -j {jobs}')


def build_linux(generator, build_type, jobs, ci, opencv_install_lib_dir):
    run_command('mkdir build')
    run_command(f'cmake -B ./build -G {generator} -D CMAKE_BUILD_TYPE={build_type} -DCI={ci} -DOPENCV_DIR={opencv_install_lib_dir}')
    run_command(f'cmake --build ./build --config {build_type} -j {jobs}')


def build(os_name, generator, build_type, jobs, ci, opencv_install_lib_dir):
    if os_name == 'Linux':
        return build_linux(generator, build_type, jobs, ci, opencv_install_lib_dir)
    elif os_name == 'Windows':
        return build_windows(generator, build_type, jobs, ci, opencv_install_lib_dir)
    else:
        raise RuntimeError(f'Unsupported os: {os_name}')


def test():
    pass


if __name__ == '__main__':
    try:
        os_name = platform.system()
        generator = 'Ninja'
        compiler = 'gcc'
        build_type = 'Release'
        jobs = multiprocessing.cpu_count() + 1
        opencv_configure_args = '-DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DOPENCV_EXTRA_MODULES_PATH="../opencv_contrib/modules" -DOPENCV_ENABLE_NONFREE=ON -DBUILD_TESTS=OFF -DBUILD_opencv_python=OFF -DBUILD_opencv_java=OFF -DBUILD_opencv_apps=OFF'
        opencv_install_name = 'opencv-install'
        opencv_install_prefix = f'../{opencv_install_name}'
        ci = 'ON'

        print('os_name: ', os_name)
        print('generator: ', generator)
        print('jobs: ', jobs)
        print('opencv_configure_args: ', opencv_configure_args)
        print('opencv_install_name: ', opencv_install_name)
        print('opencv_install_prefix: ', opencv_install_prefix)

        if os_name == 'Linux':
            cmake_install()
            generator_install(generator)
            opengl_install()
            compiler_install(compiler)

        opencv_install_lib_dir = opencv_find_lib_dir(os_name, opencv_install_name)  # cached
        if not opencv_install_lib_dir:
            opencv_install_lib_dir = opencv_install(os_name, generator, opencv_configure_args, jobs, opencv_install_prefix)
        print('opencv_install_lib_dir: ', opencv_install_lib_dir)

        build(os_name, generator, build_type, jobs, ci, opencv_install_lib_dir)
        test()

    except Exception as e:
        print(e)
        raise e
