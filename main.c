#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unistd.h>
#include <fcntl.h>
#include <cglm/cam.h>

#include "include/InputManager.h"
#include "include/Player.h"
#include "include/Rigidbody.h"
#include "include/Settings.h"
#include "include/ShaderManager.h"
#include "include/Skybox.h"
#include "include/thpool.h"
#include "include/World.h"

GLFWwindow* window_init(const WindowSettings* settings);
bool debuggerIsAttached();

int main() {
    Settings settings;
    settings_load(&settings);
    thpool_init(16);
    GLFWwindow* window = window_init(&settings.window);

    im_init((KeyCode*)&settings.controls, sizeof(settings.controls) / sizeof(KeyCode)); // Use struct as array
    im_register_button(GLFW_MOUSE_BUTTON_LEFT);
    im_register_button(GLFW_MOUSE_BUTTON_RIGHT);
    im_register_key(GLFW_KEY_1);
    im_register_key(GLFW_KEY_2);

    vec3 startPos = {-3, 3, -3};
    sm_init();
    double time = glfwGetTime();
    world_init(startPos);
    double timeElapsed = glfwGetTime() - time;
    printf("Generated %dx%dx%d chunks in %f seconds\n", WORLD_SIZE_X, WORLD_SIZE_Y, WORLD_SIZE_Z, timeElapsed);
    player_init(&settings.controls);
    skybox_init("yellowcloud");

    mat4 projection, view;
    glm_perspective(glm_rad(90), (float)settings.window.width / settings.window.height, 0.1f, 1000.0f, projection);

    float lastFrame = glfwGetTime();
    double totalFrameTimes = 0;
    int numFrames = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        im_update_input(window);
        glClear(GL_DEPTH_BUFFER_BIT);
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        rigidbody_update(deltaTime);

        player_update(deltaTime);
        player_get_view_matrix(view);
        if (im_get_key_down(settings.controls.exit)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        vec3 eye, pos;
        player_eye_position(eye);
        player_position(pos);
        skybox_draw(eye, projection, view);
        world_draw(pos, projection, view);
        player_draw(projection);

        im_reset_input();
        glfwSwapBuffers(window);
        numFrames++;
        totalFrameTimes += deltaTime;
    }
    printf("avg FPS: %f\n", numFrames / totalFrameTimes);
    skybox_destroy();
    thpool_destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

GLFWwindow* window_init(const WindowSettings* settings) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(settings->width, settings->height, settings->title, nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        exit(1);
    }
    glViewport(0, 0, settings->width, settings->height);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    if (!debuggerIsAttached())
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return window;
}

bool debuggerIsAttached() {
    char buf[4096];

    const int status_fd = open("/proc/self/status", O_RDONLY);
    if (status_fd == -1)
        return false;

    const ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);
    close(status_fd);

    if (num_read <= 0)
        return false;

    buf[num_read] = '\0';
    constexpr char tracerPidString[] = "TracerPid:";
    const auto tracer_pid_ptr = strstr(buf, tracerPidString);
    if (!tracer_pid_ptr)
        return false;

    for (const char* characterPtr = tracer_pid_ptr + sizeof(tracerPidString) - 1;
         characterPtr <= buf + num_read; ++characterPtr) {
        if (isspace(*characterPtr))
            continue;
        else
            return isdigit(*characterPtr) != 0 && *characterPtr != '0';
         }

    return false;
}
