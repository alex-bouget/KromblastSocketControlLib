project(
    'SocketControlLib',
    'cpp',
    version : '0.1',
    default_options : ['warning_level=3', 'cpp_std=c++20']
)

cc = meson.get_compiler('cpp')

if get_option('buildtype').startswith('debug')
  add_project_arguments('-DDEBUG', language : 'cpp')
endif

kromblast_api_dep = subproject('_kromblast').get_variable('kromblast_api_dep')

socket_control_lib_dep = declare_dependency(
    include_directories : include_directories('src'),
)

socket_control_lib = shared_library(
    'socket_control_lib',
    'src/main.cpp',
    'src/socket_control.cpp',
    'src/ksocket.cpp',
    dependencies: [
        kromblast_api_dep,
        socket_control_lib_dep
    ]
)