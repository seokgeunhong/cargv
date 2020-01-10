
Import('env')


obj = env.Object(
    source=['src/cargv.c'],
    CPPPATH=['include'],
)
lib = env.StaticLibrary(
    target='lib/cargv',
    source=obj,
)
env.Alias('lib', lib)


env.Default([lib])
