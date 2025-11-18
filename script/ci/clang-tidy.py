from script.setup import utils


if __name__ == '__main__':
    try:
        utils.run('sudo apt update')
        utils.run('sudo apt install -y clang-19 clang-tidy-19 clang-tools-19')
        utils.run('sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100')
        utils.run('sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100')
        utils.run('sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-19 100')

        utils.run('clang --version')
        utils.run('clang-tidy --version')

        utils.run("run-clang-tidy -p build")

    except Exception as e:
        raise
