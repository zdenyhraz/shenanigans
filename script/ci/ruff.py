from script.setup import utils

if __name__ == '__main__':
    try:
        utils.run('pip install ruff')
        utils.run('ruff check script')
    except Exception as e:
        raise
