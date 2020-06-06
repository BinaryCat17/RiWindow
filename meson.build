project('RiWindow', 'cpp', default_options : ['cpp_std=c++17'])

cmake = import('cmake')
signals_proj = cmake.subproject('signals')

signals_dep = signals_proj.dependency('signals')

RiWindow_deps = [signals_dep]
RiWindow_inc = include_directories('include')
RiWindow_dep = declare_dependency(include_directories : RiWindow_inc, dependencies : RiWindow_deps)
