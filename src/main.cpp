#include <stdio.h>

#include <functional>
#include <string>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#ifdef __unix__
#include <linux/inotify.h>
#include <pthread.h>
#endif

#include "lt_core.hpp"
#include "lt_utils.hpp"
#include "lt_math.hpp"
#include "watcher.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "draw.hpp"
#include "gl_resources.hpp"
#include "gl_context.hpp"
#include "mesh.hpp"
#include "debug_gui.hpp"
#include "resources.hpp"
#include "stb_image.h"
#include "input.hpp"
#include "entities.hpp"

//
// TODOs
//
// - Add quaterion interpolation for better rotations. (SLERP)
//   - Hopefully then fix the game loop 
// - Add antialiasing to the offscreen HDR buffer.
//

lt_global_variable lt::Logger logger("main");
lt_global_variable bool g_display_debug_gui = true;
lt_global_variable Key g_keyboard[NUM_KEYBOARD_KEYS] = {};

lt_internal void
framebuffer_size_callback(GLFWwindow *w, i32 width, i32 height)
{
    LT_Unused(w);
    glViewport(0, 0, width, height);
}

lt_internal inline f64
get_time_milliseconds()
{
	return glfwGetTime() * 1000.0f;
}

lt_internal void
update_key_state(Key &key, bool key_pressed)
{
	if ((key_pressed && key.is_pressed) || (!key_pressed && !key.is_pressed))
	{
		key.last_transition = Key::Transition_None;
	}
	else if (key_pressed && !key.is_pressed)
	{
		key.is_pressed = true;
		key.last_transition = Key::Transition_Down;
	}
	else if (!key_pressed && key.is_pressed)
	{
		key.is_pressed = false;
		key.last_transition = Key::Transition_Up;
	}
}

lt_internal void
process_input(GLFWwindow *win, Key *kb)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
        glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(win, true);
    }

	update_key_state(kb[GLFW_KEY_W], glfwGetKey(win, GLFW_KEY_W));
	update_key_state(kb[GLFW_KEY_S], glfwGetKey(win, GLFW_KEY_S));
	update_key_state(kb[GLFW_KEY_A], glfwGetKey(win, GLFW_KEY_A));
	update_key_state(kb[GLFW_KEY_D], glfwGetKey(win, GLFW_KEY_D));
	update_key_state(kb[GLFW_KEY_UP], glfwGetKey(win, GLFW_KEY_UP));
	update_key_state(kb[GLFW_KEY_DOWN], glfwGetKey(win, GLFW_KEY_DOWN));
	update_key_state(kb[GLFW_KEY_LEFT], glfwGetKey(win, GLFW_KEY_LEFT));
	update_key_state(kb[GLFW_KEY_RIGHT], glfwGetKey(win, GLFW_KEY_RIGHT));
	update_key_state(kb[GLFW_KEY_F1], glfwGetKey(win, GLFW_KEY_F1));

	if (kb[GLFW_KEY_F1].last_transition == Key::Transition_Down)
		g_display_debug_gui = !g_display_debug_gui;
}

#ifdef DEV_ENV // NOTE: Do I need to wrap this function around ifdefs?
lt_internal void
process_watcher_events(Shader &basic_shader, Shader &light_shader)
{
    WatcherEvent *ev;
    while ((ev = watcher_peek_event()) != nullptr)
    {
        const bool is_dir = ev->inotify_mask & IN_ISDIR;
        const bool is_modified = ev->inotify_mask & IN_MODIFY;

        if (!is_dir && is_modified)
        {
            logger.log("File ", ev->name, " changed, requesting shader recompile.");
            if (ev->name == basic_shader.name) basic_shader.recompile();
            if (ev->name == light_shader.name) light_shader.recompile();
        }

        // Notify the watcher that the event was consumed.
        watcher_event_peeked();
    }
}
#endif

struct Application
{
	GLFWwindow *window;
	const char *title;
	i32         screen_width;
	i32         screen_height;
	u32         hdr_fbo;
	u32         hdr_texture;
	u32         hdr_rbo;
	Mesh       *render_quad;
};

lt_internal Application
create_application_and_set_context(Resources &resources, const char *title, i32 width, i32 height)
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

	// Create the default HDR texture
	glGenTextures(1, &app.hdr_texture);
	glBindTexture(GL_TEXTURE_2D, app.hdr_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// Create a depth renderbuffer for the fbo
	glGenRenderbuffers(1, &app.hdr_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, app.hdr_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	// Create the default HDR framebuffer
	glGenFramebuffers(1, &app.hdr_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, app.hdr_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, app.hdr_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, app.hdr_rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		logger.error("Failed to properly create the HDR framebuffer for the application.");

	app.render_quad = resources.load_hdr_render_quad(app.hdr_texture);

    return app;
}

enum TextureFormat
{
	TextureFormat_RGB = GL_RGB8,
	TextureFormat_RGBA = GL_RGBA,
	TextureFormat_SRGB = GL_SRGB8,
	TextureFormat_SRGBA = GL_SRGB_ALPHA,
};

enum PixelFormat
{
	PixelFormat_RGB = GL_RGB,
	PixelFormat_RGBA = GL_RGBA,
};

lt_internal u32
load_cubemap_texture(const char **textures_files, i32 num_textures,
					 TextureFormat texture_format, PixelFormat pixel_format)
{
	LT_Assert(num_textures == 6);

	// Create and bind the new texture
	u32 texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  

	logger.log("Loading cubemap");
	for (i32 i = 0; i < num_textures; i++)
	{
		std::string fullpath = std::string(RESOURCES_PATH) + std::string(textures_files[i]);

		i32 width, height, num_channels;
		u8 *image_data = stbi_load(fullpath.c_str(), &width, &height, &num_channels, 0);

		if (image_data)
		{
			const i32 mipmap_level = 0;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipmap_level, texture_format, width, height,
						 0, pixel_format, GL_UNSIGNED_BYTE, image_data);

			logger.log("    OK: ", fullpath);
			logger.log("    [", width, " x ", height, "] (num channels = ", num_channels, ")");

			stbi_image_free(image_data);
		}
		else
		{
			logger.error("    Failed loading texture ", fullpath);
		}
	}

	return texture;
}

lt_internal u32
load_texture(const char *path, TextureFormat texture_format, PixelFormat pixel_format)
{
    std::string fullpath = std::string(RESOURCES_PATH) + std::string(path);
    // Textures
	GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // Wrapping and filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    i32 width, height, num_channels;
    uchar *image_data = stbi_load(fullpath.c_str(), &width, &height, &num_channels, 0);
    if (image_data)
    {
        logger.log("Texture ", fullpath, " loaded successfuly.");
        logger.log("    [", width, " x ", height, "] (num channels = ", num_channels, ")");

		const i32 mipmap_level = 0;
        glTexImage2D(GL_TEXTURE_2D, mipmap_level, texture_format, width, height,
					 0, pixel_format, GL_UNSIGNED_BYTE, image_data);

        glGenerateMipmap(GL_TEXTURE_2D);

        logger.log("    OpenGL read texture successfully.");
    }
    else
    {
        logger.error("    Failed loading texture");
    }
    // Free image
    stbi_image_free(image_data);
	return texture;
}

struct DirectionalLight
{
	Vec3f direction;
	Vec3f ambient;
    Vec3f diffuse;
    Vec3f specular;
};

void
game_update(Key *kb, Camera& camera, dgui::State &state)
{
	camera.update(kb);
	// Update debug gui state variables.
	state.camera_pos = camera.frustum.position;
	state.camera_front = camera.frustum.front.v;
}

struct Shaders
{
    Shader *light;
    Shader *selection;
    Shader *hdr_texture_to_quad;
    Shader *basic;
    Shader *skybox;
    Shader *shadow_map;
    Shader *shadow_map_render;
};

lt_internal void
game_render(f64 lag_offset, const Application &app, Camera &camera, Entities &entities,
			Shaders &shaders, ShadowMap &shadow_map, const Mat4f &light_view, Vec3f dir_light_pos,
			Mesh *shadow_map_surface, Mesh *skybox_mesh, GLContext &context)
{
	LT_Assert(lag_offset < 1);
	LT_Assert(lag_offset >= 0);

	camera.interpolate_frustum(lag_offset);

	const Mat4f view_matrix = camera.view_matrix();

	// Render first to depth map
	glViewport(0, 0, shadow_map.width, shadow_map.height);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_map.fbo); // TODO: move to GLContext
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	draw_entities_for_shadow_map(entities, light_view, dir_light_pos, shadow_map, context);
	glEnable(GL_CULL_FACE);

	// Actual rendering
	glViewport(0, 0, app.screen_width, app.screen_height);
	glBindFramebuffer(GL_FRAMEBUFFER, app.hdr_fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (dgui::State::instance().draw_shadow_map)
	{
		// Draws the shadowmap instead of the scene
		draw_unit_quad(shadow_map_surface, *shaders.shadow_map_render, context);
	}
	else
	{
		// Enable stencil testing but disallow writing to it.
		// Writing to it will be enabled inside draw_entities.
		glStencilMask(0xff);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xff);
		glStencilMask(0x00);

		context.use_shader(*shaders.basic);
		shaders.basic->set1i("debug_gui_state.enable_normal_mapping",
							 dgui::State::instance().enable_normal_mapping);
		shaders.basic->set1f("debug_gui_state.pcf_texel_offset", dgui::State::instance().pcf_texel_offset);
		shaders.basic->set1i("debug_gui_state.pcf_window_side", dgui::State::instance().pcf_window_side);

		draw_entities(lag_offset, entities, camera, context, shadow_map, dgui::State::instance().selected_entity_handle);

		if (dgui::State::instance().selected_entity_handle != -1)
		{
			glStencilFunc(GL_NOTEQUAL, 1, 0xff);
			glStencilMask(0x00);

			draw_selected_entity(entities, dgui::State::instance().selected_entity_handle,
								 *shaders.selection, view_matrix, context);

			glStencilFunc(GL_ALWAYS, 1, 0xff);
		}

		// Don't update the stencil buffer for the skybox
		draw_skybox(skybox_mesh, *shaders.skybox, view_matrix, context);
	}

	if (g_display_debug_gui)
	{
		dgui::draw(app.window, entities);
	}

	// Draw HDR texture to a quad.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_unit_quad(app.render_quad, *shaders.hdr_texture_to_quad, context);

	glfwSwapBuffers(app.window);
}

int
main(void)
{
	/* ==================================================================================
     *     OpenGL and GLFW initialization
     * ================================================================================== */
    const i32 WINDOW_WIDTH = 1680;
    const i32 WINDOW_HEIGHT = 1050;
    const f32 ASPECT_RATIO = (f32)WINDOW_WIDTH / WINDOW_HEIGHT;

    logger.log("Initializing glfw");
    glfwInit();

	Resources resources = {};

    Application app = create_application_and_set_context(resources, "CG playground", WINDOW_WIDTH, WINDOW_HEIGHT);

#ifdef DEV_ENV
    pthread_t watcher_thread;
    logger.log("Creating watcher thread");
    pthread_create(&watcher_thread, nullptr, watcher_start, nullptr);
#endif

    GLContext context;

	//
	// Load shaders
	//
	// TODO: Fix shader recompilation.
	//    - All uniforms that are not added every frame have to be reapplied after
	//      recompilation for the process to work.
	//    - Currently only the projection matrix is updated
	//
	Shaders shaders = {};
    shaders.light = new Shader("light.glsl");
    shaders.light->on_recompilation([&] {
        shaders.light->setup_projection_matrix(ASPECT_RATIO, context);
    });

    shaders.selection = new Shader("selection.glsl");
    shaders.selection->on_recompilation([&] {
        shaders.selection->setup_projection_matrix(ASPECT_RATIO, context);
    });

    shaders.hdr_texture_to_quad = new Shader("render-hdr-texture-to-quad.glsl");
	shaders.hdr_texture_to_quad->add_texture("texture_hdr", context);

    shaders.basic = new Shader("basic.glsl");
	shaders.basic->add_texture("material.texture_diffuse1", context);
	shaders.basic->add_texture("material.texture_specular1", context);
	shaders.basic->add_texture("material.texture_normal1", context);
	shaders.basic->add_texture("texture_shadow_map", context);
    shaders.basic->on_recompilation([&] {
        shaders.basic->setup_projection_matrix(ASPECT_RATIO, context);
    });

    shaders.skybox = new Shader("skybox.glsl");
	shaders.skybox->add_texture("skybox", context);
    shaders.skybox->on_recompilation([&] {
        shaders.skybox->setup_projection_matrix(ASPECT_RATIO, context);
    });

    shaders.shadow_map = new Shader("shadow_map.glsl");

    shaders.shadow_map_render = new Shader("shadow_map_render.glsl");
	shaders.shadow_map_render->add_texture("texture_shadow_map", context);

    const f32 FIELD_OF_VIEW = 60.0f;
    const f32 MOVE_SPEED = 0.33f;
    const f32 ROTATION_SPEED = 0.050f;
	const Vec3f CAMERA_POSITION(0, 5, 8);
	const Vec3f CAMERA_FRONT(0, 0, -1);
	const Vec3f UP_WORLD(0.0f, 1.0f, 0.0f);
    Camera camera(CAMERA_POSITION, CAMERA_FRONT, UP_WORLD,
                  FIELD_OF_VIEW, ASPECT_RATIO, MOVE_SPEED, ROTATION_SPEED);

    const u32 box_texture_diffuse = load_texture("155.JPG", TextureFormat_SRGB, PixelFormat_RGB);
    const u32 box_texture_normal = load_texture("155_norm.JPG", TextureFormat_RGB, PixelFormat_RGB);

    const u32 floor_texture_diffuse = load_texture("177.JPG", TextureFormat_SRGB, PixelFormat_RGB);
    const u32 floor_texture_normal = load_texture("177_norm.JPG", TextureFormat_RGB, PixelFormat_RGB);

	// Wall textures
    const u32 wall_texture_diffuse = load_texture("brickwall.jpg", TextureFormat_SRGB, PixelFormat_RGB);
    const u32 wall_texture_normal = load_texture("brickwall_normal.jpg", TextureFormat_RGB, PixelFormat_RGB);

	const char *skybox_faces[] = {
		"right.jpg", // pos x
		"left.jpg", // neg x
		"top.jpg", // pos y
		"bottom.jpg", // neg y
		"back.jpg", // pos z
		"front.jpg", // neg z
	};

	const u32 skybox = load_cubemap_texture(skybox_faces, LT_Count(skybox_faces),
											TextureFormat_RGB, PixelFormat_RGB);

	const i32 shadow_map_width = 1024, shadow_map_height = 1024;
	ShadowMap shadow_map = create_shadow_map(shadow_map_width, shadow_map_height, *shaders.shadow_map);
	Mesh *shadow_map_surface = resources.load_shadow_map_render_surface(shadow_map.texture);

    const u32 pallet_texture_diffuse = load_texture("pallet/diffus.tga", TextureFormat_SRGB, PixelFormat_RGB);
    const u32 pallet_texture_specular = load_texture("pallet/specular.tga", TextureFormat_SRGB, PixelFormat_RGB);
    const u32 pallet_texture_normal = load_texture("pallet/normal.tga", TextureFormat_RGB, PixelFormat_RGB);

	// ----------------------------------------------------------
	// Entities
	// ----------------------------------------------------------
	Entities entities = {};

	// Model
	Mat4f pallet_transform;
	pallet_transform = lt::translation(pallet_transform, Vec3f(10, 1, 0));
	pallet_transform = lt::scale(pallet_transform, Vec3f(0.02));
	create_entity_from_model(entities, resources, "pallet/pallet.obj", shaders.basic,
							 pallet_transform, 32.0f, pallet_texture_diffuse,
							 pallet_texture_specular, pallet_texture_normal);
	
	// cubes
	const Vec3f positions[] = {
		Vec3f(0.0f, 2.0f, 0.0f),
		Vec3f(4.0f, 3.0f, 0.0f),
		Vec3f(1.0f, 5.0f, 2.0f),
		Vec3f(-5.0f, 2.0f, -1.0f),
		Vec3f(-3.0f, 5.1f, -7.0f),
	};
	const Vec3f scales[] = {
		Vec3f(1),
		Vec3f(1),
		Vec3f(1),
		Vec3f(1),
		Vec3f(2),
	};
	for (usize i = 0; i < LT_Count(positions); i++)
	{
		if (i == 2) break;
		Mat4f transform;
		transform = lt::translation(transform, positions[i]);
		transform = lt::scale(transform, scales[i]);
		create_textured_cube(entities, resources, shaders.basic, transform, 128,
							 box_texture_diffuse, box_texture_diffuse, box_texture_normal);
	}
	//
	// Light
	//
	DirectionalLight dir_light;
	dir_light.direction = Vec3f(0.51f, -0.44f, 0.74f);
	dir_light.ambient = Vec3f(.2f);
	dir_light.diffuse = Vec3f(1);
	dir_light.specular = Vec3f(1);
	const Vec3f dir_light_pos(-55.01f, 57.25f, -101.6f);

	const Mat4f light_view = lt::look_at(dir_light_pos, dir_light_pos+dir_light.direction, Vec3f(0, 1, 0));

	context.use_shader(*shaders.basic);
	shaders.basic->set3f("dir_light.direction", dir_light.direction);
	shaders.basic->set3f("dir_light.ambient", dir_light.ambient);
	shaders.basic->set3f("dir_light.diffuse", dir_light.diffuse);
	shaders.basic->set3f("dir_light.specular", dir_light.specular);

	{
		// Set the static light space uniform for the shadow map shader
		const Mat4f light_projection = lt::orthographic(-50, 50, -50, 50, 1, 1000);
		const Mat4f light_space = light_projection * light_view;
		context.use_shader(*shaders.shadow_map);
		shaders.shadow_map->set_matrix("light_space", light_space);
		context.use_shader(*shaders.basic);
		shaders.basic->set_matrix("light_space", light_space);
	}
	{
		LightEmmiter le = {};
		le.position = Vec3f(3.0f, 5.0f, 0.0f);
		le.ambient = Vec3f(0.01f);
		le.diffuse = Vec3f(0.5f);
		le.specular = Vec3f(1.0f);
		le.constant = 1.0f;
		le.linear = 0.35;
		le.quadratic = 0.44f;
		le.shader = shaders.basic;

		Mat4f transform;
		transform = lt::translation(transform, Vec3f(3, 5, 0));
		transform = lt::scale(transform, Vec3f(0.1f));

		create_point_light(entities, resources, shaders.light, transform, le, 0, 0);
	}
	// Wall on left
	{
		Mat4f transform(1);
		transform = lt::translation(transform, Vec3f(-18, 18, 0));
		transform = lt::scale(transform, Vec3f(.5f, 18, 28));
		create_textured_cube(entities, resources, shaders.basic, transform, 128,
							 wall_texture_diffuse, wall_texture_diffuse, wall_texture_normal);
	}
	// Wall on right
	{
		Mat4f transform(1);
		transform = lt::translation(transform, Vec3f(37, 18, 0));
		transform = lt::scale(transform, Vec3f(.5f, 18, 28));
		create_textured_cube(entities, resources, shaders.basic, transform, 128,
							 wall_texture_diffuse, wall_texture_diffuse, wall_texture_normal);
	}
	// Wall on top
	{
		Mat4f transform(1);
		transform = lt::translation(transform, Vec3f(9.5f, 36.5f, 0));
		transform = lt::scale(transform, Vec3f(28, .5f, 28));
		create_textured_cube(entities, resources, shaders.basic, transform, 128,
							 wall_texture_diffuse, wall_texture_diffuse, wall_texture_normal);
	}
	// Wall on the back
	{
		Mat4f transform(1);
		transform = lt::translation(transform, Vec3f(9.5f, 18, 27.5f));
		transform = lt::scale(transform, Vec3f(27, 18, .5f));
		create_textured_cube(entities, resources, shaders.basic, transform, 128,
							 wall_texture_diffuse, wall_texture_diffuse, wall_texture_normal);
	}
	// Wall on the front
	{
		Mat4f transform(1);
		transform = lt::translation(transform, Vec3f(30.0f, 18, -28.5f));
		transform = lt::scale(transform, Vec3f(27, 18, .5f));
		create_textured_cube(entities, resources, shaders.basic, transform, 128,
							 wall_texture_diffuse, wall_texture_diffuse, wall_texture_normal);
	}
	// FLOOR
	{
		Mat4f transform(1);
		transform = lt::translation(transform, Vec3f(9.5f, 0, 0));
		transform = lt::rotation_x(transform, -90);
		transform = lt::scale(transform, Vec3f(28, 28, 1));

		create_plane(entities, resources, shaders.basic, transform, 32, 10.0f,
					 floor_texture_diffuse, floor_texture_diffuse, floor_texture_normal);
	}
	// Skybox
	Mesh *skybox_mesh = resources.load_cubemap(skybox);

	// Set projection matrices.
    shaders.light->setup_projection_matrix(ASPECT_RATIO, context);
    shaders.basic->setup_projection_matrix(ASPECT_RATIO, context);
    shaders.skybox->setup_projection_matrix(ASPECT_RATIO, context);
    shaders.selection->setup_projection_matrix(ASPECT_RATIO, context);

	// Initialize the DEBUG GUI
	dgui::init(app.window);

    // Define variables to control time
    f64 current_time = get_time_milliseconds();
	f64 accumulator = 0;
	f64 second_counter = get_time_milliseconds();

	u32 frame_count = 0;
	u32 update_count = 0;
	f64 avg_frame_time = 0;

	// Fixed clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    bool running = true;
    while (running)
    {
        // Update frame information.
        f64 new_time = get_time_milliseconds();
		f64 frame_time = new_time - current_time;
		if (frame_time > 80)
			logger.log("Frame time took too long: ", frame_time, "ms");
        current_time = new_time;

		accumulator += frame_time;

        // Process input and watcher events.
        process_input(app.window, g_keyboard);
#ifdef DEV_ENV
        process_watcher_events(*shaders.basic, *shaders.light);
#endif

        // Check if the window should close.
        if (glfwWindowShouldClose(app.window))
        {
            running = false;
#ifdef DEV_ENV
            watcher_stop();
#endif
            continue;
        }

		if (dgui::State::instance().enable_multisampling)
			context.enable_multisampling();
		else
			context.disable_multisampling();

		const f64 dt = 1000 / 30.0f;
        while (accumulator >= dt)
        {
            game_update(g_keyboard, camera, dgui::State::instance());
			update_count++;
            accumulator -= dt;
		}

		const f64 lag_offset = accumulator / dt;

		game_render(lag_offset, app, camera, entities, shaders, shadow_map, light_view, dir_light_pos,
					shadow_map_surface, skybox_mesh, context);

        glfwPollEvents();

		frame_count++;
		avg_frame_time += frame_time;
		if (get_time_milliseconds() - second_counter >= 1000)
		{
			avg_frame_time /= frame_count;
			dgui::State::instance().frame_time = avg_frame_time;
			dgui::State::instance().fps = frame_count;
			dgui::State::instance().ups = update_count;
			frame_count = 0;
			update_count = 0;
			avg_frame_time = 0;
			second_counter = get_time_milliseconds();
		}
    }

#ifdef DEV_ENV
    pthread_join(watcher_thread, nullptr);
#endif
    glfwDestroyWindow(app.window);
    glfwTerminate();
}
