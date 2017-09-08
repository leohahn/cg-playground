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

lt_internal GLFWwindow *
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
setup_projection_matrices(f32 aspect_ratio)
{
    const Mat4f projection = lt::perspective<f32>(60.0f, aspect_ratio, 0.1f, 100.0f);
    const GLuint projection_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic), "projection");
    shader_set(ShaderKind_Basic);
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, projection.data());
    shader_set(ShaderKind_Light);
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, projection.data());
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

struct PointLight
{
    Vec3f position;
    Vec3f scaling;
    Vec3f color;
    u32   vao;

    PointLight(Vec3f position, Vec3f scaling, Vec3f color)
        : position(position)
        , scaling(scaling)
        , color(color)
        , vao(gl_resources_create_vao())
    {
        gl_resources_attrs_vertices(vao, BufferType::UnitCube);
    }
};

/*
 *  TODO:
 *    Learn:
 *      - Advanced lighting
 *      - Shadow maps
 *
 *    [1] Implement basic lighting
 *    [2] Implement shadow maps
 *    [3] Create code to load 3d models and render them with textures.
 *    [3] TODO
 */

void
load_texture(const char *path, GLuint &texture)
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
    uchar *image_data = stbi_load(fullpath.c_str(), &width, &height, &num_channels, 0);
    if (image_data)
    {
        logger.log("Texture ", fullpath, " loaded successfuly.");
        logger.log("[", width, " x ", height, "] (num channels = ", num_channels, ")");
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

    shader_initialize();
    shader_on_recompilation([ASPECT_RATIO](ShaderKind kind)
    {
        LT_Unused(kind);
        setup_projection_matrices(ASPECT_RATIO);
    });

    gl_resources_initialize();

    const f32 FIELD_OF_VIEW = 60.0f;
    const f32 MOVE_SPEED = 0.05f;
    const f32 ROTATION_SPEED = 0.02f;
    Camera camera(Vec3f(0.0f, 5.0f, 10.0f), Vec3f(0.0f, 0.0f, -1.0f), Vec3f(0.0f, 1.0f, 0.0f),
                  FIELD_OF_VIEW, ASPECT_RATIO, MOVE_SPEED, ROTATION_SPEED);

    u32 cube_vao = gl_resources_create_vao();
    gl_resources_attrs_vertice_normal_texture(cube_vao, BufferType::UnitCube);

    u32 floor_vao = gl_resources_create_vao();
    gl_resources_attrs_vertice_normal_texture(floor_vao, BufferType::UnitPlane);

    PointLight lights[4] = {
        PointLight(Vec3f(3.0f, 5.0f, 0.0f), Vec3f(0.1f, 0.1f, 0.1f), Vec3f(1.0f, 1.0f, 1.0f)),
        PointLight(Vec3f(7.0f, 2.0f, 0.0f), Vec3f(0.1f, 0.1f, 0.1f), Vec3f(1.0f, 1.0f, 1.0f)),
        PointLight(Vec3f(0.0f, 3.0f, 0.0f), Vec3f(0.1f, 0.1f, 0.1f), Vec3f(1.0f, 1.0f, 1.0f)),
        PointLight(Vec3f(3.0f, 4.0f, 2.0f), Vec3f(0.1f, 0.1f, 0.1f), Vec3f(1.0f, 1.0f, 1.0f)),
    };

    GLuint box_texture_diffuse, box_texture_specular, floor_texture_diffuse;
    load_texture("wooden-container.png", box_texture_diffuse);
    load_texture("wooden-container-specular.png", box_texture_specular);
    load_texture("stone.png", floor_texture_diffuse);

    setup_projection_matrices(ASPECT_RATIO);

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

        const Mat4f view = camera.view_matrix();
        // TODO: Eventually call a render function here.
        {
            shader_set(ShaderKind_Basic);

            Mat4f box_model(1.0f);
            box_model = lt::translation(box_model, Vec3f(0.0f, 1.0f, 0.0f));
            const GLuint model_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic), "model");
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, box_model.data());

            const GLuint view_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic), "view");
            glUniformMatrix4fv(view_loc, 1, GL_FALSE, view.data());

            glUniform1i(glGetUniformLocation(shader_get_program(ShaderKind_Basic), "material.diffuse"), 0);
            glUniform1i(glGetUniformLocation(shader_get_program(ShaderKind_Basic), "material.specular"), 1);
            glUniform1f(glGetUniformLocation(shader_get_program(ShaderKind_Basic), "material.shininess"), 128);

            for (isize i = 0; i < 4; ++i)
            {
                std::string i_str = std::to_string(i);
                GLuint pos_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic),
                                                      ("point_lights[" + i_str + "].position").c_str());
                GLuint ambient_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic),
                                                          ("point_lights[" + i_str + "].ambient").c_str());
                GLuint diffuse_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic),
                                                          ("point_lights[" + i_str + "].diffuse").c_str());
                GLuint specular_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic),
                                                           ("point_lights[" + i_str + "].specular").c_str());
                GLuint constant_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic),
                                                           ("point_lights[" + i_str + "].constant").c_str());
                GLuint linear_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic),
                                                         ("point_lights[" + i_str + "].linear").c_str());
                GLuint quadratic_loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic),
                                                            ("point_lights[" + i_str + "].quadratic").c_str());

                glUniform3f(pos_loc, lights[i].position.x, lights[i].position.y, lights[i].position.z);
                glUniform3f(ambient_loc, 0.10f, 0.10f, 0.10f);
                glUniform3f(diffuse_loc, 0.7f, 0.7f, 0.7f);
                glUniform3f(specular_loc, 1.0f, 1.0f, 1.0f);
                glUniform1f(constant_loc, 1.0f);
                glUniform1f(linear_loc, 0.35f);
                glUniform1f(quadratic_loc, 0.44f);
            }
            glUniform3f(glGetUniformLocation(shader_get_program(ShaderKind_Basic), "view_position"),
                        camera.frustum.position.x, camera.frustum.position.y, camera.frustum.position.z);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, box_texture_diffuse);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, box_texture_specular);

            glBindVertexArray(cube_vao);
            glDrawArrays(GL_TRIANGLES, 0, UNIT_CUBE_NUM_VERTICES);
            glBindVertexArray(0);

            // FLOOR

            Mat4f floor_model(1);
            floor_model = lt::scale(floor_model, Vec3f(20.0f));
            glUniformMatrix4fv(model_loc, 1, GL_FALSE, floor_model.data());

            for (isize i = 0; i < 4; ++i)
            {
                GLuint loc = glGetUniformLocation(shader_get_program(ShaderKind_Basic),
                                                  ("point_lights[" + std::to_string(i) + "].quadratic").c_str());
                glUniform3f(loc, 0.5f, 0.5f, 0.5f);
            }

            glUniform1f(glGetUniformLocation(shader_get_program(ShaderKind_Basic), "material.shininess"), 32);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, floor_texture_diffuse);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, floor_texture_diffuse);

            glBindVertexArray(floor_vao);
            glDrawArrays(GL_TRIANGLES, 0, UNIT_PLANE_NUM_VERTICES);
            glBindVertexArray(0);
        }
        {
            shader_set(ShaderKind_Light);

            for (isize i = 0; i < 4; ++i)
            {
                Mat4f model = lt::translation(Mat4f(1.0f), lights[i].position);
                model = lt::scale(model, lights[i].scaling);

                const GLuint model_loc = glGetUniformLocation(shader_get_program(ShaderKind_Light), "model");
                glUniformMatrix4fv(model_loc, 1, GL_FALSE, model.data());

                const GLuint view_loc = glGetUniformLocation(shader_get_program(ShaderKind_Light), "view");
                glUniformMatrix4fv(view_loc, 1, GL_FALSE, view.data());

                glBindVertexArray(lights[i].vao);
                glDrawArrays(GL_TRIANGLES, 0, UNIT_CUBE_NUM_VERTICES);
                glBindVertexArray(0);
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
