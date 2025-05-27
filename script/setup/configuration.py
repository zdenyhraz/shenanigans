import platform


def check_platform():
    platforms = ['Linux', 'Windows', 'Darwin']
    if platform.system() not in platforms:
        raise RuntimeError(f'Platform {platform.system()} not supported - supported platforms: {platforms}')


def check_compiler(compiler):
    compilers = {'Windows': ['msvc'], 'Linux': ['gcc', 'clang'], 'Darwin': ['clang']}
    if compiler and compiler not in compilers[platform.system()]:
        raise RuntimeError(f'Compiler {compiler} on {platform.system()} not supported - supported compilers: {compilers[platform.system()]}')


def check_generator(generator):
    generators = {'Windows': [], 'Linux': ['Ninja'], 'Darwin': ['Ninja']}
    if generator and generator not in generators[platform.system()]:
        raise RuntimeError(f'Generator {generator} on {platform.system()} not supported - supported generators: {generators[platform.system()]}')


def check_build_type(build_type):
    build_types = ['Debug', 'Release']
    if build_type not in build_types:
        raise RuntimeError(f'Build type {build_type} not supported - supported build types: {build_types}')


def check(args):
    check_platform()
    check_compiler(args.compiler)
    check_generator(args.generator)
    check_build_type(args.build_type)
