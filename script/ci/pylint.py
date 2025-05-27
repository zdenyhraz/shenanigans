from script.setup import utils

if __name__ == '__main__':
    try:
        utils.run('pip install pylint')
        utils.run('pylint --rcfile=.pylintrc script/')
    except Exception as e:
        raise
