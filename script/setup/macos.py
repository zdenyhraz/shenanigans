from . import utils


def cmake_install():
    utils.run('brew install cmake')


def ninja_install():
    utils.run('brew install ninja')


def generator_install(generator):
    if generator is None:
        return
    elif generator == 'Ninja':
        return ninja_install()
    else:
        raise RuntimeError(f'Unsupported generator {generator}')


def opengl_install():
    utils.run('brew install mesa')


def gcc_install():
    utils.run('brew install gcc')
    utils.run('gcc-14 --version')
    utils.run('brew link --overwrite gcc')


def clang_install():
    utils.run('brew install llvm')
    utils.run('clang --version')
    utils.run('brew install libomp')


def compiler_install(compiler):
    if compiler == 'gcc':
        return gcc_install()
    elif compiler == 'clang':
        return clang_install()
    else:
        raise RuntimeError(f'Unsupported compiler {compiler}')


def setup_buildtools(compiler, generator, opengl):
    utils.run('brew update')
    cmake_install()
    generator_install(generator)
    if opengl:
        opengl_install()
    compiler_install(compiler)
