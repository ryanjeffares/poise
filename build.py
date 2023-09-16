import os
import sys

if __name__ == '__main__':
    if not os.path.isdir('build'):
        os.system('mkdir build')

    if len(sys.argv) > 2:
        config = sys.argv[2]
        if config not in ['Debug', 'Release']:
            print('Config must match "Debug" or "Release"')
            sys.exit(1)
        os.system(f'cmake -B build -S . -DCMAKE_BUILD_TYPE={config} -DCMAKE_EXPORT_COMPILE_COMMANDS=1')
    else:
        os.system('cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1')

    os.system('cmake --build build')
