import os
import multiprocessing
import argparse
import tools.build as build


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Build script')
    parser.add_argument('--compiler', help='compiler', type=str, required=False, default='gcc' if build.utils.linux() else 'msvc')
    parser.add_argument('--generator', help='generator', type=str, required=False, default='Ninja' if build.utils.linux() else None)
    parser.add_argument('--build_type', help='build_type', type=str, required=False, default='Release')
    parser.add_argument('--targets', help='targets', type=str, required=False, default='shenanigans shenanigans_test')
    parser.add_argument('--build_dir', help='build_dir', type=str, required=False, default='./build')
    parser.add_argument('--jobs', help='jobs', type=int, required=False, default=multiprocessing.cpu_count())
    parser.add_argument('--ci', help='ci', required=False, action='store_true', default='CI' in os.environ)
    parser.add_argument('--sanitizer', help='sanitizer', required=False, action='store_true', default=False)
    parser.add_argument('--opengl', help='opengl', required=False, action='store_true', default=True)

    args = parser.parse_args()
    for arg, val in vars(args).items():
        print(f'{arg}: {val}')

    build.configuration.check(args)
    build.buildtools.setup(args.compiler, args.generator, args.opengl, args.build_type, args.sanitizer)
    opencv_dir = build.opencv.setup(args.jobs, args.build_type)

    configure_args = {
        'CMAKE_BUILD_TYPE': args.build_type,
        'CI': 'ON' if args.ci else 'OFF',
        'ENABLE_SANITIZER': 'ON' if args.sanitizer else 'OFF',
        'OPENCV_DIR': opencv_dir
    }

    print(f"Building {args.targets}")
    os.makedirs("build", exist_ok=True)
    build.utils.run(f"cmake -B {args.build_dir} {f'-G {args.generator}' if args.generator else ''} {build.utils.generate_configure_args(configure_args)}")
    build.utils.run(f'cmake --build {args.build_dir} --config {args.build_type} --target {args.targets} -j {args.jobs}')
    print(f'{args.targets} built successfully')
