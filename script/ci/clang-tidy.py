from script.setup import utils

if __name__ == '__main__':
    try:
        utils.run('apt-get install clang-tidy')
        utils.run(r'find ../src -type f \( -name "*.cpp" -o -name "*.hpp" \) | xargs clang-tidy -p=.', 'build')
    except Exception as e:
        raise
