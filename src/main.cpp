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

#include "stb_image.h"

lt_internal lt::Logger logger("main");

lt_internal void
framebuffer_size_callback(GLFWwindow *w, i32 width, i32 height)
{
    LT_Unused(w);
    glViewport(0, 0, width, height);
}

lt_internal void
process_input(GLFWwindow *win, bool *keyboard)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
        glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(win, true);
    }

    keyboard[GLFW_KEY_W] = glfwGetKey(win, GLFW_KEY_W);
    keyboard[GLFW_KEY_S] = glfwGetKey(win, GLFW_KEY_S);
    keyboard[GLFW_KEY_A] = glfwGetKey(win, GLFW_KEY_A);
    keyboard[GLFW_KEY_D] = glfwGetKey(win, GLFW_KEY_D);
    keyboard[GLFW_KEY_UP] = glfwGetKey(win, GLFW_KEY_UP);
    keyboard[GLFW_KEY_DOWN] = glfwGetKey(win, GLFW_KEY_DOWN);
    keyboard[GLFW_KEY_LEFT] = glfwGetKey(win, GLFW_KEY_LEFT);
    keyboard[GLFW_KEY_RIGHT] = glfwGetKey(win, GLFW_KEY_RIGHT);
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
game_update(bool *keyboard, Camera& camera, f64 delta)
{
    // Move
    if (keyboard[GLFW_KEY_A] == GLFW_PRESS)
        camera.move(Camera::Direction::Left, delta);

    if (keyboard[GLFW_KEY_D] == GLFW_PRESS)
        camera.move(Camera::Direction::Right, delta);

    if (keyboard[GLFW_KEY_W] == GLFW_PRESS)
        camera.move(Camera::Direction::Forwards, delta);

    if (keyboard[GLFW_KEY_S] == GLFW_PRESS)
        camera.move(Camera::Direction::Backwards, delta);

    // Rotation
    if (keyboard[GLFW_KEY_RIGHT] == GLFW_PRESS)
        camera.rotate(Camera::RotationAxis::Up, -delta);

    if (keyboard[GLFW_KEY_LEFT] == GLFW_PRESS)
        camera.rotate(Camera::RotationAxis::Up, delta);

    if (keyboard[GLFW_KEY_UP] == GLFW_PRESS)
        camera.rotate(Camera::RotationAxis::Right, delta);

    if (keyboard[GLFW_KEY_DOWN] == GLFW_PRESS)
        camera.rotate(Camera::RotationAxis::Right, -delta);
}

struct TexturedCube
{
    Vec3f      position;
    Vec3f      scaling;
    f32        shininess;
    Mesh       mesh;

    explicit TexturedCube(Vec3f position, Vec3f scaling, f32 shininess, u32 diffuse_texture, u32 specular_texture)
        : position(position)
        , scaling(scaling)
        , shininess(shininess)
        , mesh(Mesh::static_unit_cube(diffuse_texture, specular_texture))
    {}
};

struct PointLight
{
    Vec3f     position;
    Vec3f     scaling;
    Vec3f     color;
    Mesh      mesh;

    explicit PointLight(Vec3f position, Vec3f scaling, Vec3f color, u32 diffuse_texture, u32 specular_texture)
        : position(position)
        , scaling(scaling)
        , color(color)
        , mesh(Mesh::static_unit_cube(diffuse_texture, specular_texture))
    {}
};

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

void
load_texture(const char *path, GLuint &texture, TextureFormat texture_format, PixelFormat pixel_format)
{
    std::string fullpath = std::string(RESOURCES_PATH) + std::string(path);
    // Textures
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
        logger.log("[", width, " x ", height, "] (num channels = ", num_channels, ")");

		const i32 mipmap_level = 0;
        glTexImage2D(GL_TEXTURE_2D, mipmap_level, texture_format, width, height,
					 0, pixel_format, GL_UNSIGNED_BYTE, image_data);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        logger.error("Failed loading texture");
    }
    // Free image
    stbi_image_free(image_data);
}

int
main(void)
{
    lt_local_persist bool keyboard[1024] = {};
    /* ==================================================================================
     *     OpenGL and GLFW initialization
     * ================================================================================== */
    const i32 WINDOW_WIDTH = 1024;
    const i32 WINDOW_HEIGHT = 768;
    const f32 ASPECT_RATIO = (f32)WINDOW_WIDTH / WINDOW_HEIGHT;

    logger.log("Initializing glfw");
    glfwInit();

    GLFWwindow *window = create_window_and_set_context("Hot Shader Loader", WINDOW_WIDTH, WINDOW_HEIGHT);

#ifdef DEV_ENV
    pthread_t watcher_thread;
    logger.log("Creating watcher thread");
    pthread_create(&watcher_thread, nullptr, watcher_start, nullptr);
#endif

    //GLResources::instance().initialize();

    GLContext context;

    Shader light_shader("light.glsl");
    light_shader.on_recompilation([&] {
        light_shader.setup_projection_matrix(ASPECT_RATIO, context);
    });

    Shader basic_shader("basic.glsl");
    basic_shader.on_recompilation([&] {
        basic_shader.setup_projection_matrix(ASPECT_RATIO, context);
    });

    const f32 FIELD_OF_VIEW = 60.0f;
    const f32 MOVE_SPEED = 0.05f;
    const f32 ROTATION_SPEED = 0.02f;
    Camera camera(Vec3f(0.0f, 5.0f, 10.0f), Vec3f(0.0f, 0.0f, -1.0f), Vec3f(0.0f, 1.0f, 0.0f),
                  FIELD_OF_VIEW, ASPECT_RATIO, MOVE_SPEED, ROTATION_SPEED);

    GLuint box_texture_diffuse, box_texture_specular, floor_texture_diffuse;
    load_texture("wooden-container.png", box_texture_diffuse, TextureFormat_SRGBA, PixelFormat_RGBA);
    load_texture("wooden-container-specular.png", box_texture_specular, TextureFormat_SRGBA, PixelFormat_RGBA);
    // load_texture("Brick/Brick1/1024/Brick-Diffuse.tga", floor_texture_diffuse, true);
    load_texture("tile.jpg", floor_texture_diffuse, TextureFormat_SRGB, PixelFormat_RGB);

    TexturedCube cubes[4] = {
        TexturedCube(Vec3f(0.0f, 1.0f, 0.0f), Vec3f(1), 128, box_texture_diffuse, box_texture_specular),
        TexturedCube(Vec3f(4.0f, 3.0f, 0.0f), Vec3f(1), 128, box_texture_diffuse, box_texture_specular),
        TexturedCube(Vec3f(1.0f, 5.0f, 2.0f), Vec3f(1), 128, box_texture_diffuse, box_texture_specular),
        TexturedCube(Vec3f(-5.0f, 2.0f, -1.0f), Vec3f(1), 128, box_texture_diffuse, box_texture_specular),
    };

    PointLight lights[4] = {
        PointLight(Vec3f(3.0f, 5.0f, 0.0f), Vec3f(0.1f, 0.1f, 0.1f), Vec3f(1.0f, 1.0f, 1.0f), 0, 0),
        PointLight(Vec3f(7.0f, 2.0f, 0.0f), Vec3f(0.1f, 0.1f, 0.1f), Vec3f(1.0f, 1.0f, 1.0f), 0, 0),
        PointLight(Vec3f(0.0f, 3.0f, 0.0f), Vec3f(0.1f, 0.1f, 0.1f), Vec3f(1.0f, 1.0f, 1.0f), 0, 0),
        PointLight(Vec3f(3.0f, 4.0f, 2.0f), Vec3f(0.1f, 0.1f, 0.1f), Vec3f(1.0f, 1.0f, 1.0f), 0, 0),
    };

    Mesh floor_mesh = Mesh::static_unit_plane(floor_texture_diffuse, floor_texture_diffuse);

    light_shader.setup_projection_matrix(ASPECT_RATIO, context);
    basic_shader.setup_projection_matrix(ASPECT_RATIO, context);

    // Define variables to control time
    constexpr f64 DESIRED_FPS = 60.0;
    constexpr f64 DESIRED_FRAMETIME = 1.0 / DESIRED_FPS;
    constexpr u32 MAX_STEPS = 6;
    constexpr f64 MAX_DELTA_TIME = 1.0;

    f64 new_time, total_delta, delta, frame_time;
    f64 previous_time = glfwGetTime();
    f32 fps;
    u32 loops;

    bool running = true;
    while (running)
    {
        // Update frame information.
        new_time = glfwGetTime();
        frame_time = new_time - previous_time;
        previous_time = new_time;
        fps = 1 / frame_time; // used for logging

        // Process input and watcher events.
        process_input(window, keyboard);
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

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        total_delta = frame_time / DESIRED_FRAMETIME;
        loops = 0;
        while (total_delta > 0.0 && loops < MAX_STEPS)
        {
            delta = std::min(total_delta, MAX_DELTA_TIME);

            game_update(keyboard, camera, delta);

            total_delta -= delta;
            loops++;
        }

        // TODO: Eventually call a render function here.
        {
            context.use_shader(basic_shader);

            basic_shader.set_matrix("view", camera.view_matrix());
            basic_shader.set3f("view_position", camera.frustum.position);

            for (isize i = 0; i < 4; ++i)
            {
                std::string i_str = std::to_string(i);
                basic_shader.set3f(("point_lights["+i_str+"].position").c_str(), lights[i].position);
                basic_shader.set3f(("point_lights["+i_str+"].ambient").c_str(), Vec3f(0.10f));
                basic_shader.set3f(("point_lights["+i_str+"].diffuse").c_str(), Vec3f(0.7f));
                basic_shader.set3f(("point_lights["+i_str+"].specular").c_str(), Vec3f(1.0f));
                basic_shader.set1f(("point_lights["+i_str+"].constant").c_str(), 1.0f);
                basic_shader.set1f(("point_lights["+i_str+"].linear").c_str(), 0.35f);
                basic_shader.set1f(("point_lights["+i_str+"].quadratic").c_str(), 0.44f);
            }
            for (isize i = 0; i < 4; ++i)
            {
                Mat4f box_model(1.0f);
                box_model = lt::scale(box_model, cubes[i].scaling);
                box_model = lt::translation(box_model, cubes[i].position);

                basic_shader.set_matrix("model", box_model);
                basic_shader.set1f("material.shininess", cubes[i].shininess);

                draw_mesh(cubes[i].mesh, basic_shader, context);
            }

            // FLOOR
            Mat4f floor_model(1);
            floor_model = lt::scale(floor_model, Vec3f(20.0f));
            basic_shader.set_matrix("model", floor_model);

            for (isize i = 0; i < 4; ++i)
                basic_shader.set3f(("point_lights[" + std::to_string(i) + "].quadratic").c_str(),
                                   Vec3f(0.5f, 0.5f, 0.5f));

            basic_shader.set1f("material.shininess", 32);

            draw_mesh(floor_mesh, basic_shader, context);
        }
        {
            context.use_shader(light_shader);

            for (isize i = 0; i < 4; ++i)
            {
                Mat4f model = lt::translation(Mat4f(1.0f), lights[i].position);
                model = lt::scale(model, lights[i].scaling);

                light_shader.set_matrix("model", model);
                light_shader.set_matrix("view", camera.view_matrix());

                draw_mesh(lights[i].mesh, light_shader, context);
            }
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
