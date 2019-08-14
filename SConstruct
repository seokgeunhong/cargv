
# List of toolchains
# ==================

import toolchain.x64_linux_gcc
import toolchain.x64_windows_mingw_gcc
import toolchain.x64_windows_msys_gcc
import toolchain.x64_linux_clang
import toolchain.x64_windows_mingw_clang


# Find a toolchain configuration
# ==============================

from toolchain import toolchains

try:
    toolchain = toolchains.Toolchain.from_args(**ARGUMENTS)
    env = Environment(**toolchain.envargs())
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
    variant_dir=os.path.join('out', toolchain.id),
)

SConscript(
    'googletest.SConscript',
    exports='env',
    variant_dir=os.path.join('out', toolchain.id),
)

SConscript(
    'test.SConscript',
    exports='env',
    variant_dir=os.path.join('out', toolchain.id),
)
