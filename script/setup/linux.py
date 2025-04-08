from . import utils


def cmake_install():
    utils.run('apt-get install -y cmake')


def ninja_install():
    utils.run('apt install ninja-build')


def generator_install(generator):
    if generator == None:
        return
    elif generator == 'Ninja':
        return ninja_install()
    else:
        raise RuntimeError(f'Unsupported generator {generator}')


def opengl_install():
    utils.run('apt install libglu1-mesa-dev mesa-common-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev libwayland-dev libxkbcommon-dev')


def gcc_install():
    utils.run('apt update')
    utils.run('apt install -y software-properties-common')
    utils.run('add-apt-repository ppa:ubuntu-toolchain-r/test')
    utils.run('apt update')
    utils.run('apt install -y gcc-14 g++-14')
    utils.run('gcc-14 --version')
    utils.run('update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-14 100')
    utils.run('update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-14 100')


def clang_install():
    utils.run('apt update')
    utils.run('apt install -y wget gnupg software-properties-common')
    utils.run('wget -O - https://apt.llvm.org/llvm.sh | bash')
    utils.run('apt install -y clang')
    utils.run('clang --version')
    utils.run('apt install -y libc++-dev libc++abi-dev libomp-dev')
    utils.run('update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100')
    utils.run('update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100')


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
    if opengl:
        opengl_install()
    compiler_install(compiler)
