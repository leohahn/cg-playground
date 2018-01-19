#ifndef DRAW_HPP
#define DRAW_HPP

#include "lt_core.hpp"
#include "lt_math.hpp"
#include "glad/glad.h"

struct Mesh;
struct Shader;
struct GLContext;
struct CubemapMesh;
struct Entities;
struct Camera;

struct ShadowMap
{
	Shader *shader;
	u32 fbo;
	u32 texture;
	i32 width, height;

	~ShadowMap()
	{
		glDeleteFramebuffers(1, &fbo);
	}
};

ShadowMap create_shadow_map(i32 width, i32 height, Shader &shader);

void draw_skybox(const Mesh *skybox_mesh, Shader &shader, const Mat4f &view, GLContext &context);
void draw_entities(const Entities &e, const Camera &camera, GLContext &context, ShadowMap &shadow_map);
void draw_entities_for_shadow_map(const Entities &e, const Mat4f &light_view, const Vec3f &light_pos,
								  ShadowMap &shadow_map, GLContext &context);
void draw_shadow_map(Mesh *mesh, Shader &shadow_map_render_shader, GLContext &context);

#endif // DRAW_HPP
