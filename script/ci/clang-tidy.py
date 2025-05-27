from script.setup import utils

if __name__ == '__main__':
    try:
        utils.run('apt-get install clang-tidy')
        utils.run("find src/ apps/ -name '*.cpp' -o -name '*.h' | xargs clang-tidy -p=.", 'build')
    except Exception as e:
        raise
