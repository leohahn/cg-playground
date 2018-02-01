#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include "lt_core.hpp"

struct GLFWwindow;
struct Mesh;
struct Resources;

struct Application
{
	GLFWwindow *window;
	const char *title;
	i32         screen_width;
	i32         screen_height;
	u32         hdr_fbo;
	u32         hdr_texture;
	u32         bloom_texture;
	u32         hdr_rbo;
	Mesh       *render_quad;
	u32         pingpong_fbos[2];
	u32         pingpong_textures[2];

	~Application();
};

Application application_create_and_set_context(Resources &resources, const char *title, i32 width, i32 height);


#endif // __APPLICATION_HPP__
