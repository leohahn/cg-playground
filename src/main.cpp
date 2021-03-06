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
#include "application.hpp"
#include "macros.hpp"

//
// TODOs
//
// - Add antialiasing to the offscreen HDR buffer.
//   - Not so easy to do
//

lt_internal inline f64
get_time_milliseconds()
{
	return glfwGetTime() * 1000.0;
}

struct Counter
{
    f64 millis;
	u32 frames;
	u32 updates;

	inline void start() {millis = get_time_milliseconds();}
	inline bool second_passed() {return (get_time_milliseconds() - millis) >= 1000.0;}
	inline void restart()
	{
		start();
		frames = 0;
		updates = 0;
	}
};

lt_global_variable lt::Logger logger("main");
lt_global_variable bool g_display_debug_gui = true;
lt_global_variable Key g_keyboard[NUM_KEYBOARD_KEYS] = {};
lt_global_variable Counter g_counter = {};

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

lt_internal void
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
	Shader *bloom;

	~Shaders()
	{
		delete light;
		delete selection;
		delete hdr_texture_to_quad;
		delete basic;
		delete skybox;
		delete shadow_map;
		delete shadow_map_render;
		delete bloom;
	}
};

lt_internal void
game_render(f64 lag_offset, const Application &app, Camera &camera, Entities &entities,
			Shaders &shaders, ShadowMap &shadow_map, const Mat4f &light_view, Vec3f dir_light_pos,
			Mesh *shadow_map_surface, Mesh *skybox_mesh, GLContext &context)
{
	LT_Assert(lag_offset < 1);
	LT_Assert(lag_offset >= 0);

	auto &state = dgui::State::instance();

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

	if (state.draw_shadow_map)
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

		BEGIN_REGION(PerformanceRegion_DrawEntities);
		draw_entities(lag_offset, entities, camera, context, shadow_map, dgui::State::instance().selected_entity_handle);
		END_REGION(PerformanceRegion_DrawEntities);

		if (state.selected_entity_handle != -1)
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

	context.use_shader(*shaders.hdr_texture_to_quad);
	shaders.hdr_texture_to_quad->set1i("enable_tone_mapping", state.enable_tone_mapping);
	shaders.hdr_texture_to_quad->set1i("enable_gamma_correction", state.enable_gamma_correction);
	shaders.hdr_texture_to_quad->set1f("exposure", state.exposure);

	// draw_unit_quad(app.render_quad, *shaders.hdr_texture_to_quad, context);
	draw_unit_quad_and_apply_bloom(app, *shaders.hdr_texture_to_quad, *shaders.bloom, context);

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

    Application app = application_create_and_set_context(resources, "CG playground", WINDOW_WIDTH, WINDOW_HEIGHT);

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
	shaders.hdr_texture_to_quad->add_texture("texture_scene", context);
	shaders.hdr_texture_to_quad->add_texture("texture_bloom", context);

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

    shaders.bloom = new Shader("bloom.glsl");
	shaders.bloom->add_texture("texture_image", context);

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
		le.diffuse = Vec3f(3.0f);
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

	g_counter.start();
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

		BEGIN_REGION(PerformanceRegion_UpdateLoop);
        while (accumulator >= dt)
        {
            game_update(g_keyboard, camera, dgui::State::instance());
			g_counter.updates++;
            accumulator -= dt;
		}
		END_REGION(PerformanceRegion_UpdateLoop);

		const f64 lag_offset = accumulator / dt;

		BEGIN_REGION(PerformanceRegion_RenderLoop);
		game_render(lag_offset, app, camera, entities, shaders, shadow_map, light_view, dir_light_pos,
					shadow_map_surface, skybox_mesh, context);
		END_REGION(PerformanceRegion_RenderLoop);

        glfwPollEvents();

		g_counter.frames++;
		avg_frame_time += frame_time;
		if (g_counter.second_passed())
		{
			avg_frame_time /= g_counter.frames;
			dgui::State::instance().frame_time = avg_frame_time;
			dgui::State::instance().fps = g_counter.frames;
			dgui::State::instance().ups = g_counter.updates;
			avg_frame_time = 0;
			g_counter.restart();
		}
    }

#ifdef DEV_ENV
    pthread_join(watcher_thread, nullptr);
#endif
    glfwDestroyWindow(app.window);
    glfwTerminate();
}
