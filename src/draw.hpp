#ifndef DRAW_HPP
#define DRAW_HPP

#include "lt_core.hpp"
#include "lt_math.hpp"

struct Mesh;
struct Shader;
struct GLContext;
struct CubemapMesh;
struct Entities;
struct Camera;

void draw_skybox(const Mesh *skybox_mesh, Shader &shader, const Mat4f &view, GLContext &context);
void draw_entities(const Entities &e, const Camera &camera, GLContext &context);
void draw_entities_for_shadow_map(const Entities &e, const Mat4f &light_view, const Vec3f &light_pos,
								  u32 shadow_map_texture, GLContext &context);

#endif // DRAW_HPP
