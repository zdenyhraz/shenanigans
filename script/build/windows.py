import os
from . import utils
from script.log import log


def find_sanitizer_binaries_windows():
    search_paths = [
        'C:/Program Files/Microsoft Visual Studio',
    ]

    binaries = []
    for path in search_paths:
        for root, _, files in os.walk(path):
            for file in files:
                if 'asan' in file and file.endswith('.dll'):
                    binaries.append(os.path.join(root, file))

    return binaries


def setup_sanitizer_windows(build_type):
    sanitizer_binaries = find_sanitizer_binaries_windows()
    if not sanitizer_binaries:
        log.debug('Warning: Could not find sanitizer binaries')
    log.debug('Sanitizer binaries: ', sanitizer_binaries)
    utils.copy_files_to_directory(sanitizer_binaries, utils.get_runtime_directory(build_type))


def setup_buildtools(build_type, sanitizer):
    if sanitizer:
        setup_sanitizer_windows(build_type)
