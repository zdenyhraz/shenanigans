from . import utils


def setup(args):
    requirements_file = "requirements.txt"
    utils.run('python --version')
    utils.run('pip install --upgrade pip')
    utils.run(f'pip install -r {requirements_file}')
