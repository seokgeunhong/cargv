
Import('env')


test = env.Program(
    target='test/test-all',
    source=Glob('test/test_*.cpp'),
    CPPPATH=['include', 'googletest/googletest/include'],
    LIBS=['cargv', 'gtest_main'],
    LIBPATH=['lib', 'googletest/googletest/lib'],
)
env.Alias('test', test)
