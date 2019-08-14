
Import('env')


lib = env.StaticLibrary(
    target='lib/cargv',
    source=['src/cargv.c',],
    CPPPATH=['include'],
)
env.Alias('lib', lib)


version = env.Program(
    target='bin/cargv-version',
    source=['src/cargv_version.c'],
    CPPPATH=['include'],
    LIBS=['cargv'],
    LIBPATH=['lib'],
)
env.Alias('version', version)


env.Default([lib])
