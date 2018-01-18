#ifndef __ENTITIES_HPP__
#define __ENTITIES_HPP__

#include "lt_core.hpp"
#include "lt_math.hpp"

#define MAX_ENTITIES 100

struct Mesh;
struct Resources;
struct Shader;

enum ComponentKind : u32
{
	ComponentKind_None = 0,
	ComponentKind_Transform = (1 << 0),
	ComponentKind_Renderable = (1 << 1),
	ComponentKind_LightEmmiter = (1 << 2),
	ComponentKind_ShadowCaster = (1 << 3),
};

struct Transform
{
	Mat4f mat;
};

struct Renderable
{
	Mesh *mesh;
	Shader *shader;
	f32 shininess;
};

struct LightEmmiter
{
	Vec3f ambient;
	Vec3f diffuse;
	Vec3f specular;
	Vec3f position;
	f32 constant;
	f32 linear;
	f32 quadratic;
	Shader *shader;
};

struct ShadowCaster
{
	Shader *shader;
};

typedef isize EntityHandle;

struct Entities
{
	// initialize arrays to zero
	u32           mask[MAX_ENTITIES];
	Transform     transform[MAX_ENTITIES];
	Renderable    renderable[MAX_ENTITIES];
	LightEmmiter  light_emmiter[MAX_ENTITIES];
	ShadowCaster  shadow_caster[MAX_ENTITIES];

	EntityHandle create(u32 components_mask);
	void         destroy(EntityHandle id);
};

EntityHandle create_textured_cube(Entities *entities, Resources *resources, Shader *shader,
								  const Mat4f &transform, f32 shininess, Shader *shadow_cast_texture,
								  u32 diffuse_texture, u32 specular_texture, u32 normal_texture = 0);

EntityHandle create_point_light(Entities *entities, Resources *resources, Shader *shader,
								const Mat4f &transform, const LightEmmiter &light_emmiter,
								Shader *shadow_cast_texture, u32 diffuse_texture, u32 specular_texture);

EntityHandle create_plane(Entities *entities, Resources *resources, Shader *shader, const Mat4f &transform,
						  f32 shininess, f32 tex_coords_scale, Shader *shadow_cast_texture, u32 diffuse_texture,
						  u32 specular_texture, u32 normal_texture = 0);

EntityHandle create_skybox(Entities *entities, Resources *resources, Shader *shader, u32 skybox_texture);

#endif // __ENTITIES_HPP__
