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
#include "entities.hpp"

lt_global_variable lt::Logger logger("main");
lt_global_variable bool g_display_debug_gui = true;
lt_global_variable DebugGuiState g_debug_gui_state;

struct Key
{
	enum Transition
	{
		Transition_None = 0,
		Transition_Up = 1,
		Transition_Down = 2,
	};

	bool is_pressed;
	Transition last_transition;
};

const i32 NUM_KEYBOARD_KEYS = 1024;

lt_global_variable Key g_keyboard[NUM_KEYBOARD_KEYS] = {};

lt_internal void
framebuffer_size_callback(GLFWwindow *w, i32 width, i32 height)
{
    LT_Unused(w);
    glViewport(0, 0, width, height);
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

lt_internal GLFWwindow *
create_window_and_set_context(const char *title, i32 width, i32 height)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);
	glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if (!window)
    {
        glfwTerminate();
        LT_Fail("Failed to create glfw window.\n");
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        LT_Fail("Failed to initialize GLAD\n");

    glEnable(GL_CULL_FACE);
    // glFrontFace(GL_CCW);
    // glCullFace(GL_FRONT);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    return window;
}

void
game_update(Key *kb, Camera& camera, f64 delta, DebugGuiState *state)
{
    // Move
    if (kb[GLFW_KEY_A].is_pressed)
        camera.move(Camera::Direction::Left, delta);

    if (kb[GLFW_KEY_D].is_pressed)
        camera.move(Camera::Direction::Right, delta);

    if (kb[GLFW_KEY_W].is_pressed)
        camera.move(Camera::Direction::Forwards, delta);

    if (kb[GLFW_KEY_S].is_pressed)
        camera.move(Camera::Direction::Backwards, delta);

    // Rotation
    if (kb[GLFW_KEY_RIGHT].is_pressed)
        camera.rotate(Camera::RotationAxis::Up, -delta);

    if (kb[GLFW_KEY_LEFT].is_pressed)
        camera.rotate(Camera::RotationAxis::Up, delta);

    if (kb[GLFW_KEY_UP].is_pressed)
        camera.rotate(Camera::RotationAxis::Right, delta);

    if (kb[GLFW_KEY_DOWN].is_pressed)
        camera.rotate(Camera::RotationAxis::Right, -delta);
	
	state->camera_pos = camera.frustum.position;
	state->camera_front = camera.frustum.front.v;
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
	// logger.log("Trying to load ", fullpath);
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

int
main(void)
{
    /* ==================================================================================
     *     OpenGL and GLFW initialization
     * ================================================================================== */
    const i32 WINDOW_WIDTH = 1024;
    const i32 WINDOW_HEIGHT = 768;
    const f32 ASPECT_RATIO = (f32)WINDOW_WIDTH / WINDOW_HEIGHT;

    logger.log("Initializing glfw");
    glfwInit();

    GLFWwindow *window = create_window_and_set_context("CG playground", WINDOW_WIDTH, WINDOW_HEIGHT);

#ifdef DEV_ENV
    pthread_t watcher_thread;
    logger.log("Creating watcher thread");
    pthread_create(&watcher_thread, nullptr, watcher_start, nullptr);
#endif

    GLContext context;
	Resources resources = {};

	//
	// Load shaders
	//
    Shader light_shader("light.glsl");
    light_shader.on_recompilation([&] {
        light_shader.setup_projection_matrix(ASPECT_RATIO, context);
    });

    Shader basic_shader("basic.glsl");
	basic_shader.add_texture("material.texture_diffuse1");
	basic_shader.add_texture("material.texture_specular1");
	basic_shader.add_texture("material.texture_normal1");
	basic_shader.add_texture("texture_shadow_map");
    basic_shader.on_recompilation([&] {
        basic_shader.setup_projection_matrix(ASPECT_RATIO, context);
    });

    Shader skybox_shader("skybox.glsl");
	skybox_shader.add_texture("skybox");
    skybox_shader.on_recompilation([&] {
        skybox_shader.setup_projection_matrix(ASPECT_RATIO, context);
    });

    Shader shadow_map_shader("shadow_map.glsl");

    Shader shadow_map_render_shader("shadow_map_render.glsl");
	shadow_map_render_shader.add_texture("texture_shadow_map");

    const f32 FIELD_OF_VIEW = 60.0f;
    const f32 MOVE_SPEED = 0.05f;
    const f32 ROTATION_SPEED = 0.02f;
	// const Vec3f CAMERA_POSITION(0.0f, 5.0f, 10.0f);
	const Vec3f CAMERA_POSITION(-15.47f, 24.12f, -28.7f);
	// const Vec3f CAMERA_FRONT(0.0f, 0.0f, -1.0f);
	const Vec3f CAMERA_FRONT(0.4f, -0.57f, 0.72f);
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

	// const i32 shadow_map_width = 1024, shadow_map_height = 1024;
	const i32 shadow_map_width = WINDOW_WIDTH, shadow_map_height = WINDOW_HEIGHT;
	ShadowMap shadow_map = create_shadow_map(shadow_map_width, shadow_map_height, &shadow_map_shader);
	Mesh *shadow_map_surface = resources.load_shadow_map_render_surface(shadow_map.texture);

	// ----------------------------------------------------------
	// Entities
	// ----------------------------------------------------------
	Entities entities = {};

	// cubes
	const Vec3f positions[4] = {
		Vec3f(0.0f, 2.0f, 0.0f),
		Vec3f(4.0f, 3.0f, 0.0f),
		Vec3f(1.0f, 5.0f, 2.0f),
		Vec3f(-5.0f, 2.0f, -1.0f),
	};
	const Vec3f scales[4] = {
		Vec3f(1),
		Vec3f(1),
		Vec3f(1),
		Vec3f(1),
	};
	for (i32 i = 0; i < 4; i++)
	{
		Mat4f transform;
		transform = lt::translation(transform, positions[i]);
		transform = lt::scale(transform, scales[i]);
		create_textured_cube(&entities, &resources, &basic_shader, transform, 128,
							 nullptr, box_texture_diffuse, box_texture_diffuse, box_texture_normal);
	}
	//
	// Light
	//
	DirectionalLight dir_light;
	dir_light.direction = Vec3f(0.4f, -0.57f, 0.72f);
	dir_light.ambient = Vec3f(.2f);
	dir_light.diffuse = Vec3f(1);
	dir_light.specular = Vec3f(1);
	const Vec3f dir_light_pos(-15.47f, 24.12f, -28.7f);

	const Mat4f light_view = lt::look_at(dir_light_pos, dir_light_pos+dir_light.direction, Vec3f(0, 1, 0));

	context.use_shader(basic_shader);
	basic_shader.set3f("dir_light.direction", dir_light.direction);
	basic_shader.set3f("dir_light.ambient", dir_light.ambient);
	basic_shader.set3f("dir_light.diffuse", dir_light.diffuse);
	basic_shader.set3f("dir_light.specular", dir_light.specular);

	{
		// Set the static light space uniform for the shadow map shader
		const Mat4f light_projection = lt::orthographic(-30, 30, -30, 30, 1, 100);
		const Mat4f light_space = light_projection * light_view;
		context.use_shader(shadow_map_shader);
		shadow_map_shader.set_matrix("light_space", light_space);
		context.use_shader(basic_shader);
		basic_shader.set_matrix("light_space", light_space);
	}
	{
		LightEmmiter le = {};
		le.position = Vec3f(3.0f, 5.0f, 0.0f);
		le.ambient = Vec3f(0.1f);
		le.diffuse = Vec3f(0.7f);
		le.specular = Vec3f(1.0f);
		le.constant = 1.0f;
		le.linear = 0.35;
		le.quadratic = 0.44f;
		le.shader = &basic_shader;

		Mat4f transform;
		transform = lt::translation(transform, Vec3f(3, 5, 0));
		transform = lt::scale(transform, Vec3f(0.1f));

		create_point_light(&entities, &resources, &light_shader, transform, le, nullptr, 0, 0);
	}
	// WALL
	{
		Mat4f transform(1);
		transform = lt::translation(transform, Vec3f(-10, 0, 0));
		transform = lt::rotation_y(transform, 90.0f);
		transform = lt::scale(transform, Vec3f(8.0f));
		create_plane(&entities, &resources, &basic_shader, transform, 32, 5.0f,
					 nullptr, wall_texture_diffuse, wall_texture_diffuse, wall_texture_normal);
	}
	// FLOOR
	{
		Mat4f transform(1);
		transform = lt::rotation_x(transform, -90);
		transform = lt::scale(transform, Vec3f(20.0f));

		create_plane(&entities, &resources, &basic_shader, transform, 32, 10.0f,
					 nullptr, floor_texture_diffuse, floor_texture_diffuse, floor_texture_normal);
	}
	// Skybox
	const Mesh *skybox_mesh = resources.load_cubemap(skybox);

    light_shader.setup_projection_matrix(ASPECT_RATIO, context);
    basic_shader.setup_projection_matrix(ASPECT_RATIO, context);
    skybox_shader.setup_projection_matrix(ASPECT_RATIO, context);

	// Initialize the DEBUG GUI
	debug_gui_init(window);

    // Define variables to control time
    const f64 DESIRED_FPS = 60.0;
    const f64 DESIRED_FRAMETIME = 1.0 / DESIRED_FPS;
    const u32 MAX_STEPS = 6;
    const f64 MAX_DELTA_TIME = 1.0;

    f64 new_time, total_delta, delta;
    f64 previous_time = glfwGetTime();

	// Fixed clear color
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    bool running = true;
    while (running)
    {
        // Update frame information.
        new_time = glfwGetTime();
        g_debug_gui_state.frame_time = new_time - previous_time;
        previous_time = new_time;
        g_debug_gui_state.fps = 1 / g_debug_gui_state.frame_time; // used for logging

        // Process input and watcher events.
        process_input(window, g_keyboard);
#ifdef DEV_ENV
        process_watcher_events(basic_shader, light_shader);
#endif

        // Check if the window should close.
        if (glfwWindowShouldClose(window))
        {
            running = false;
#ifdef DEV_ENV
            watcher_stop();
#endif
            continue;
        }

		if (g_debug_gui_state.enable_multisampling)
			context.enable_multisampling();
		else
			context.disable_multisampling();

        total_delta = g_debug_gui_state.frame_time / DESIRED_FRAMETIME;
        u32 loops = 0;
        while (total_delta > 0.0 && loops < MAX_STEPS)
        {
            delta = std::min(total_delta, MAX_DELTA_TIME);

            game_update(g_keyboard, camera, delta, &g_debug_gui_state);

            total_delta -= delta;
            loops++;
        }

		// Render first to depth map
		glViewport(0, 0, shadow_map_width, shadow_map_height);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_map.fbo); // TODO: move to GLContext
		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		draw_entities_for_shadow_map(entities, light_view, dir_light_pos, shadow_map, context);

		// Actual rendering
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);

		if (g_debug_gui_state.draw_shadow_map)
		{
			// Draws the shadowmap instead of the scene
			draw_shadow_map(shadow_map_surface, shadow_map_render_shader, context);
		}
		else
		{
			context.use_shader(basic_shader);
			draw_entities(entities, camera, context, shadow_map);
			draw_skybox(skybox_mesh, skybox_shader, camera.view_matrix(), context);
		}

		if (g_display_debug_gui)
		{
			debug_gui_draw(window, &g_debug_gui_state);
		}

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

#ifdef DEV_ENV
    pthread_join(watcher_thread, nullptr);
#endif
    glfwDestroyWindow(window);
    glfwTerminate();
}
