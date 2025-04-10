import os
from . import utils


def cmake_install():
    utils.run('brew install cmake')


def ninja_install():
    utils.run('brew install ninja')


def generator_install(generator):
    if not generator:
        return
    if generator == 'Ninja':
        return ninja_install()
    raise RuntimeError(f'Unsupported generator {generator}')


def opengl_install():
    utils.run('brew install mesa')


def gcc_install(version='14'):
    utils.run('brew search gcc')
    utils.run(f'brew install gcc@{version}')
    utils.run('brew unlink gcc')
    utils.run(f'brew link --overwrite --force gcc@{version}')
    utils.run(f'gcc-{version} --version')
    os.environ['CC'] = f'gcc-{version}'
    os.environ['CXX'] = f'g++-{version}'


def clang_install(version='18'):
    utils.run('brew search llvm')
    utils.run(f'brew install llvm@{version}')
    utils.run('brew unlink llvm')
    utils.run(f'brew link --overwrite --force llvm@{version}')
    utils.run(f'clang-{version} --version')
    os.environ['CC'] = f'clang-{version}'
    os.environ['CXX'] = f'clang-{version}'


def compiler_install(compiler):
    if compiler == 'gcc':
        return gcc_install()
    if compiler == 'clang':
        return clang_install()
    raise RuntimeError(f'Unsupported compiler {compiler}')


def setup_buildtools(compiler, generator, opengl):
    utils.run('brew update')
    cmake_install()
    generator_install(generator)
    if opengl:
        opengl_install()
    compiler_install(compiler)
