#include "mesh.hpp"
#include <cstring>
#include "lt/src/lt_utils.hpp"

#include "glad/glad.h"

lt_global_variable lt::Logger logger("mesh");

Mesh::~Mesh()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);
}

