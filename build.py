import os
import argparse
from script.log import log
from script.setup import utils
from script.setup import configuration
from script.setup import buildtools
from script.setup import opencv
from script.setup import onnxruntime


def configure(args):
    log.info(f"Configuring target {args.targets}")
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
    log.info(f'Target {args.targets} configured successfully')
    args.configure_only and exit()


def build(args):
    log.info(f"Building target {args.targets}")
    utils.run(f'cmake --build {args.build_dir} --config {args.build_type} --target {args.targets} --parallel')
    log.info(f'Target {args.targets} built successfully')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Build script')
    parser.add_argument('--compiler', help='compiler', type=str, required=False, default='msvc' if utils.windows() else 'clang' if utils.macos() else 'gcc')
    parser.add_argument('--generator', help='generator', type=str, required=False, default=None if utils.windows() else 'Ninja')
    parser.add_argument('--targets', help='targets to build', type=str, required=False, default='ALL_BUILD' if utils.windows() else 'all')
    parser.add_argument('--build_type', help='build type', type=str, required=False, default='Release')
    parser.add_argument('--build_dir', help='build directory', type=str, required=False, default='build')
    parser.add_argument('--ci', help='ci', required=False, action='store_true', default='CI' in os.environ)
    parser.add_argument('--sanitizer', help='sanitizer', required=False, default=None)
    parser.add_argument('--opengl', help='install opengl', required=False, action='store_true', default=True)
    parser.add_argument('--configure_only', help='configure only', required=False, action='store_true', default=False)
    parser.add_argument('--opencv_dir', help='opencv dir', type=str, required=False, default=None)
    parser.add_argument('--onnxruntime_dir', help='onnxruntime dir', type=str, required=False, default=None)
    parser.add_argument('--jobs', help='build jobs', type=int, required=False, default=os.cpu_count())
    args = parser.parse_args()

    log.info(f"Setting up target {args.targets}")
    for arg, val in vars(args).items():
        log.debug(f'{arg}: {val}')

    configuration.check(args)
    buildtools.setup(args.compiler, args.generator, args.opengl, args.build_type, args.sanitizer)

    log.info("Setting up libraries")
    args.onnxruntime_dir = args.onnxruntime_dir or onnxruntime.setup(args.build_type)
    args.opencv_dir = args.opencv_dir or opencv.setup(args.build_type, args.jobs)
    log.info("Libraries set up successfully")

    configure(args)
    build(args)
