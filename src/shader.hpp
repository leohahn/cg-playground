#ifndef SHADER_H
#define SHADER_H
#include "glad/glad.h"

#include <functional>
#include <unordered_map>
#include "lt_core.hpp"
#include "lt_math.hpp"

struct GLContext;

struct Shader
{
    const char *name;
    u32 program;

    explicit Shader(const char *name);
    ~Shader();

    void recompile();
    void on_recompilation(const std::function<void()> &handler);
    void setup_projection_matrix(f32 aspect_ratio, GLContext &context);

    void set3f(const char *name, Vec3f v);
    void set1i(const char *name, i32 i);
    void set1f(const char *name, f32 f);
    void set_matrix(const char *name, const Mat4f &m);

	void add_texture(const char *name, GLContext &context);
	u32 texture_unit(const char *name) const;
	u32 texture_unit(const std::string &name) const;

private:
	i32 m_next_texture_unit;
    std::unordered_map<std::string, u32> m_locations;
	std::unordered_map<std::string, u32> m_texture_units;
    std::function<void()> m_recompilation_handler;

    u32 get_location(const char *name);
};

#endif // SHADER_H
