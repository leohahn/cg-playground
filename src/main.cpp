#include <stdio.h>

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

#include "stb_image.h"

static lt::Logger logger("main");

static void
framebuffer_size_callback(GLFWwindow *w, i32 width, i32 height)
{
    LT_Unused(w);
    glViewport(0, 0, width, height);
}

static void
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

static void
process_watcher_events()
{
    WatcherEvent *ev;
    while ((ev = watcher_peek_event()) != nullptr)
    {
        const bool is_dir = ev->inotify_mask & IN_ISDIR;
        const bool is_modified = ev->inotify_mask & IN_MODIFY;

        if (!is_dir && is_modified)
        {
            logger.log("File ", ev->name, " changed, requesting shader recompile.");
            shader_recompile(ShaderKind_Basic);
        }

        // Notify the watcher that the event was consumed.
        watcher_event_peeked();
    }
}

static GLFWwindow *
create_window_and_set_context(const char *title, i32 width, i32 height)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if (window == nullptr)
    {
        glfwTerminate();
        LT_Fail("Failed to create glfw window.\n");
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return window;
}

/*
 *  TODO:
 *    Implement some form of rigit body simulation.
 *    Reference: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch29.html
 *
 *    @Important: LEARN QUATERNIONS!!!!
 *      https://www.youtube.com/watch?v=fKIss4EV6ME
 *      Double-cover: https://mollyrocket.com/837
 *
 *    [1] Make the camera work freely on 3 dimensions with quaternions.
 *        - Test the quaternion implementation on the camera object!!!!
 *    [2] Create code to load 3d models and render them with textures.
 *    [3] TODO
 */

int
main(void)
{
    static bool keyboard[1024] = {};
    /* ==================================================================================
     *     OpenGL and GLFW initialization
     * ================================================================================== */
    const i32 WINDOW_WIDTH = 1024;
    const i32 WINDOW_HEIGHT = 768;
    const f32 ASPECT_RATIO = (f32)WINDOW_WIDTH / WINDOW_HEIGHT;

    logger.log("Initializing glfw");
    glfwInit();

    GLFWwindow *window = create_window_and_set_context("Hot Shader Loader", WINDOW_WIDTH, WINDOW_HEIGHT);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        LT_Fail("Failed to initialize GLAD\n");

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

#ifdef DEV_ENV
    pthread_t watcher_thread;
    logger.log("Creating watcher thread");
    pthread_create(&watcher_thread, nullptr, watcher_start, nullptr);
#endif

    shader_initialize();

    const f32 FIELD_OF_VIEW = 60.0f;
    const f32 MOVE_SPEED = 0.05f;
    const f32 ROTATION_SPEED = 0.02f;
    Camera camera(Vec3f(0.0f, 0.0f, -20.0f), Vec3f(0.0f, 0.0f, 1.0f), Vec3f(0.0f, 1.0f, 0.0f),
                  FIELD_OF_VIEW, ASPECT_RATIO, MOVE_SPEED, ROTATION_SPEED);

    GLfloat vertices[] = {
        // vertices       texture coords
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        2.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        2.0f, 2.0f, 0.0f, 1.0f, 1.0f,

        2.0f, 2.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 2.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    };

    GLuint vao, vbo;
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
    }
    GLuint texture;
    {
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
        uchar *image_data = stbi_load("/home/lhahn/dev/cpp/quadrado/resources/wooden-create.png",
                                   &width, &height, &num_channels, 0);
        // uchar *image_data = stbi_load("/home/lhahn/dev/cpp/quadrado/resources/brick_wall.jpg",
        //                            &width, &height, &num_channels, 0);
        if (image_data)
        {
            logger.log("Texture image loaded successfuly.");
            logger.log("width = ", width, " and height = ", height, " (num channels = ", num_channels, ")");
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            logger.error("Failed loading texture");
        }

        // Free image
        stbi_image_free(image_data);
    }

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
        process_watcher_events();

        // Check if the window should close.
        if (glfwWindowShouldClose(window))
        {
            running = false;
#ifdef DEV_ENV
            watcher_stop();
#endif
            continue;
        }

        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        total_delta = frame_time / DESIRED_FRAMETIME;
        loops = 0;
        while (total_delta > 0.0 && loops < MAX_STEPS)
        {
            delta = std::min(total_delta, MAX_DELTA_TIME);
            // TODO: Eventually call an update function.

            if (keyboard[GLFW_KEY_A] == GLFW_PRESS)
                camera.move(Camera::Direction::Left, delta);

            if (keyboard[GLFW_KEY_D] == GLFW_PRESS)
                camera.move(Camera::Direction::Right, delta);

            if (keyboard[GLFW_KEY_W] == GLFW_PRESS)
                camera.move(Camera::Direction::Forwards, delta);

            if (keyboard[GLFW_KEY_S] == GLFW_PRESS)
                camera.move(Camera::Direction::Backwards, delta);

            //
            // Rotation
            //

            if (keyboard[GLFW_KEY_RIGHT] == GLFW_PRESS)
                camera.rotate(Camera::RotationAxis::Up, -delta);

            if (keyboard[GLFW_KEY_LEFT] == GLFW_PRESS)
                camera.rotate(Camera::RotationAxis::Up, delta);

            if (keyboard[GLFW_KEY_UP] == GLFW_PRESS)
                camera.rotate(Camera::RotationAxis::Right, delta);

            if (keyboard[GLFW_KEY_DOWN] == GLFW_PRESS)
                camera.rotate(Camera::RotationAxis::Right, -delta);

            total_delta -= delta;
            loops++;
        }

        // TODO: Eventually call a render function here.
#if 1
        shader_set(ShaderKind_Basic);

        Mat4f model(1.0f);
        GLuint model_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic), "model");

        // Mat4f view = lt::look_at(Vec3f(0.0f, 0.0f, -5.0f), Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f));
        Mat4f view = camera.view_matrix();
        GLuint view_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic), "view");

        Mat4f projection = lt::perspective<f32>(60.0f, ASPECT_RATIO, 0.1f, 100.0f);
        GLuint projection_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic), "projection");

        glUniformMatrix4fv(model_loc, 1, GL_FALSE, model.data());
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, view.data());
        glUniformMatrix4fv(projection_loc, 1, GL_FALSE, projection.data());

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
#else
        draw_tile_map(tile_map);
#endif

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

#ifdef DEV_ENV
    pthread_join(watcher_thread, nullptr);
#endif

    glfwDestroyWindow(window);
    glfwTerminate();
}
