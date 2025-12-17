#include "Player.h"

#include <stdio.h>
#include <glad/glad.h>

#include <string.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/vec3.h>

#include "AABB.h"
#include "Chunk.h"
#include "FaceOrientation.h"
#include "../include/VoxelEngine/Block.h"
#include "Collisions.h"
#include "managers/InputManager.h"
#include "ChunkManager.h"
#include "CommonVertices.h"
#include "Inventory.h"
#include "Engine/Time.h"
#include "managers/ShaderManager.h"
#include "ui/UIHotbar.h"
#include "ui/UIInventory.h"
#include "ui/UIManager.h"
#include "ui/UIText.h"

constexpr vec3 WorldUp = {0, 1, 0};
constexpr vec3 cameraOffset = {0, .75f, 0};

#define DEFAULT_YAW (90.0f)
#define DEFAULT_PITCH (0.0f)
#define YAW 0
#define PITCH 1
#define SENSITIVITY 0.1
#define COOLDOWN_BLOCK_DESTRUCTION .2f
#define COOLDOWN_BLOCK_PLACEMENT .2f

#define MAX_RANGE 4

struct controls_t* controls;

static constexpr vec3 jumpForce = {0, 10, 0};
static constexpr vec3 aabbSize = {.5f, 1.99f, .5f};

static mat4 projectionMatrix;
static mat4 viewMatrix;

static vec2 rotation = {DEFAULT_YAW, DEFAULT_PITCH}; // yaw and pitch
static vec3 position = {500, 17, 500};
static vec3 front;
static vec3 up;
static vec3 right;
static float movementSpeed = 7;
static float fallSpeed = 4;
static BlockId selectedBlock = 0;
static bool is_freecam_enabled = false;
static unsigned int vao, vbo;
static vec3 blockLookedAt = {-1, -1, -1};
static float destroyBlockCooldown = 1;
static float placeBlockCooldown = 1;
static UISprite crosshair;
static UIText fpsCounter;
static UIText posText;
static UIText chunkCoords;

static const vec3 Gravity = {0, -30.f, 0};
static vec3 velocity = {0, 0, 0};

void recalculate_view();

void player_init() {
    controls = &Settings.controls;
    glm_perspective(glm_rad(120), (float)Settings.window.width / (float)Settings.window.height, 0.1f, 1000.0f,
                    projectionMatrix);
    recalculate_view();
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glBindVertexArray(0);

    UIInventory_init();
    UIHotbar_init();
    crosshair = UISprite_init("assets/ui/crosshair.png", (vec2){Settings.window.width / 2, Settings.window.height / 2},
                              (vec2){20, 20}, true);
    fpsCounter = UIText_init("FPS:", (vec2){20, 70}, true);
    posText = UIText_init("position:", (vec2){600, 70}, true);
    chunkCoords = UIText_init("blockCoords:", (vec2){20, 200}, true);
}

void player_get_eye_position(vec3 eye_pos) {
    glm_vec3_add(position, cameraOffset, eye_pos);
}

void player_get_aabb(vec3 pos, AABB* out) {
    out->max[0] = pos[0] + aabbSize[0] / 2;
    out->min[0] = pos[0] - aabbSize[0] / 2;
    out->max[1] = pos[1] + aabbSize[1] / 2;
    out->min[1] = pos[1] - aabbSize[1] / 2;
    out->max[2] = pos[2] + aabbSize[2] / 2;
    out->min[2] = pos[2] - aabbSize[2] / 2;
}

bool player_is_grounded() {
    vec3 pos = {position[0], position[1] - .01f, position[2]};
    for (int x = -ceilf(aabbSize[0] / 2); x <= ceilf(aabbSize[0] / 2); x++) {
        for (int z = -ceilf(aabbSize[2] / 2); z <= ceilf(aabbSize[2] / 2); z++) {
            vec3 blockPos = {roundf(pos[0] + x), roundf(pos[1] - 1), roundf(pos[2] + z)};
            BlockId block = world_get_block_at(blockPos[0], blockPos[1], blockPos[2]);
            if (block == BLOCK_INVALID_ID || block == BLOCK_AIR)
                continue;

            AABB blockAABB;
            AABB playerAABB;
            player_get_aabb(pos, &playerAABB);
            block_get_aabb(blockPos, &blockAABB);
            if (collisions_aabb_to_aabb(playerAABB, blockAABB)) {
                return true;
            }
        }
    }

    return false;
}

bool player_is_colliding_with_near_blocks(vec3 pos) {
    AABB player;
    player_get_aabb(pos, &player);
    for (int x = -ceil(aabbSize[0] / 2); x <= ceil(aabbSize[0] / 2); x++) {
        for (int y = -ceil(aabbSize[1] / 2); y <= ceil(aabbSize[1] / 2); y++) {
            for (int z = -ceil(aabbSize[2] / 2); z <= ceil(aabbSize[2] / 2); z++) {
                vec3 blockPos = {round(pos[0] + x), round(pos[1] + y), round(pos[2] + z)};
                BlockId block = world_get_block_at(blockPos[0], blockPos[1], blockPos[2]);
                if (block == BLOCK_INVALID_ID || block == BLOCK_AIR)
                    continue;

                AABB blockAABB;
                block_get_aabb(blockPos, &blockAABB);
                if (collisions_aabb_to_aabb(player, blockAABB)) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool player_is_colliding_with_block(vec3 playerPos, vec3 blockPos) {
    AABB playerAABB;
    player_get_aabb(playerPos, &playerAABB);
    AABB blockAABB;
    block_get_aabb(blockPos, &blockAABB);
    return collisions_aabb_to_aabb(playerAABB, blockAABB);
}

void recalculate_view() {
    front[0] = cosf(glm_rad(rotation[YAW])) * cosf(glm_rad(rotation[PITCH]));
    front[1] = sinf(glm_rad(rotation[PITCH]));
    front[2] = sinf(glm_rad(rotation[YAW])) * cosf(glm_rad(rotation[PITCH]));
    glm_normalize(front);

    glm_cross(front, WorldUp, right);
    glm_normalize(right);

    glm_cross(right, front, up);
    glm_normalize(up);

    vec3 eye;
    player_get_eye_position(eye);
    glm_look(eye, front, up, viewMatrix);
}

void look_around(vec2 rotation, vec2 mouseDelta) {
    rotation[YAW] += mouseDelta[YAW] * SENSITIVITY;
    rotation[PITCH] -= mouseDelta[PITCH] * SENSITIVITY;

    if (rotation[PITCH] > 89.0f)
        rotation[PITCH] = 89.0f;
    if (rotation[PITCH] < -89.0f)
        rotation[PITCH] = -89.0f;
}

void get_block_looked_at(vec3 eye, vec3 front, vec3 blockPos, FaceOrientation* faceHit) {
    vec3 currentBlock = {roundf(eye[0]), roundf(eye[1]), roundf(eye[2])};
    blockPos[0] = -1;
    blockPos[1] = -1;
    blockPos[2] = -1;
    float minDistance = INFINITY;
    int xDir = (front[0] < 0 ? -1 : 1);
    int yDir = (front[1] < 0 ? -1 : 1);
    int zDir = (front[2] < 0 ? -1 : 1);
    for (int x = 0; x >= -MAX_RANGE && x <= MAX_RANGE; x += xDir) {
        for (int y = 0; y >= -MAX_RANGE && y <= MAX_RANGE; y += yDir) {
            for (int z = 0; z >= -MAX_RANGE && z <= MAX_RANGE; z += zDir) {
                vec3 blockCoords = {currentBlock[0] + x, currentBlock[1] + y, currentBlock[2] + z};
                BlockId block = world_get_block_at(blockCoords[0], blockCoords[1], blockCoords[2]);
                if (block == BLOCK_INVALID_ID || block == BLOCK_AIR) continue;

                float distance;
                FaceOrientation face;
                AABB box;
                block_get_aabb(blockCoords, &box);
                if (collisions_ray_to_aabb(eye, front, box, &distance, &face)) {
                    if (distance > MAX_RANGE) continue;
                    if (distance < minDistance) {
                        *faceHit = face;
                        minDistance = distance;
                        memcpy(blockPos, blockCoords, sizeof(ivec3));
                    }
                }
            }
        }
    }
}

void freecam_movement(vec2 input) {
    float speed = movementSpeed * Time.deltaTime;
    vec3 horizontalMovement, forwardMovement, totalMovement;
    glm_vec3_scale(right, input[0] * speed, horizontalMovement);
    glm_vec3_scale(front, input[1] * speed, forwardMovement);
    glm_vec3_add(horizontalMovement, forwardMovement, totalMovement);

    glm_vec3_add(position, totalMovement, position);
}

void player_physics_update() {
    vec3 oldVelocity = {velocity[0], velocity[1], velocity[2]};
    vec3 oldPosition = {position[0], position[1], position[2]};

    float dt = fminf(Time.deltaTime, 1.0f / 60.0f);

    glm_vec3_muladds(Gravity, dt, oldVelocity);
    glm_vec3_muladds(oldVelocity, dt, oldPosition);

    AABB aabb;
    vec3 halfSize;
    glm_vec3_divs(aabbSize, 2, halfSize);
    glm_vec3_add(oldPosition, halfSize, aabb.max);
    glm_vec3_sub(oldPosition, halfSize, aabb.min);

    if (player_is_colliding_with_near_blocks(oldPosition)) {
        if (velocity[1] < 0 && Time.deltaTime > 1 / 70.f)
            position[1] = floorf(oldPosition[1]) + .5f;

        memset(velocity, 0, sizeof(vec3));
        return;
    }

    memcpy(velocity, oldVelocity, sizeof(vec3));
    memcpy(position, oldPosition, sizeof(vec3));
}

void normal_movement(vec2 input) {
    float speed = movementSpeed * Time.deltaTime;
    vec3 horizontalMovement, forwardMovement, totalMovement;
    vec3 forwardAxis = {front[0], 0, front[2]};
    glm_normalize(forwardAxis);
    glm_vec3_scale(right, input[0] * speed, horizontalMovement);
    glm_vec3_scale(forwardAxis, input[1] * speed, forwardMovement);
    glm_vec3_add(horizontalMovement, forwardMovement, totalMovement);

    vec3 newPos = {position[0], position[1], position[2]};
    newPos[0] += totalMovement[0];
    if (player_is_colliding_with_near_blocks(newPos))
        newPos[0] = position[0];

    newPos[2] += totalMovement[2];
    if (player_is_colliding_with_near_blocks(newPos))
        newPos[2] = position[2];

    if (im_get_key(controls->jump) && player_is_grounded() && velocity[1] == 0) {
        glm_vec3_add(velocity, jumpForce, velocity);
    }

    memcpy(position, newPos, sizeof(vec3));
    player_physics_update();
}

void player_update() {
    vec2 mouseDelta;
    static int loopCounter = 0;
    constexpr int avgFpsNum = 60;
    static float deltaTimes[avgFpsNum];
    loopCounter++;
    loopCounter = loopCounter % avgFpsNum;
    deltaTimes[loopCounter] = Time.deltaTime;
    float avgFps = 0;
    for (int i = 0; i < avgFpsNum; i++) {
        avgFps += deltaTimes[i];
    }
    avgFps = avgFpsNum / avgFps;
    char fps[32];
    sprintf(fps, "FPS: %d", (int)(avgFps));
    UIText_set_text(fpsCounter, fps);

    char pos[32];
    sprintf(pos, "Player x: %d y: %d z: %d", (int)position[0], (int)position[1], (int)position[2]);
    UIText_set_text(posText, pos);
    sprintf(pos, "Chunk x: %d y: %d z: %d", (int)position[0] / CHUNK_SIZE_X, (int)position[1] / CHUNK_SIZE_Y, (int)position[2] / CHUNK_SIZE_Z);
    UIText_set_text(chunkCoords, pos);

    if (im_get_key_down(GLFW_KEY_E)) {
        UIInventory_toggle();
        UISprite_set_enabled(crosshair, !UIInventory_is_enabled());
    }
    if (UIInventory_is_enabled()) {
        UIInventory_update();
        return;
    }

    im_get_mouse_delta(mouseDelta);
    look_around(rotation, mouseDelta);

    vec2 input = {0, 0};
    if (im_get_key(controls->forward))
        input[1] += 1;
    if (im_get_key(controls->backward))
        input[1] -= 1;
    if (im_get_key(controls->left))
        input[0] -= 1;
    if (im_get_key(controls->right))
        input[0] += 1;

    if (im_get_key_down(controls->hotbar_1)) {
        selectedBlock = UIHotbar_move_selector_to_slot(0);
    }
    else if (im_get_key_down(controls->hotbar_2)) {
        selectedBlock = UIHotbar_move_selector_to_slot(1);
    }
    else if (im_get_key_down(controls->hotbar_3)) {
        selectedBlock = UIHotbar_move_selector_to_slot(2);
    }
    else if (im_get_key_down(controls->hotbar_4)) {
        selectedBlock = UIHotbar_move_selector_to_slot(3);
    }
    else if (im_get_key_down(controls->hotbar_5)) {
        selectedBlock = UIHotbar_move_selector_to_slot(4);
    }
    else if (im_get_key_down(controls->hotbar_6)) {
        selectedBlock = UIHotbar_move_selector_to_slot(5);
    }
    else if (im_get_key_down(controls->hotbar_7)) {
        selectedBlock = UIHotbar_move_selector_to_slot(6);
    }
    else if (im_get_key_down(controls->hotbar_8)) {
        selectedBlock = UIHotbar_move_selector_to_slot(7);
    }
    else if (im_get_key_down(controls->hotbar_9)) {
        selectedBlock = UIHotbar_move_selector_to_slot(8);
    }

    selectedBlock = UIHotbar_move_selector_to_slot(UIHotbar_get_current_index() - im_get_scroll_direction());

    if (im_get_key_down(controls->freecam)) {
        is_freecam_enabled = !is_freecam_enabled;
    }

    if (is_freecam_enabled)
        freecam_movement(input);
    else
        normal_movement(input);


    vec3 eye;
    FaceOrientation faceLookedAt;
    player_get_eye_position(eye);
    get_block_looked_at(eye, front, blockLookedAt, &faceLookedAt);

    destroyBlockCooldown += Time.deltaTime;
    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT) || (im_get_mouse_button(GLFW_MOUSE_BUTTON_LEFT) &&
        destroyBlockCooldown >= COOLDOWN_BLOCK_DESTRUCTION)) {
        destroyBlockCooldown = 0;
        BlockId destroyedBlock = world_destroy_block(blockLookedAt[0], blockLookedAt[1], blockLookedAt[2]);
        BlockStack stack = {destroyedBlock, 1, 10};
        inventory_add_block(stack);
    }

    placeBlockCooldown += Time.deltaTime;
    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_RIGHT) || (im_get_mouse_button(GLFW_MOUSE_BUTTON_RIGHT) &&
        placeBlockCooldown >= COOLDOWN_BLOCK_PLACEMENT)) {
        placeBlockCooldown = 0;
        vec3 newBlockPos = {blockLookedAt[0], blockLookedAt[1], blockLookedAt[2]};
        switch (faceLookedAt) {
        case FACE_TOP:
            newBlockPos[1] += 1;
            break;
        case FACE_BOTTOM:
            newBlockPos[1] -= 1;
            break;
        case FACE_LEFT:
            newBlockPos[0] += 1;
            break;
        case FACE_RIGHT:
            newBlockPos[0] -= 1;
            break;
        case FACE_FRONT:
            newBlockPos[2] += 1;
            break;
        case FACE_BACK:
            newBlockPos[2] -= 1;
            break;
        }

        if (is_freecam_enabled || !player_is_colliding_with_block(position, newBlockPos)) {
            bool success = world_place_block((int)newBlockPos[0], (int)newBlockPos[1], (int)newBlockPos[2],
                                             selectedBlock);
            if (success) {
                inventory_use_block_from_hotbar();
                selectedBlock = 0;
            }
        }
    }

    recalculate_view();
}

void player_get_position(vec3 pos) {
    memcpy(pos, position, sizeof(vec3));
}

void player_draw() {
    glBindVertexArray(vao);
    const Shader shader = sm_get_shader(SHADER_DEFAULT);
    shader_use(shader);
    mat4 bounds, view;
    glm_mat4_identity(bounds);
    glm_translate(bounds, blockLookedAt);
    vec3 scale = {1.01f, 1.01f, 1.01f};
    glm_scale(bounds, scale);
    player_get_view(view);
    glm_mat4_scale(bounds, 1.01f);

    shader_set_mat4(shader, "model", &bounds);
    shader_set_int(shader, "TextureUnitId", 2);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);
}

void player_get_view(mat4 outView) {
    memcpy(outView, viewMatrix, sizeof(mat4));
}

void player_get_projection(mat4 outProjection) {
    memcpy(outProjection, projectionMatrix, sizeof(mat4));
}
