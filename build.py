import argparse
import os
import shutil
import sys

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument('-c', '--config')
arg_parser.add_argument('-bp', '--boost_path')
arg_parser.add_argument('-g', '--generator')
arg_parser.add_argument('-j', '--jobs', action='store_true')

if __name__ == '__main__':
    if not os.path.isdir('build'):
        os.system('mkdir build')

    args = arg_parser.parse_args()

    config = args.config
    boost_path = args.boost_path
    generator = args.generator

    if config:
        if config not in ['Debug', 'Release']:
            print('--config must match "Debug" or "Release"')
            sys.exit(1)
    else:
        config = 'Debug'

    command = f'cmake -B build -S . -DCMAKE_BUILD_TYPE={config} -DCMAKE_EXPORT_COMPILE_COMMANDS=1'
    if generator:
        command += f' -G {generator}'

    if boost_path:
        command += f' -DPOISE_BOOST_PATH={boost_path}'

    ret_code = os.system(command)

    if ret_code == 0:
        if args.jobs:
            ret_code = os.system(f'cmake --build build --config {config} -- -j')
        else:
            ret_code = os.system(f'cmake --build build --config {config}')

