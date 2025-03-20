import os
import multiprocessing
import argparse
from tools.build import utils
from tools.build import configuration
from tools.build import buildtools
from tools.build import opencv
from tools.build import onnxruntime


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Build script')
    parser.add_argument('--compiler', help='compiler', type=str, required=False, default='gcc' if utils.linux() else 'msvc')
    parser.add_argument('--generator', help='generator', type=str, required=False, default='Ninja' if utils.linux() else None)
    parser.add_argument('--build_type', help='build_type', type=str, required=False, default='Release')
    parser.add_argument('--targets', help='targets', type=str, required=False, default='shenanigans umbellula shenanigans_test')
    parser.add_argument('--build_dir', help='build_dir', type=str, required=False, default='./build')
    parser.add_argument('--jobs', help='jobs', type=int, required=False, default=multiprocessing.cpu_count())
    parser.add_argument('--ci', help='ci', required=False, action='store_true', default='CI' in os.environ)
    parser.add_argument('--sanitizer', help='sanitizer', required=False, default=None)
    parser.add_argument('--opengl', help='opengl', required=False, action='store_true', default=True)
    parser.add_argument('--configure_only', help='configure_only', required=False, action='store_true', default=False)

    args = parser.parse_args()
    for arg, val in vars(args).items():
        print(f'{arg}: {val}')

    configuration.check(args)
    buildtools.setup(args.compiler, args.generator, args.opengl, args.build_type, args.sanitizer)

    print("Setting up libraries")
    onnxruntime_dir = onnxruntime.setup(args.build_type)
    opencv_dir = opencv.setup(args.jobs, args.build_type)

    configure_args = {
        'CMAKE_BUILD_TYPE': args.build_type,
        'CI': 'ON' if args.ci else 'OFF',
        'OPENCV_DIR': opencv_dir,
        'ONNXRUNTIME_DIR': onnxruntime_dir,
        'CMAKE_EXPORT_COMPILE_COMMANDS': 'ON' if args.configure_only or args.ci else 'OFF'
    }

    if args.sanitizer:
        configure_args[f'ENABLE_SANITIZER_{args.sanitizer.upper()}'] = 'ON'

    print(f"Configuring {args.targets}")
    os.makedirs(args.build_dir, exist_ok=True)

    utils.run(f"cmake -B {args.build_dir} {f'-G {args.generator}' if args.generator else ''} {utils.generate_configure_args(configure_args)} ")
    if args.configure_only:
        print(f'Repository configured successfully')
        exit()

    print(f"Building {args.targets}")
    utils.run(f'cmake --build {args.build_dir} --config {args.build_type} --target {args.targets} -j {args.jobs}')
    print(f'{args.targets} built successfully')
