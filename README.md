# The project
This is a project started to learn algorithms of computer graphics. It implements normal mapping, shadow mapping, HDR buffers, bloom, Blinn-Phong shading, etc. as well as a gui to help analyse the algorithms.

An album showing a few gifs is located here: https://imgur.com/a/lULV3.

# How to run
*Note*: The code is only tested under linux, as well as having parts that currently only work under linux (watching shaders and recompiling them use inotify, for example), and has glfw3 and Assimp as external dependencies as well as meson for building the project.

```
meson build
cd build
ninja
./rbs
```
