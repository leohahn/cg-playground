#include "gl_resources.hpp"

#include <unordered_map>
#include "glad/glad.h"
#include "lt_utils.hpp"

lt_internal lt::Logger logger("gl_resources");
lt_internal std::unordered_map<BufferType, u32> g_buffers;

void
gl_resources_initialize()
{
    logger.log("Setting up UnitCube buffer.");
    u32 unit_cube_vbo;
    glGenBuffers(1, &unit_cube_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, unit_cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(UNIT_CUBE_VERTICES), UNIT_CUBE_VERTICES, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    logger.log("Setting up UnitPlane buffer.");
    u32 unit_plane_vbo;
    glGenBuffers(1, &unit_plane_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, unit_plane_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(UNIT_PLANE_VERTICES), UNIT_PLANE_VERTICES, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    g_buffers[BufferType::UnitCube] = unit_cube_vbo;
    g_buffers[BufferType::UnitPlane] = unit_plane_vbo;
}

void
gl_resources_attrs_vertices(u32 vao, BufferType buffer_type)
{
    u32 vbo = g_buffers.at(buffer_type);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
}

void
gl_resources_attrs_vertice_normal_texture(u32 vao, BufferType buffer_type)
{
    u32 vbo = g_buffers.at(buffer_type);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Vertice
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // Texture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(6*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
}

u32
gl_resources_get_vbo(BufferType type)
{
    return g_buffers.at(type);
}

u32
gl_resources_create_vao()
{
    u32 vao;
    glGenVertexArrays(1, &vao);
    return vao;
}

void
gl_resources_release_vao()
{

}
