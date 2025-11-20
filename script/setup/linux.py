from . import utils


def cmake_install():
    utils.run('apt-get install -y cmake')


def ninja_install():
    utils.run('apt install ninja-build')


def generator_install(generator):
    if not generator:
        return
    elif generator == 'Ninja':
        return ninja_install()
    else:
        raise RuntimeError(f'Unsupported generator {generator}')


def opengl_install():
    utils.run('apt install mesa-common-dev libglu1-mesa-dev')
    utils.run('apt install libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev libwayland-dev libxkbcommon-dev')


def gcc_install(version=14):
    utils.run('apt update')
    utils.run('apt install -y software-properties-common')
    utils.run('add-apt-repository ppa:ubuntu-toolchain-r/test')
    utils.run('apt update')
    utils.run(f'apt install -y gcc-{version} g++-{version}')
    utils.run('apt install -y libomp-dev')
    utils.run(f'gcc-{version} --version')
    utils.run(f'update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-{version} 100')
    utils.run(f'update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-{version} 100')
    utils.run(f'update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-{version} 100')


def clang_install(version=19):
    utils.run('apt update')
    utils.run('apt install -y wget gnupg lsb-release software-properties-common')
    utils.run(f"wget -O - https://apt.llvm.org/llvm.sh | bash -s -- {version}")
    utils.run('apt update')
    utils.run(f'apt install -y clang-{version} clang-tidy-{version} clang-tools-{version} lld-{version}')
    utils.run(f'apt install -y libomp-{version}-dev libc++-{version}-dev libc++abi-{version}-dev')
    utils.run(f'update-alternatives --install /usr/bin/clang       clang       /usr/bin/clang-{version}       100')
    utils.run(f'update-alternatives --install /usr/bin/clang++     clang++     /usr/bin/clang++-{version}     100')
    utils.run(f'update-alternatives --install /usr/bin/cc          cc          /usr/bin/clang-{version}       100')
    utils.run(f'update-alternatives --install /usr/bin/c++         c++         /usr/bin/clang++-{version}     100')
    utils.run(f'update-alternatives --install /usr/bin/clang-tidy  clang-tidy  /usr/bin/clang-tidy-{version}  100')
    utils.run(f'clang-{version} --version')


def compiler_install(compiler):
    if compiler == 'gcc':
        return gcc_install()
    elif compiler == 'clang':
        return clang_install()
    else:
        raise RuntimeError(f'Unsupported compiler {compiler}')


def setup_buildtools(compiler, generator, opengl):
    utils.run('apt-get update')
    cmake_install()
    generator_install(generator)
    compiler_install(compiler)
    if opengl:
        opengl_install()
