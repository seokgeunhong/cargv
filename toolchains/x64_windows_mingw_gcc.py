
from toolchains import Config

# Need apt packages:
#   gcc-mingw-w64-x86-64
#   g++-mingw-w64-x86-64
#
Config(
    id='x64-windows-mingw-gcc',
    target_os='x64-windows',
    compiler='mingw-gcc',
    prefix='x86_64-w64-mingw32-',
    CC='gcc',
    CXX='g++',
    LINK='g++',
    CCFLAGS=['-g'],
)
