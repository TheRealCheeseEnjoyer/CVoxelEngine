#include "managers/WindowManager.h"

#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>

#include "managers/SettingsManager.h"

static GLFWwindow* window = nullptr;

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
        if (!isspace(*characterPtr))
            return isdigit(*characterPtr) != 0 && *characterPtr != '0';
         }

    return false;
}


GLFWwindow* window_create() {
    WindowSettings* settings = settings_manager_get_window_settings();
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWmonitor* monitor = nullptr;
    if (settings->fullscreen) {
        monitor = glfwGetPrimaryMonitor();
    }

    window = glfwCreateWindow(settings->width, settings->height, settings->title, monitor, nullptr);
    if (window == nullptr) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        exit(1);
    }
    glViewport(0, 0, settings->width, settings->height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    //if (!debuggerIsAttached())
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return window;
}

GLFWwindow* window_get_handler() {
    return window;
}

void window_get_size(vec2 size) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    size[0] = width;
    size[1] = height;
}


