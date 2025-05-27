from script.setup import utils

if __name__ == '__main__':
    try:
        utils.run('pip install cpplint')
        utils.run("find src/ apps/ -name '*.cpp' -o -name '*.hpp' | xargs cpplint")
    except Exception as e:
        raise
