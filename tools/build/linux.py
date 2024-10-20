from . import utils


def cmake_install():
    utils.run('sudo apt-get install -y cmake')


def ninja_install():
    utils.run('sudo apt install ninja-build')


def generator_install(generator):
    if generator == None:
        return
    elif generator == 'Ninja':
        return ninja_install()
    else:
        raise RuntimeError(f'Unsupported generator {generator}')


def opengl_install():
    utils.run('sudo apt install libglu1-mesa-dev mesa-common-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev libwayland-dev libxkbcommon-dev')


def gcc_install():
    utils.run('sudo add-apt-repository ppa:ubuntu-toolchain-r/test && sudo apt update && sudo apt install g++-13')
    utils.run('sudo apt install libstdc++-dev')
    utils.run('sudo update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-13 100')
    utils.run('sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-13 100')


def clang_install():
    utils.run('wget https://apt.llvm.org/llvm.sh')
    utils.run('sudo chmod u+x llvm.sh')
    utils.run('sudo ./llvm.sh 17')
    utils.run('clang-17 --version')
    utils.run('sudo apt install libc++-dev libc++abi-dev libomp-17-dev')
    utils.run('sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang-17 100')
    utils.run('sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-17 100')


def compiler_install(compiler):
    if compiler == 'gcc':
        return gcc_install()
    elif compiler == 'clang':
        return clang_install()
    else:
        raise RuntimeError(f'Unsupported compiler {compiler}')


def setup_buildtools(compiler, generator, opengl):
    utils.run('sudo apt-get update')
    cmake_install()
    generator_install(generator)
    if opengl:
        opengl_install()
    compiler_install(compiler)
