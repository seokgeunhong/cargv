
Import('env')


test = env.Program(
    target='test/test-cargv',
    source=['test/cargv_test.cpp'],
    CPPPATH=['include', 'googletest/googletest/include'],
    LIBS=['cargv', 'gtest_main'],
    LIBPATH=['lib', 'googletest/googletest/lib'],
    CCFLAGS=env['CCFLAGS']+['-Wall','-pedantic','-Wextra',
        '-Wcast-align',
        '-Wcast-qual',
        '-Wdisabled-optimization',
        '-Wformat=2',
        '-Winit-self',
        '-Wlogical-op',
        '-Wmissing-include-dirs',
        '-Wredundant-decls',
        '-Wshadow',
        '-Wstrict-overflow=5',
        # '-Wundef',    # GoogleTest
        '-Wno-unused',
        '-Wno-variadic-macros',
        '-Wno-parentheses',
        '-fdiagnostics-show-option',
    ],
    CXXFLAGS=env['CXXFLAGS']+['-Wall','-pedantic','-Wextra',
        '-Wcast-align',
        '-Wcast-qual',
        #'-Wctor-dtor-privacy',  # GoogleTest
        '-Wdisabled-optimization',
        '-Wformat=2',
        '-Winit-self',
        '-Wlogical-op',
        '-Wmissing-include-dirs',
        '-Wnoexcept',
        '-Wold-style-cast',
        '-Woverloaded-virtual',
        '-Wredundant-decls',
        '-Wshadow',
        '-Wsign-promo',
        '-Wstrict-null-sentinel',
        '-Wstrict-overflow=5',
        #'-Wundef',  # GoogleTest
        '-Wno-unused',
        '-Wno-variadic-macros',
        '-Wno-parentheses',
        '-fdiagnostics-show-option',
    ],
)
env.Alias('test', test)
