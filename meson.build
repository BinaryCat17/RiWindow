project('RiWindow', 'cpp',
    version : '1.0',
    default_options : ['cpp_std=c++2a', 'warning_level=3'])

cmake = import('cmake')

# dependencies

signals = dependency('signals', required : false)
if not signals.found()
    signals = cmake.subproject('signals').dependency('signals')
endif

glfw = dependency('glfw3', required : false)
if not glfw.found()
    glfw = cmake.subproject('glfw').dependency('glfw')
endif

RiUtil = dependency('RiUtil', fallback : ['RiUtil', 'RiUtil_dep'])

# project

includes = include_directories('src')
deps = [signals, glfw, RiUtil]

lib = static_library('RiWindow',
    'src/RiWindow/window.cpp',
    install : true,
    dependencies : deps,
    include_directories : includes)

RiWindow_dep = declare_dependency(link_with : lib,
    include_directories : includes, dependencies : deps)


# tests

testWindow = executable('testWindow', 'tests/window_test.cpp', dependencies : RiWindow_dep)
test('testWindow', testWindow)
