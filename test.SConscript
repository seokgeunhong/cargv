
Import('env')


test = env.Program(
    target='test/test-cargv',
    source=['test/cargv_test.cpp'],
    CPPPATH=['include', 'googletest/googletest/include'],
    LIBS=['cargv', 'gtest_main'],
    LIBPATH=['lib', 'googletest/googletest/lib'],
)
env.Alias('test', test)
