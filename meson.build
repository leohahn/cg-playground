project('tutorial', 'cpp',
        default_options: ['cpp_std=c++14'])

# Include directories
project_inc_dir    = include_directories('src')
lt_inc_dir         = include_directories('lt/src')
thirdparty_inc_dir = include_directories('thirdparty')

cpp = meson.get_compiler('cpp')
# Dependencies
thread_dep = dependency('threads')
m_dep      = cpp.find_library('m')
gl_dep     = dependency('GL')
dl_dep     = cpp.find_library('dl')
glfw3_dep  = dependency('glfw3')
assimp_dep = dependency('assimp')

cpp_args = ['-Wall', '-DDEV_ENV', '-DLT_DEBUG', '-g']
executable('rbs',
           'src/main.cpp', 'thirdparty/glad/glad.cpp', 'src/watcher.cpp', 'src/shader.cpp',
           'lt/src/lt_fs.cpp', 'lt/src/lt_math.cpp', 'src/camera.cpp', 'src/gl_resources.cpp',
           'thirdparty/stb_image.cpp', 'src/draw.cpp', 'src/mesh.cpp', 'src/debug_gui.cpp',
		   'src/imgui_impl_glfw.cpp', 'thirdparty/imgui/imgui.cpp', 'thirdparty/imgui/imgui_draw.cpp',
		   'thirdparty/imgui/imgui_demo.cpp', 'src/resources.cpp', 'src/entities.cpp', 'src/model.cpp',
           dependencies: [
             thread_dep,
             m_dep,
             dl_dep,
             gl_dep,
             glfw3_dep,
             assimp_dep
           ],
           include_directories: [
             project_inc_dir, lt_inc_dir, thirdparty_inc_dir
           ],
           cpp_args: cpp_args)
