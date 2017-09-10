#include "gl_resources.hpp"

// #include <unordered_map>
// #include "glad/glad.h"
// #include "lt_utils.hpp"

// lt_internal lt::Logger logger("gl_resources");
// lt_internal std::unordered_map<GLuint, i32> g_vaos;
// lt_internal std::unordered_map<BufferType, u32> g_buffers;

// lt_internal inline VAOHandle
// make_vao_handle()
// {
//     GLuint vao;
//     glGenVertexArrays(1, &vao);
//     return VAOHandle(vao);
// }

// lt_internal inline VBOHandle
// make_vbo_handle()
// {
//     GLuint vbo;
//     glGenBuffers(1, &vbo);
//     return VBOHandle(vbo);
// }

// void
// GLResources::initialize()
// {
//     logger.log("Setting up UnitCube buffer.");
//     u32 unit_cube_vbo;
//     glGenBuffers(1, &unit_cube_vbo);
//     glBindBuffer(GL_ARRAY_BUFFER, unit_cube_vbo);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(UNIT_CUBE_VERTICES), UNIT_CUBE_VERTICES, GL_STATIC_DRAW);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);

//     logger.log("Setting up UnitPlane buffer.");
//     u32 unit_plane_vbo;
//     glGenBuffers(1, &unit_plane_vbo);
//     glBindBuffer(GL_ARRAY_BUFFER, unit_plane_vbo);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(UNIT_PLANE_VERTICES), UNIT_PLANE_VERTICES, GL_STATIC_DRAW);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);

//     g_buffers[BufferType::UnitCube] = unit_cube_vbo;
//     g_buffers[BufferType::UnitPlane] = unit_plane_vbo;
// }

// VAOHandle
// GLResources::register_mesh(Mesh &mesh, Mesh::VertexFormat f, ReadBuffer rb)
// {
//     VAOHandle vao_handle = make_vao_handle();
//     if (!mesh.vbo_handle.is_valid())
//         mesh.vbo_handle = make_vbo_handle();

//     switch (f)
//     {
//     case Mesh::VertexFormat::VerticeNormalTexture: {
//         if (rb == ReadBuffer::OnlyVertices)
//         {
//             glBindVertexArray(vao_handle.gl_vao);
//             glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_handle.gl_vbo);
//             glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)0);
//             glEnableVertexAttribArray(0);
//         }
//         else if (rb == ReadBuffer::All)
//         {
//             glBindVertexArray(vao_handle.gl_vao);
//             glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_handle.gl_vbo);
//             // Vertice
//             glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)0);
//             glEnableVertexAttribArray(0);
//             // Normal
//             glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
//             glEnableVertexAttribArray(1);
//             // Texture
//             glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(6*sizeof(GLfloat)));
//             glEnableVertexAttribArray(2);
//         }
//         else LT_Assert(false);
//     } break;

//     default: LT_Assert(false);
//     }

//     return vao_handle;
// }
