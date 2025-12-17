#include <stdio.h>
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "managers/InputManager.h"
#include "Player.h"
#include "include/Settings.h"
#include "Skybox.h"
#include "ChunkManager.h"
#include "Engine/Engine.h"
#include "Engine/Time.h"
#include "ui/UIManager.h"
#include "VoxelEngine/VoxelEngine.h"

int main() {
    engine_init();
    VoxelEngine_init();
    double time = glfwGetTime();
    ChunkManager_init();
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
        ChunkManager_draw_chunks();
        player_draw();

        UIManager_draw();

        engine_main_loop_end();
        numFrames++;
        totalFrameTimes += Time.deltaTime;
    }
    printf("avg FPS: %f\n", numFrames / totalFrameTimes);
    skybox_destroy();
    return 0;
}
