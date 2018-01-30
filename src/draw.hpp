#ifndef DRAW_HPP
#define DRAW_HPP

#include "lt_core.hpp"
#include "lt_math.hpp"
#include "glad/glad.h"
#include "entities.hpp"

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
void draw_entities(f64 lag_offset, const Entities &e, const Camera &camera, GLContext &context,
				   ShadowMap &shadow_map, EntityHandle selected_entity = -1);
void draw_entities_for_shadow_map(const Entities &e, const Mat4f &light_view, const Vec3f &light_pos,
								  ShadowMap &shadow_map, GLContext &context);
void draw_unit_quad(Mesh *mesh, Shader &shader, GLContext &context);
void draw_selected_entity(const Entities &e, EntityHandle handle, Shader &selection_shader,
						  const Mat4f &view, GLContext &context);

#endif // DRAW_HPP
