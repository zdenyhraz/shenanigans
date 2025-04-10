from . import linux, windows, macos, utils


def setup(compiler, generator, opengl, build_type, sanitizer):
    if utils.linux():
        linux.setup_buildtools(compiler, generator, opengl)
    if utils.windows():
        windows.setup_buildtools(build_type, sanitizer)
    if utils.macos():
        macos.setup_buildtools(compiler, generator, opengl)
