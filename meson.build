project('RiWindow', 'cpp',
    version : '1.0',
    default_options : ['cpp_std=c++17', 'warning_level=3'])

cmake = import('cmake')

# dependencies

glfw = dependency('glfw3', required : false)
if not glfw.found()
    glfw = cmake.subproject('glfw').dependency('glfw')
endif

rxcpp = dependency('rxcpp', required : false)
if not rxcpp.found()
    rxcpp = cmake.subproject('rxcpp').dependency('rxcpp')
endif

threads = dependency('threads')

x11 = dependency('', required : false)
if host_machine.system() == 'linux'
  x11 = dependency('x11', required : true)
endif

RiUtil = dependency('RiUtil', fallback : ['RiUtil', 'RiUtil_dep'])

# project

includes = include_directories('src')
deps = [RiUtil, rxcpp, glfw, threads, x11]

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
