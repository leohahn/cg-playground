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
    Vec2f tex_coords;
    Vec3f normal;
	Vec3f tangent;
	Vec3f bitangent;
};

static_assert(sizeof(Vertex) == sizeof(f32) * 14, "Vertex should be packed.");

struct Texture
{
    u32 id;
    std::string type;

    Texture(u32 id, const char *type) : id(id), type(type) {}
};

typedef Vec3i Face;

// struct TriangleList
// {
	
// };

struct Vertex_PUNTB
{
	Vec3f position;
	Vec2f tex_coords;
	Vec3f normal;
	Vec3f tangent;
	Vec3f bitangent;
};

static_assert(sizeof(Vertex_PUNTB) == sizeof(f32)*14, "Vertex_PUNTB should be packed.");

struct Mesh
{
	isize id;
    u32 vao = 0, vbo = 0, ebo = 0;
	// std::vector<TriangleList>       triangle_lists;

	std::vector<Vec3f>              vertices;
	std::vector<Vec2f>              tex_coords;

	// Calculated
	std::vector<Vec3f>              normals;
	std::vector<Vec3f>              tangents;
	std::vector<Vec3f>              bitangents;

	std::vector<Face>               faces;
	std::vector<std::vector<isize>> faces_textures; // TODO: find eventually a better format than this
    std::vector<Texture>            textures;

	~Mesh();

	inline isize number_of_indices() const {return faces.size() * 3;}
};

// Mesh make_mesh_cubemap(u32 cubemap_texture);
// Mesh make_mesh_unit_cube(u32 diffuse_texture, u32 specular_texture, u32 normal_texture = 0);
// Mesh make_mesh_unit_plane(f32 tex_coords_scale, u32 diffuse_texture,
// 						  u32 specular_texture, u32 normal_texture = 0);

#endif // MESH_HPP
