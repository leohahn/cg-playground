#ifndef SHADER_H
#define SHADER_H

#include <functional>
#include <unordered_map>
#include "glad/glad.h"
#include "lt_core.hpp"
#include "lt_math.hpp"

struct GLContext;

struct Shader
{
    const char *name;
    GLuint program;

#ifdef DEV_ENV
    bool is_stale; // This is used whenever the shader is recompiled, we have to reuse it again.
#endif

    explicit Shader(const char *name);
    void recompile();
    void on_recompilation(const std::function<void()> &handler);
    void setup_projection_matrix(f32 aspect_ratio, GLContext &context);

    void set3f(const char *name, Vec3f v);
    void set1i(const char *name, i32 i);
    void set1f(const char *name, f32 f);
    void set_matrix(const char *name, const Mat4f &m);

private:
    std::unordered_map<std::string, GLuint> m_locations;
    std::function<void()> m_recompilation_handler;

    GLuint get_location(const char *name);
};

#endif // SHADER_H
