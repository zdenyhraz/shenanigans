from script.setup import utils


if __name__ == '__main__':
    try:
        utils.run('wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -')
        utils.run('sudo add-apt-repository "deb http://apt.llvm.org/jammy/ llvm-shivam main"')
        utils.run('sudo apt update')
        utils.run('sudo apt install -y clang-19 clang-tidy-19 clang-tools-19')
        utils.run('sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100')
        utils.run('sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100')
        utils.run('sudo update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-19 100')

        utils.run('clang --version')
        utils.run('clang-tidy --version')
        utils.run('run-clang-tidy --version')

        utils.run("run-clang-tidy -p build -header-filter='.*' -checks='*'")

    except Exception as e:
        raise
