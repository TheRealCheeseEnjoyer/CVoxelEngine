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
#include "include/Settings.h"
#include "include/Skybox.h"
#include "include/World.h"

GLFWwindow* window_init(const WindowSettings* settings);
bool debuggerIsAttached();

int main(void) {
    Settings settings;
    settings_load(&settings);

    GLFWwindow* window = window_init(&settings.window);

    im_init((KeyCode*)&settings.controls, sizeof(settings.controls) / sizeof(KeyCode)); // Use struct as array

    vec3 startPos = {-3, 3, -3};
    world_init(startPos);

    Player player = player_init(&settings.controls);
    skybox_init("yellowcloud");

    mat4 projection, view;
    glm_perspective(glm_rad(60), (float)settings.window.width / settings.window.height, 0.1f, 1000.0f, projection);

    float lastFrame = 0;
    double totalFrameTimes = 0;
    int numFrames = 0;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        im_update_input(window);
        glClear(GL_DEPTH_BUFFER_BIT);
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        player_update(player, deltaTime);
        player_get_view_matrix(player, view);
        if (im_get_key_down(settings.controls.exit)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        vec3 eye;
        player_eye_position(player, eye);
        skybox_draw(eye, projection, view);
        world_draw(projection, view);

        im_reset_input();
        glfwSwapBuffers(window);
        numFrames++;
        totalFrameTimes += deltaTime;
    }
    printf("avg FPS: %f\n", numFrames / totalFrameTimes);
    skybox_destroy();
    player_free(player);
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
