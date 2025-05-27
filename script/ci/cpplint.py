from script.setup import utils

if __name__ == '__main__':
    try:
        utils.run('pip install cpplint')
        utils.run('find apps/ src/ -type f \( -iname \*.hpp -o -iname \*.cpp \) | xargs cpplint')
    except Exception as e:
        raise
