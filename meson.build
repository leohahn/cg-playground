project('tutorial', 'cpp',
        default_options: ['cpp_std=c++14'])

# Include directories
project_inc_dir = include_directories('src')
lt_inc_dir       = include_directories('lt/src')

cpp = meson.get_compiler('cpp')
# Dependencies
thread_dep = dependency('threads')
m_dep      = cpp.find_library('m')
gl_dep     = dependency('GL')
dl_dep  = cpp.find_library('dl')
glfw3_dep  = dependency('glfw3')

cpp_args = ['-Wall', '-DDEV_ENV', '-g']
executable('rbs',
           'src/main.cpp', 'src/glad/glad.cpp', 'src/watcher.cpp', 'src/shader.cpp',
           'lt/src/lt_fs.cpp', 'lt/src/lt_math.cpp', 'src/camera.cpp', 'src/gl_resources.cpp',
           'src/stb_image.cpp',
           dependencies: [
             thread_dep,
             m_dep,
             dl_dep,
             gl_dep,
             glfw3_dep
           ],
           include_directories: [
             project_inc_dir, lt_inc_dir
           ],
           cpp_args: cpp_args)
