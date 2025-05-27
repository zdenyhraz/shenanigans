from script.setup import utils

if __name__ == '__main__':
    try:
        utils.run('pip install flake8')
        utils.run('flake8 script')
    except Exception as e:
        raise
