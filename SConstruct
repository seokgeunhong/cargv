
import os

# List of toolchains
# ==================

import toolchain.x64_linux_gcc_8
import toolchain.x64_windows_mingw_gcc
#import toolchain.x64_windows_msys_gcc
import toolchain.x64_linux_clang
#import toolchain.x64_windows_mingw_clang

import toolchain.gcc


# Find a toolchain configuration
# ==============================

from toolchain import toolchains

try:
    toolchain = toolchains.find(**ARGUMENTS)
    env = Environment(ENV=os.environ, **toolchain.env())
    print('cargv: found a toolchain.\n  ' + '\n  '.join(toolchain.log()))

except Exception as e:
    print('cargv: ERROR: '+e.args[0])
    exit(1)


# Modules
# =======

import os.path

SConscript(
    'cargv.SConscript',
    exports='env',
    variant_dir=os.path.join('out', toolchain.id[0]),
)

SConscript(
    'googletest.SConscript',
    exports='env',
    variant_dir=os.path.join('out', toolchain.id[0]),
)

SConscript(
    'test.SConscript',
    exports='env',
    variant_dir=os.path.join('out', toolchain.id[0]),
)
