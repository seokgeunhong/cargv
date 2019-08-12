
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


test = env.Program(
    target='test/test-cargv',
    source=['test/cargv_test.cpp'],
    CPPPATH=['googletest/googletest/include', 'include'],
    LIBS=['cargv','gtest_main'],
    LIBPATH=['lib','googletest/googletest/lib'],
)
env.Alias('test', test)


env.Default([lib])
