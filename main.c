#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <unistd.h>
#include <cglm/cam.h>

#include "managers/InputManager.h"
#include "Player.h"
#include "Rigidbody.h"
#include "include/Settings.h"
#include "managers/ShaderManager.h"
#include "Skybox.h"
#include "World.h"
#include "Engine/Engine.h"
#include "Engine/Time.h"
#include "ui/UIManager.h"

int main() {
    engine_init();

    double time = glfwGetTime();
    world_init();
    double timeElapsed = glfwGetTime() - time;
    printf("Generated %dx%dx%d chunks in %f seconds\n", WORLD_SIZE_X, WORLD_SIZE_Y, WORLD_SIZE_Z, timeElapsed);
    skybox_init("yellowcloud");

    UIManager_init();
    player_init();

    double totalFrameTimes = 0;
    int numFrames = 0;
    engine_pre_main_loop();
    while (engine_isRunning()) {
        engine_main_loop_start();

        if (im_get_key_down(Settings.controls.exit)) {
            engine_terminate();
            break;
        }
        //rigidbody_update();

        player_update();

        skybox_draw();
        world_draw();
        player_draw();

        engine_main_loop_end();
        numFrames++;
        totalFrameTimes += Time.deltaTime;
    }
    printf("avg FPS: %f\n", numFrames / totalFrameTimes);
    skybox_destroy();
    return 0;
}