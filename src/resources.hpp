#ifndef __RESOURCES_HPP__
#define __RESOURCES_HPP__

#include <vector>
#include "lt_core.hpp"
#include "mesh.hpp"

const i32 MAX_NUM_MESHES = 50;

struct Mesh;

enum StaticResource
{
	StaticResource_UnitPlane,
	StaticResource_UnitCube,
};

struct Resources
{
	Mesh meshes[MAX_NUM_MESHES] = {};

	Mesh *load_cubemap(u32 cubemap_texture);
	Mesh *load_unit_cube(u32 diffuse_texture, u32 specular_texture, u32 normal_texture = 0);
	Mesh *load_unit_plane(f32 tex_coords_scale, u32 diffuse_texture,
						  u32 specular_texture, u32 normal_texture = 0);
private:
	isize m_mesh_id = 0;
	inline isize get_new_id()
	{
		LT_Assert(m_mesh_id < MAX_NUM_MESHES);
		return m_mesh_id++;
	}
};

#endif // __RESOURCES_HPP__
