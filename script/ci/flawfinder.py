from script.setup import utils
import os

if __name__ == '__main__':
    try:
        file_path = 'flawfinder_output.txt'
        filtered_file_path = 'filtered_output.txt'
        utils.run('apt-get update && apt-get install -y flawfinder')
        utils.run(f'flawfinder --savehitlist={file_path} --minlevel=0 --columns --context apps/ src/')
        utils.run(f'grep -v "" {file_path} > {filtered_file_path}')
        if os.path.getsize(filtered_file_path) > 0:
            raise RuntimeError(f'Flawfinder found issues in the code. See {filtered_file_path} for details.')
    except Exception as e:
        raise
