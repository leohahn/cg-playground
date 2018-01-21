#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <array>
#include <string>

#include "lt_core.hpp"
#include "lt_math.hpp"

struct Texture
{
    u32 id;
    std::string type;

    Texture(u32 id, const char *type) : id(id), type(type) {}
};

typedef Vec3i Face;

struct Submesh
{
	isize                start_index;
	i32                  num_indices;
	std::vector<Texture> textures;
};

struct Mesh
{
	isize id;
    u32 vao = 0, vbo = 0, ebo = 0;
	std::vector<Submesh>            submeshes;

	std::vector<Vec3f>              vertices;
	std::vector<Vec2f>              tex_coords;

	// Calculated
	std::vector<Vec3f>              normals;
	std::vector<Vec3f>              tangents;
	std::vector<Vec3f>              bitangents;

	std::vector<Face>               faces;
	std::vector<std::vector<isize>> faces_textures; // TODO: find eventually a better format than this
    // std::vector<Texture>            textures;

	~Mesh();

	inline isize number_of_indices() const {return faces.size() * 3;}
};

#endif // MESH_HPP
