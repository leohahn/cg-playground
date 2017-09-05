#ifndef GL_RESOURCES_HPP
#define GL_RESOURCES_HPP

#include "lt_core.hpp"

#ifndef RESOURCES_PATH
#define RESOURCES_PATH "/home/lhahn/dev/cpp/rigid-body-simulation/resources/"
#endif

enum class BufferType
{
    UnitCube,
    UnitPlane,
};

void gl_resources_initialize();

void gl_resources_attrs_vertice_normal_texture(u32 vao, BufferType buffer_type);
void gl_resources_attrs_vertices(u32 vao, BufferType buffer_type);

u32  gl_resources_create_vao();
void gl_resources_release_vao();

constexpr f32 UNIT_CUBE_VERTICES[] = {
    // vertices            normals               texture coords
    // FRONT
    -1.0f, -1.0f, 1.0f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
     1.0f, -1.0f, 1.0f,    0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
     1.0f,  1.0f, 1.0f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f,

     1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
    // BACK
    -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
     1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,

     1.0f,  1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
     1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
    // LEFT
    -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,

    -1.0f,  1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
    // RIGHT
     1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
     1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

     1.0f,  1.0f, -1.0f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
     1.0f,  1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
     1.0f, -1.0f,  1.0f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
    // TOP
    -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
     1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
     1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,

     1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
    // BOTTOM
    -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
     1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,

     1.0f, -1.0f, -1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
     1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
};
constexpr f32 UNIT_CUBE_NUM_VERTICES = LT_Count(UNIT_CUBE_VERTICES) / 8.0f;

constexpr f32 UNIT_PLANE_VERTICES[] = {
    // vertices           normals             texture coords
    -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
     1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,   10.0f,10.0f,
     1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,   10.0f, 0.0f,

     1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,   10.0f, 0.0f,
    -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
    -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
};
constexpr f32 UNIT_PLANE_NUM_VERTICES = LT_Count(UNIT_PLANE_VERTICES) / 8.0f;

#endif // GL_RESOURCES_HPP
