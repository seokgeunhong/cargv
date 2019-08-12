
Import('env')


lib = env.StaticLibrary(
    target='googletest/googletest/lib/gtest',
    source=['googletest/googletest/src/gtest-all.cc'],
    CPPPATH=[
        'googletest/googletest/include',
        'googletest/googletest'
    ],
    CCFLAGS=['-DGTEST_HAS_PTHREAD=0'],
)
env.Alias('gtest', lib)


main = env.StaticLibrary(
    target='googletest/googletest/lib/gtest_main',
    source=[
        'googletest/googletest/src/gtest-all.cc',
        'googletest/googletest/src/gtest_main.cc',
    ],
    CPPPATH=[
        'googletest/googletest/include',
        'googletest/googletest'
    ],
    CCFLAGS=['-DGTEST_HAS_PTHREAD=0'],
)
env.Alias('gtest_main', main)
