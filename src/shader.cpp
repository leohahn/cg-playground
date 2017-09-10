#include "shader.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unordered_map>

#include "glad/glad.h"

#include "gl_resources.hpp"
#include "lt_fs.hpp"
#include "lt_utils.hpp"
#include "gl_context.hpp"

lt_internal lt::Logger logger("shader");

lt_internal GLuint
make_program(const char* shader_name)
{
    using std::string;

    // Fetch source codes from each shader
    string shader_src_path = string(RESOURCES_PATH) + string(shader_name);
    FileContents *shader_src = file_read_contents(shader_src_path.c_str());

    LT_Assert(shader_src != nullptr);

    if (shader_src->error != FileError_None)
    {
        fprintf(stderr, "Error reading shader source from %s\n", shader_src_path.c_str());
        file_free_contents(shader_src);
        return 0;
    }

    string shader_string((char*)shader_src->data, (char*)shader_src->data + shader_src->size - 1);

    file_free_contents(shader_src);

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    if (vertex_shader == 0 || fragment_shader == 0)
    {
        fprintf(stderr, "Error creating shaders (glCreateShader)\n");
        file_free_contents(shader_src);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }

    GLuint program = 0;
    GLchar info[512] = {};
    GLint success;
    {
        const char *vertex_string[3] = {
            "#version 330 core\n",
            "#define COMPILING_VERTEX\n",
            shader_string.c_str(),
        };
        glShaderSource(vertex_shader, 3, &vertex_string[0], NULL);

        const char *fragment_string[3] = {
            "#version 330 core\n",
            "#define COMPILING_FRAGMENT\n",
            shader_string.c_str(),
        };
        glShaderSource(fragment_shader, 3, &fragment_string[0], NULL);
    }

    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, info);
        printf("ERROR: Vertex shader compilation failed:\n");
        printf("%s\n", info);
        goto error_cleanup;
    }

    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, info);
        printf("ERROR: Fragment shader compilation failed:\n");
        printf("%s\n", info);
        goto error_cleanup;
    }

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info);
        printf("ERROR: Shader linking failed:\n");
        printf("%s\n", info);
        goto error_cleanup;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;

error_cleanup:
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return 0;
}

void
Shader::recompile()
{
    logger.log("Recompiling ", name, " shader");
    GLuint old_program = program;
    GLuint new_program = make_program(name);

    if (new_program == 0)
        return;

    program = new_program;
    glDeleteProgram(old_program);

    if (m_recompilation_handler)
        m_recompilation_handler();
}

Shader::Shader(const char *name)
    : name(name)
{
    program = make_program(name);
}

void
Shader::on_recompilation(const std::function<void()> &handler)
{
    LT_Assert(m_recompilation_handler == nullptr);
    m_recompilation_handler = handler;
}

void
Shader::setup_projection_matrix(f32 aspect_ratio, GLContext &context)
{
    const Mat4f projection = lt::perspective<f32>(60.0f, aspect_ratio, 0.1f, 100.0f);

    context.use_shader(*this);
    glUniformMatrix4fv(get_location("projection"), 1, GL_FALSE, projection.data());
}

void
Shader::set3f(const char *name, Vec3f v)
{
    glUniform3f(get_location(name), v.x, v.y, v.z);
}

void
Shader::set1i(const char *name, i32 i)
{
    glUniform1i(get_location(name), i);
}

void
Shader::set1f(const char *name, f32 f)
{
    glUniform1f(get_location(name), f);
}

void
Shader::set_matrix(const char *name, const Mat4f &m)
{
    glUniformMatrix4fv(get_location(name), 1, GL_FALSE, m.data());
}

GLuint
Shader::get_location(const char *name)
{
    if (m_locations.find(name) == m_locations.end())
    {
        const GLuint location = glGetUniformLocation(program, name);
        m_locations[std::string(name)] = location;
    }

    return m_locations.at(name);
}
