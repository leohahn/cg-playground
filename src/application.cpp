#include "application.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "resources.hpp"
#include "lt_utils.hpp"

lt_global_variable lt::Logger logger("application");

lt_internal void
framebuffer_size_callback(GLFWwindow *w, i32 width, i32 height)
{
    LT_Unused(w);
    glViewport(0, 0, width, height);
}

Application::~Application()
{
	glDeleteTextures(1, &hdr_texture);
	glDeleteTextures(1, &bloom_texture);
	glDeleteFramebuffers(1, &hdr_fbo);
	glDeleteRenderbuffers(1, &hdr_rbo);
}

Application
application_create_and_set_context(Resources &resources, const char *title, i32 width, i32 height)
{
	logger.log("Creating the application.");
	Application app = {};
	app.title = title;
	app.screen_width = width;
	app.screen_height = height;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);
	glfwWindowHint(GLFW_SAMPLES, 4);

    app.window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!app.window)
    {
        glfwTerminate();
        LT_Fail("Failed to create glfw window.\n");
    }

    glfwMakeContextCurrent(app.window);
    glfwSetFramebufferSizeCallback(app.window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        LT_Fail("Failed to initialize GLAD\n");

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
    glViewport(0, 0, width, height);

	{
		// Create the default HDR framebuffer.
		glGenFramebuffers(1, &app.hdr_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, app.hdr_fbo);

		// Create the default HDR texture.
		glGenTextures(1, &app.hdr_texture);
		glBindTexture(GL_TEXTURE_2D, app.hdr_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, app.hdr_texture, 0);

		// Create the bloom texture.
		glGenTextures(1, &app.bloom_texture);
		glBindTexture(GL_TEXTURE_2D, app.bloom_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, app.bloom_texture, 0);

		// Create a depth renderbuffer for the fbo.
		glGenRenderbuffers(1, &app.hdr_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, app.hdr_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, app.hdr_rbo);

		// Specify opengl to render to the two attachments specified above.
		const u32 attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
		glDrawBuffers(2, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			logger.error("Failed to properly create the HDR framebuffer for the application.");
	}
	// Create pingpong buffers
	{
		glGenFramebuffers(2, app.pingpong_fbos);
		glGenTextures(2, app.pingpong_textures);
		for (i32 i = 0; i < LT_Count(app.pingpong_fbos); i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, app.pingpong_fbos[i]);
			glBindTexture(GL_TEXTURE_2D, app.pingpong_textures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
								   app.pingpong_textures[i], 0);
		}
	}

	app.render_quad = resources.load_hdr_render_quad(app.hdr_texture);

    return app;
}
