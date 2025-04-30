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


def gcc_install(version=14):
    utils.run('brew search gcc')
    utils.run(f'brew install gcc@{version}')
    utils.run(f'brew link --overwrite --force gcc@{version}')
    utils.run('gcc --version')
    utils.run('g++ --version')
    os.environ['CC'] = 'gcc'
    os.environ['CXX'] = 'g++'


def clang_install(version=18):
    utils.run('brew search llvm')
    utils.run(f'brew install llvm@{version}')
    utils.run(f'brew link --overwrite --force llvm@{version}')
    utils.run('clang --version')
    utils.run('clang++ --version')
    os.environ['CC'] = '/opt/homebrew/opt/llvm/bin/clang'
    os.environ['CXX'] = '/opt/homebrew/opt/llvm/bin/clang++'


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
    compiler_install(compiler)
    if opengl:
        opengl_install()
