#ifndef GL_RESOURCES_HPP
#define GL_RESOURCES_HPP

#include "glad/glad.h"
#include "lt_core.hpp"
#include "mesh.hpp"

#ifndef RESOURCES_PATH
#define RESOURCES_PATH "/home/lhahn/dev/cpp/cg-playground/resources/"
#endif

enum class BufferType
{
    UnitCube,
    UnitPlane,
};

enum class ReadBuffer
{
    OnlyVertices, All,
};

// struct GLResources
// {
//     lt_local_persist GLResources& instance()
//     {
//         lt_local_persist GLResources inst;
//         return inst;
//     }

//     void      initialize();
//     VAOHandle attrs_vertice_normal_texture(BufferType buffer_type);
//     VAOHandle attrs_vertices(BufferType buffer_type);

//     VAOHandle register_mesh(Mesh &mesh, Mesh::VertexFormat f, ReadBuffer rb = ReadBuffer::All);

// private:
//     GLResources() {}

// public:
//     GLResources(const GLResources&) = delete;
//     void operator=(const GLResources&) = delete;
// };

#endif // GL_RESOURCES_HPP
