#ifndef DRAW_HPP
#define DRAW_HPP

#include "lt_core.hpp"

struct Mesh;
struct Shader;
struct GLContext;

void draw_mesh(const Mesh& mesh, Shader& shader, GLContext& context);

#endif // DRAW_HPP
