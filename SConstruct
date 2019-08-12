
# List of toolchains
# ==================

import toolchains.x64_linux_gcc
import toolchains.x64_windows_mingw_gcc
import toolchains.x64_windows_msys_gcc
import toolchains.x64_linux_clang
import toolchains.x64_windows_mingw_clang


# Find a toolchain configuration
# ==============================

from toolchains import toolchains
try:
    toolchain = toolchains.Config.find_with_args(**ARGUMENTS)
    for l in toolchain.log():
        print(':'+l)

except toolchains.ArgNotSupportedError as e:
    print('cargv:'+e.err)
    print()
    print(e.help)
    exit(1)


# Construct an environment
# ========================

import os

env = Environment(
    ENV=os.environ,
    CC=toolchain.prefix+toolchain.env('CC'),
    CXX=toolchain.prefix+toolchain.env('CXX'),
    LINK=toolchain.prefix+toolchain.env('LINK'),
    CPPPATH=toolchain.env('CPPPATH'),
    CCFLAGS=toolchain.env('CCFLAGS'),
    LINKFLAGS=toolchain.env('LINKFLAGS'),
)
env.PrependENVPath('PATH', toolchain.env('PATH'))


# Modules
# =======

SConscript(
    'cargv.SConscript',
    exports='env',
    variant_dir=os.path.join('out', toolchain.id),
)

SConscript(
    'gtest.SConscript',
    exports='env',
    variant_dir=os.path.join('out', toolchain.id),
)
