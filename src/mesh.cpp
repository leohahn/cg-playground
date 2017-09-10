#include "mesh.hpp"
#include <cstring>

#include "glad/glad.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<u32> indices, std::vector<Texture> textures)
    : vertices(vertices)
    , indices(indices)
    , textures(textures)
{
    setup_mesh();
}

Mesh
Mesh::static_unit_cube(u32 diffuse_texture, u32 specular_texture)
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Texture> textures;

    for (usize i = 0; i < LT_Count(UNIT_CUBE_VERTICES); ++i) vertices.push_back(UNIT_CUBE_VERTICES[i]);
    for (usize i = 0; i < LT_Count(UNIT_CUBE_INDICES); ++i) indices.push_back(UNIT_CUBE_INDICES[i]);

    textures.push_back(Texture(diffuse_texture, "texture_diffuse"));
    textures.push_back(Texture(specular_texture, "texture_specular"));

    return Mesh(vertices, indices, textures);
}

Mesh
Mesh::static_unit_plane(u32 diffuse_texture, u32 specular_texture)
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Texture> textures;

    for (usize i = 0; i < LT_Count(UNIT_PLANE_VERTICES); ++i) vertices.push_back(UNIT_PLANE_VERTICES[i]);
    for (usize i = 0; i < LT_Count(UNIT_PLANE_INDICES); ++i) indices.push_back(UNIT_PLANE_INDICES[i]);

    textures.push_back(Texture(diffuse_texture, "texture_diffuse"));
    textures.push_back(Texture(specular_texture, "texture_specular"));

    return Mesh(vertices, indices, textures);
}

void
Mesh::setup_mesh()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32), &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    // vertex normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // vertex texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}
