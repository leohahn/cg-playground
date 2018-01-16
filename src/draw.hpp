#ifndef DRAW_HPP
#define DRAW_HPP

#include "lt_core.hpp"

struct Mesh;
struct Shader;
struct GLContext;
struct CubemapMesh;
union Mat4f;

void draw_mesh(const Mesh *mesh, Shader &shader, GLContext &context);
void draw_skybox(const Mesh *skybox_mesh, Shader &shader, const Mat4f &view, GLContext &context);

#endif // DRAW_HPP
