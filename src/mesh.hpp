#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <array>
#include <string>

#include "lt_core.hpp"
#include "lt_math.hpp"

struct Vertex
{
    Vec3f position;
    Vec3f normal;
    Vec2f tex_coords;

    Vertex(Vec3f position, Vec3f normal, Vec2f tex_coords)
        : position(position)
        , normal(normal)
        , tex_coords(tex_coords)
    {}
};

struct Texture
{
    u32 id;
    std::string type;

    Texture(u32 id, const char *type) : id(id), type(type) {}
};

struct Mesh
{
    std::vector<Vertex>    vertices;
    std::vector<u32>       indices;
    std::vector<Texture>   textures;
    u32 vao, vbo, ebo;

    Mesh(std::vector<Vertex> vertexes, std::vector<u32> indices, std::vector<Texture> textures);

    static Mesh static_unit_cube(u32 diffuse_texture, u32 specular_texture);
    static Mesh static_unit_plane(u32 diffuse_texture, u32 specular_texture);

private:

    void setup_mesh();
};

const Vertex UNIT_CUBE_VERTICES[] = {
    // vertices                        normals                     texture coords
    // FRONT
    Vertex(Vec3f(-1.0f, -1.0f,  1.0f), Vec3f( 0.0f,  0.0f,  1.0f), Vec2f(0.0f, 0.0f)), // 0
    Vertex(Vec3f( 1.0f, -1.0f,  1.0f), Vec3f( 0.0f,  0.0f,  1.0f), Vec2f(1.0f, 0.0f)), // 1
    Vertex(Vec3f( 1.0f,  1.0f,  1.0f), Vec3f( 0.0f,  0.0f,  1.0f), Vec2f(1.0f, 1.0f)), // 2
    Vertex(Vec3f(-1.0f,  1.0f,  1.0f), Vec3f( 0.0f,  0.0f,  1.0f), Vec2f(0.0f, 1.0f)), // 3
    // BACK
    Vertex(Vec3f(-1.0f, -1.0f, -1.0f), Vec3f( 0.0f,  0.0f, -1.0f), Vec2f(0.0f, 0.0f)), // 4
    Vertex(Vec3f(-1.0f,  1.0f, -1.0f), Vec3f( 0.0f,  0.0f, -1.0f), Vec2f(0.0f, 1.0f)), // 5
    Vertex(Vec3f( 1.0f,  1.0f, -1.0f), Vec3f( 0.0f,  0.0f, -1.0f), Vec2f(1.0f, 1.0f)), // 6
    Vertex(Vec3f( 1.0f, -1.0f, -1.0f), Vec3f( 0.0f,  0.0f, -1.0f), Vec2f(1.0f, 0.0f)), // 7
    // LEFT
    Vertex(Vec3f(-1.0f, -1.0f,  1.0f), Vec3f(-1.0f,  0.0f,  0.0f), Vec2f(1.0f, 0.0f)), // 8
    Vertex(Vec3f(-1.0f,  1.0f,  1.0f), Vec3f(-1.0f,  0.0f,  0.0f), Vec2f(1.0f, 1.0f)), // 9
    Vertex(Vec3f(-1.0f,  1.0f, -1.0f), Vec3f(-1.0f,  0.0f,  0.0f), Vec2f(0.0f, 1.0f)), // 10
    Vertex(Vec3f(-1.0f, -1.0f, -1.0f), Vec3f(-1.0f,  0.0f,  0.0f), Vec2f(0.0f, 0.0f)), // 11
    // RIGHT
    Vertex(Vec3f( 1.0f, -1.0f,  1.0f), Vec3f(1.0f,  0.0f,  0.0f), Vec2f(0.0f, 1.0f)), // 12
    Vertex(Vec3f( 1.0f, -1.0f, -1.0f), Vec3f(1.0f,  0.0f,  0.0f), Vec2f(1.0f, 1.0f)), // 13
    Vertex(Vec3f( 1.0f,  1.0f, -1.0f), Vec3f(1.0f,  0.0f,  0.0f), Vec2f(1.0f, 0.0f)), // 14
    Vertex(Vec3f( 1.0f,  1.0f,  1.0f), Vec3f(1.0f,  0.0f,  0.0f), Vec2f(0.0f, 0.0f)), // 15
    // TOP
    Vertex(Vec3f(-1.0f,  1.0f,  1.0f), Vec3f(0.0f,  1.0f,  0.0f), Vec2f(0.0f, 1.0f)), // 16
    Vertex(Vec3f( 1.0f,  1.0f,  1.0f), Vec3f(0.0f,  1.0f,  0.0f), Vec2f(1.0f, 1.0f)), // 17
    Vertex(Vec3f( 1.0f,  1.0f, -1.0f), Vec3f(0.0f,  1.0f,  0.0f), Vec2f(1.0f, 0.0f)), // 18
    Vertex(Vec3f(-1.0f,  1.0f, -1.0f), Vec3f(0.0f,  1.0f,  0.0f), Vec2f(0.0f, 0.0f)), // 19
    // BOTTOM
    Vertex(Vec3f(-1.0f, -1.0f,  1.0f), Vec3f(0.0f, -1.0f,  0.0f), Vec2f(0.0f, 1.0f)), // 20
    Vertex(Vec3f(-1.0f, -1.0f, -1.0f), Vec3f(0.0f, -1.0f,  0.0f), Vec2f(0.0f, 0.0f)), // 21
    Vertex(Vec3f( 1.0f, -1.0f, -1.0f), Vec3f(0.0f, -1.0f,  0.0f), Vec2f(1.0f, 0.0f)), // 22
    Vertex(Vec3f( 1.0f, -1.0f,  1.0f), Vec3f(0.0f, -1.0f,  0.0f), Vec2f(1.0f, 1.0f)), // 23
};
const u32 UNIT_CUBE_INDICES[] = {
     0,  1,  2,  2,  3,  0,
     4,  5,  6,  6,  7,  4,
     8,  9, 10, 10, 11,  8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
};

const Vertex UNIT_PLANE_VERTICES[] = {
    // vertices           normals             texture coords
    Vertex(Vec3f(-1.0f,  0.0f,  1.0f), Vec3f(0.0f, 1.0f, 0.0f),  Vec2f(0.0f, 10.0f)), // 0
    Vertex(Vec3f( 1.0f,  0.0f,  1.0f), Vec3f(0.0f, 1.0f, 0.0f),  Vec2f(10.0f,10.0f)), // 1
    Vertex(Vec3f( 1.0f,  0.0f, -1.0f), Vec3f(0.0f, 1.0f, 0.0f),  Vec2f(10.0f, 0.0f)), // 2
    Vertex(Vec3f(-1.0f,  0.0f, -1.0f), Vec3f(0.0f, 1.0f, 0.0f),  Vec2f(0.0f, 0.0f)), // 3
};
const u32 UNIT_PLANE_INDICES[] = {
    0, 1, 2, 2, 3, 0
};


#endif // MESH_HPP
