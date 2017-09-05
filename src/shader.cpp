#include "shader.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "glad/glad.h"

#include "gl_resources.hpp"
#include "lt_core.hpp"
#include "lt_fs.hpp"
#include "lt_utils.hpp"

struct Shader
{
    GLuint program;
#ifdef DEV_ENV
    // This is used whenever the shader is recompiled, we have to reuse it again.
    bool is_stale;
#endif
};

lt_internal const char *shader_names[] =
{
#define SHADER_KIND(v, s) (const char*)s
    SHADER_KINDS
#undef SHADER_KIND
};

lt_internal lt::Logger logger("shader");
// TODO: Try not to use hardcoded path.
lt_internal Shader g_shaders[ShaderKind_Count] = {};
lt_internal i32 g_current_shader_index = -1;
lt_internal std::function<void(ShaderKind)> g_on_recompilation_handler = nullptr;

void
shader_set(ShaderKind kind)
{
    LT_Assert(kind != ShaderKind_Count);

#ifdef DEV_ENV
    bool should_reuse_shader = g_current_shader_index != kind || g_shaders[kind].is_stale == true;
#else
    bool should_reuse_shader = g_current_shader_index != kind;
#endif
    if (should_reuse_shader)
    {
        // logger.log("Using ", shader_names[kind], " shader");
        glUseProgram(g_shaders[kind].program);
#ifdef DEV_ENV
        g_shaders[kind].is_stale = false;
#endif
        g_current_shader_index = kind;
    }
}

void
shader_on_recompilation(const std::function<void(ShaderKind)> &handler)
{
    LT_Assert(g_on_recompilation_handler == nullptr);
    g_on_recompilation_handler = handler;
}

lt_internal GLuint
make_program(const char* shader_name)
{
    using std::string;

    // Fetch source codes from each shader
    string shader_src_path = string(RESOURCES_PATH) + string(shader_name);
    FileContents *shader_src = file_read_contents(shader_src_path.c_str());

    LT_Assert(shader_src != nullptr);

    if (shader_src->error != FileError_None) {
        fprintf(stderr, "Error reading shader source from %s\n", shader_src_path.c_str());
        file_free_contents(shader_src);
        return 0;
    }

    string shader_string((char*)shader_src->data, (char*)shader_src->data + shader_src->size - 1);

    file_free_contents(shader_src);

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    if (vertex_shader == 0 || fragment_shader == 0) {
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

GLuint shader_get_program(ShaderKind kind) { return g_shaders[kind].program; }

void
shader_recompile(ShaderKind kind)
{
    switch (kind)
    {
    case ShaderKind_Basic: {
        logger.log("Recompiling Basic Shader");
        GLuint old_program = g_shaders[kind].program;
        GLuint new_program = make_program("basic.glsl");

        if (new_program == 0)
            return;

        g_shaders[kind].program = new_program;
#ifdef DEV_ENV
        // Set the shader as stale, so the next time someone calls shader_set, the shader is reused.
        g_shaders[kind].is_stale = true;
#endif
        glDeleteProgram(old_program);
    } break;

    default:
        LT_Assert(false);
    }

    if (g_on_recompilation_handler) g_on_recompilation_handler(kind);
}

void
shader_initialize(void)
{
    Shader basic_shader;
    basic_shader.program = make_program("basic.glsl");
    basic_shader.is_stale = false;

    Shader light_shader;
    light_shader.program = make_program("light.glsl");
    light_shader.is_stale = false;

    g_shaders[ShaderKind_Basic] = basic_shader;
    g_shaders[ShaderKind_Light] = light_shader;
}
