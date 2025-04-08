import os
import argparse
from script.log import log
from script.setup import utils
from script.setup import configuration
from script.setup import buildtools
from script.setup import opencv
from script.setup import onnxruntime


def configure(args):
    log.info(f"Configuring {args.targets}")
    configure_args = {
        'CMAKE_BUILD_TYPE': args.build_type,
        'OPENCV_DIR': args.opencv_dir,
        'ONNXRUNTIME_DIR': args.onnxruntime_dir,
        **({f'CMAKE_EXPORT_COMPILE_COMMANDS': 'ON'} if args.configure_only else {}),
        **({f'CI': 'ON'} if args.ci else {}),
        **({f'ENABLE_SANITIZER_{args.sanitizer.upper()}': 'ON'} if args.sanitizer else {}),
    }
    os.makedirs(args.build_dir, exist_ok=True)
    builddir_args = f'-B {args.build_dir}'
    generator_args = f'-G {args.generator}' if args.generator else ''
    cmake_args = utils.generate_configure_args(configure_args)
    utils.run(f"cmake {builddir_args} {generator_args} {cmake_args}")
    log.info('Repository configured successfully')
    args.configure_only and exit()


def build(args):
    log.info(f"Building {args.targets}")
    utils.run(f'cmake --build {args.build_dir} --config {args.build_type} --target {args.targets} --parallel')
    log.info(f'{args.targets} built successfully')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Build script')
    parser.add_argument('--compiler', help='compiler', type=str, required=False, default='gcc' if utils.linux() else 'msvc')
    parser.add_argument('--generator', help='generator', type=str, required=False, default='Ninja' if utils.linux() else None)
    parser.add_argument('--build_type', help='build_type', type=str, required=False, default='Release')
    parser.add_argument('--targets', help='targets', type=str, required=False, default='all' if utils.linux() else 'ALL_BUILD')
    parser.add_argument('--build_dir', help='build_dir', type=str, required=False, default='./build')
    parser.add_argument('--ci', help='ci', required=False, action='store_true', default='CI' in os.environ)
    parser.add_argument('--sanitizer', help='sanitizer', required=False, default=None)
    parser.add_argument('--opengl', help='opengl', required=False, action='store_true', default=True)
    parser.add_argument('--configure_only', help='configure_only', required=False, action='store_true', default=False)
    parser.add_argument('--opencv_dir', help='opencv_dir', type=str, required=False, default=None)
    parser.add_argument('--onnxruntime_dir', help='onnxruntime_dir', type=str, required=False, default=None)

    args = parser.parse_args()
    log.info(f"Setting up {args.targets}")
    for arg, val in vars(args).items():
        log.debug(f'{arg}: {val}')

    configuration.check(args)
    buildtools.setup(args.compiler, args.generator, args.opengl, args.build_type, args.sanitizer)

    log.info("Setting up libraries")
    args.onnxruntime_dir = args.onnxruntime_dir if args.onnxruntime_dir else onnxruntime.setup(args.build_type)
    args.opencv_dir = args.opencv_dir if args.opencv_dir else opencv.setup(args.build_type)
    log.info("Libraries set up successfully")

    configure(args)
    build(args)
