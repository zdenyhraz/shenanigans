from script.setup import utils

if __name__ == '__main__':
    try:
        utils.run('apt-get update && apt-get install -y cppcheck')
        utils.run('cppcheck --enable=all --suppressions-list=.cppcheck-suppressions --inline-suppr --inconclusive --error-exitcode=1 apps/ src/')
    except Exception as e:
        raise
