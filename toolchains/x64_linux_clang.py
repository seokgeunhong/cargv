
from toolchains import Config


# Need apt packages:
#   clang-8
#   gcc
#   g++
#
Config(
    id='x64-linux-clang',
    target_os='x64-linux',
    compiler='clang',
    CC='clang-8',
    CXX='clang++-8',
    LINK='clang++-8',
    CCFLAGS=[
        '--target=x86_64-pc-linux-',
        '-fno-exceptions',
        '-fstandalone-debug',
        '--debug',
    ],
    CXXFLAGS=[
        '-std=c++17',
    ],
    LINKFLAGS=[
        '--target=x86_64-pc-linux-',
        '--debug',
    ],
)
