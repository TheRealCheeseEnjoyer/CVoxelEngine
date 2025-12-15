#include "Engine/Engine.h"

#include <time.h>
#include <glad/glad.h>

#include "Engine/Time.h"
#include "Settings.h"
#include "thpool.h"
#include "managers/ShaderManager.h"
#include "managers/TextureManager.h"
#include "managers/WindowManager.h"

static GLFWwindow* window;

struct time_t Time = {};

void engine_init() {
    srand(0);
    settings_load();
    thpool_init(16);
    window = window_create();
    tm_init();
    sm_init();

    im_init(window, (KeyCode*)&Settings.controls, sizeof(Settings.controls) / sizeof(KeyCode)); // Use struct as array
    im_register_button(GLFW_MOUSE_BUTTON_LEFT);
    im_register_button(GLFW_MOUSE_BUTTON_RIGHT);
}

bool engine_isRunning() {
    return !glfwWindowShouldClose(window);
}

void engine_pre_main_loop() {
    Time.elapsedTime = (float)glfwGetTime();
    Time.deltaTime = 0;
}

void engine_main_loop_start() {
    glfwPollEvents();
    im_update_input(window);
    glClear(GL_DEPTH_BUFFER_BIT);
    float currentTime = (float)glfwGetTime();
    Time.deltaTime = currentTime - Time.elapsedTime;
    Time.elapsedTime = currentTime;
}

void engine_main_loop_end() {
    im_reset_input();
    glfwSwapBuffers(window);
}

void engine_terminate() {
    im_destroy();
    //thpool_destroy();

    glfwSetWindowShouldClose(window, GLFW_TRUE);
    glfwDestroyWindow(window);
    glfwTerminate();
}
