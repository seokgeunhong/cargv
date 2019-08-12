
from toolchains import Config

# Need apt packages:
#   gcc-7
#   g++-7
#
Config(
    id='x64-linux-gcc',
    target_os='x64-linux',
    compiler='gcc',
    prefix='x86_64-linux-gnu-',
    CC='gcc-7',
    CXX='g++-7',
    LINK='g++-7',
    CCFLAGS=['-g'],
)
