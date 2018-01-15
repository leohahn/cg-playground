#ifndef GL_CONTEXT_HPP
#define GL_CONTEXT_HPP

#include "glad/glad.h"
#include "shader.hpp"

struct GLContext
{
    GLuint bound_program;
    GLuint bound_vao;

    explicit GLContext() : bound_program(0) {}

    void
    use_shader(const Shader& shader)
    {
        if (bound_program != shader.program)
        {
            glUseProgram(shader.program);
            bound_program = shader.program;
        }
    }

    void
    bind_vao(GLuint vao)
    {
        if (bound_vao != vao)
        {
            glBindVertexArray(vao);
            bound_vao = vao;
        }
    }

    inline void
    unbind_vao()
    {
        glBindVertexArray(0);
    }

	inline void
	enable_multisampling()
	{
		glEnable(GL_MULTISAMPLE);
	}

	inline void
	disable_multisampling()
	{
		glDisable(GL_MULTISAMPLE);
	}
};

#endif // GL_CONTEXT_HPP
