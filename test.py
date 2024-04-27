import subprocess
import os


def run(command, cwd=None):
    print(f"Running command{f' in subdirectory {cwd}' if cwd else ''}: ", command)
    try:
        subprocess.run(command, shell=True, check=True, cwd=cwd if cwd else None)
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Error running command {command}: {e}")


if __name__ == '__main__':
    cwd = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'build')
    if not os.path.exists(cwd):
        raise RuntimeError(f'Cannot find build directory {cwd}')

    run('ctest --rerun-failed --output-on-failure', cwd)
