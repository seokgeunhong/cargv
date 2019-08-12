
from toolchains import Config

# Need apt packages:
#   clang-8
#   gcc-mingw-w64-x86-64
#   g++-mingw-w64-x86-64
#
Config(
    id='x64-windows-mingw-clang',
    target_os='x64-windows',
    compiler='mingw-clang',
    CC='clang-8',
    CXX='clang++-8',
    LINK='clang++-8',
    CCFLAGS=[
        '--target=x86_64-w64-mingw32',
        '-fno-exceptions',
        '-fstandalone-debug',
        '--debug',
    ],
    CXXFLAGS=[
        '-std=c++17',
    ],
    LINKFLAGS=[
        '--target=x86_64-w64-mingw32',
        '--debug',
    ],
)
