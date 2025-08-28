#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <unistd.h>
#include <cglm/cam.h>

#include "InputManager.h"
#include "Player.h"
#include "Rigidbody.h"
#include "Settings.h"
#include "ShaderManager.h"
#include "Skybox.h"
#include "TextureManager.h"
#include "libs/thpool.h"
#include "WindowManager.h"
#include "World.h"
#include "ui/Hotbar.h"
#include "ui/UIManager.h"

int main() {
    Settings settings;
    settings_load(&settings);
    thpool_init(16);
    tm_init();
    GLFWwindow* window = window_create(&settings.window);

    im_init(window, (KeyCode*)&settings.controls, sizeof(settings.controls) / sizeof(KeyCode)); // Use struct as array
    im_register_button(GLFW_MOUSE_BUTTON_LEFT);
    im_register_button(GLFW_MOUSE_BUTTON_RIGHT);

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

    UIManager_init();

    Hotbar_init();
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

        UIManager_begin_draw();
        Hotbar_draw();
        UIManager_end_draw();

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