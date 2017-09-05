#ifndef SHADER_H
#define SHADER_H

#include <functional>
#include "glad/glad.h"

#define SHADER_KINDS                            \
    SHADER_KIND(ShaderKind_Basic, "Basic"),     \
    SHADER_KIND(ShaderKind_Light, "Light"), \
    SHADER_KIND(ShaderKind_Count, "Count")

enum ShaderKind
{
#define SHADER_KIND(v, s) v
    SHADER_KINDS
#undef SHADER_KIND
};

void   shader_initialize(void);
GLuint shader_get_program(ShaderKind kind);
void   shader_recompile(ShaderKind kind);
void   shader_on_recompilation(const std::function<void(ShaderKind)> &handler);
void   shader_set(ShaderKind kind);

#endif // SHADER_H
