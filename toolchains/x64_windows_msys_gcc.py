
from toolchains import Config


Config(
    id='x64-windows-msys-gcc',
    target_os='x64-windows-msys',
    compiler='gcc',
    prefix='x86_64-pc-msys-',
    CC='gcc',
    CXX='g++',
    LINK='g++',
    CCFLAGS=['-g'],
)
