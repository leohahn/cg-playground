#ifndef __RESOURCES_HPP__
#define __RESOURCES_HPP__

#include <vector>
#include "lt_core.hpp"
#include "mesh.hpp"

const i32 MAX_NUM_MESHES = 50;

struct Mesh;

// enum StaticResource
// {
// 	StaticResource_UnitPlane,
// 	StaticResource_UnitCube,
// };

// enum VertexFormat
// {
// 	VertexF
// };

struct Vertex_PU
{
	Vec3f position;
	Vec2f tex_coords;
};

static_assert(sizeof(Vertex_PU) == sizeof(f32)*5, "Vertex_PU should be packed.");

struct Vertex_PUNTB
{
	Vec3f position;
	Vec2f tex_coords;
	Vec3f normal;
	Vec3f tangent;
	Vec3f bitangent;
};

static_assert(sizeof(Vertex_PUNTB) == sizeof(f32)*14, "Vertex_PUNTB should be packed.");

struct Resources
{
	Mesh meshes[MAX_NUM_MESHES] = {};

	Mesh *load_cubemap(u32 cubemap_texture);
	Mesh *load_unit_cube(u32 diffuse_texture, u32 specular_texture, u32 normal_texture = 0);
	Mesh *load_unit_plane(f32 tex_coords_scale, u32 diffuse_texture,
						  u32 specular_texture, u32 normal_texture = 0);
	Mesh *load_shadow_map_render_surface(u32 shadow_map_texture);
	Mesh *load_hdr_render_quad(u32 hdr_texture);
	Mesh *load_mesh_from_model(const char *path, u32 diffuse_texture, u32 specular_texture,
							   u32 normal_texture, Resources &resources);

private:
	inline i64 get_new_id()
	{
		lt_local_persist i64 mesh_id = 0;
		LT_Assert(mesh_id < MAX_NUM_MESHES);
		return mesh_id++;
	}
};

#endif // __RESOURCES_HPP__
