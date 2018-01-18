#ifndef DRAW_HPP
#define DRAW_HPP

#include "lt_core.hpp"

struct Mesh;
struct Shader;
struct GLContext;
struct CubemapMesh;
struct Entities;
union Mat4f;
struct Camera;

void draw_skybox(const Mesh *skybox_mesh, Shader &shader, const Mat4f &view, GLContext &context);
void draw_entities(const Entities &entities, const Camera &camera, GLContext &context);
void draw_entities_for_shadow_map(const Entities &entities, const Camera &camera,
								  u32 shadow_map_texture, GLContext &context);

#endif // DRAW_HPP
