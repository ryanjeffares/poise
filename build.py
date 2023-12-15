import argparse
import os
import sys

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument("--config")
arg_parser.add_argument("--boost_path")

if __name__ == '__main__':
    if not os.path.isdir('build'):
        os.system('mkdir build')

    args = arg_parser.parse_args()

    config = args.config
    boost_path = args.boost_path

    if config:
        if config not in ['Debug', 'Release']:
            print('--config must match "Debug" or "Release"')
            sys.exit(1)
    else:
        config = 'Debug'

    if boost_path:
        ret_code = os.system(f'cmake -B build -S . -DCMAKE_BUILD_TYPE={config} -DPOISE_BOOST_PATH="{boost_path}" -DCMAKE_EXPORT_COMPILE_COMMANDS=1')
    else:
        ret_code = os.system(f'cmake -B build -S . -DCMAKE_BUILD_TYPE={config} -DCMAKE_EXPORT_COMPILE_COMMANDS=1')

    if ret_code == 0:
        os.system(f'cmake --build build --config {config} -- -j')
