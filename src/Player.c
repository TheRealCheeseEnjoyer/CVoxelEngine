#include "Player.h"

#include <stdio.h>
#include <glad/glad.h>

#include <string.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/vec3.h>

#include "AABB.h"
#include "FaceOrientation.h"
#include "Block.h"
#include "Collisions.h"
#include "managers/InputManager.h"
#include "World.h"
#include "Constants.h"
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
static vec2 rotation = {DEFAULT_YAW, DEFAULT_PITCH}; // yaw and pitch
static vec3 position = {500, 17, 500};
static vec3 front;
static vec3 up;
static vec3 right;
static float movementSpeed = 7;
static float fallSpeed = 4;
static BlockType selectedBlockType = 0;
static bool is_freecam_enabled = false;
static unsigned int VAO, VBO;
static vec3 blockLookedAt = {-1, -1, -1};
static float destroyBlockCooldown = 1;
static float placeBlockCooldown = 1;
static UISprite crosshair;
static UIText fpsCounter;

static const vec3 Gravity = {0, -30.f, 0};
static vec3 velocity = {0, 0, 0};

void recalculate_vectors();

void player_init() {
    controls = &Settings.controls;
    recalculate_vectors();
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(block_vertices), block_vertices, GL_STATIC_DRAW);
    glBindVertexArray(0);
    UIInventory_init();
    UIHotbar_init();
    crosshair = UISprite_init("assets/ui/crosshair.png", (vec2){Settings.window.width / 2, Settings.window.height / 2},
                              (vec2){20, 20});
    fpsCounter = UIText_init("FPS:", (vec2){0, 50}, true);
}

void player_eye_position(vec3 eye_pos) {
    glm_vec3_add(position, cameraOffset, eye_pos);
}

void player_get_aabb(vec3 pos, AABB* out) {
    out->max[X] = pos[X] + aabbSize[X] / 2;
    out->min[X] = pos[X] - aabbSize[X] / 2;
    out->max[Y] = pos[Y] + aabbSize[Y] / 2;
    out->min[Y] = pos[Y] - aabbSize[Y] / 2;
    out->max[Z] = pos[Z] + aabbSize[Z] / 2;
    out->min[Z] = pos[Z] - aabbSize[Z] / 2;
}

bool player_is_grounded() {
    vec3 pos = {position[X], position[Y] - .01f, position[Z]};
    for (int x = -ceilf(aabbSize[X] / 2); x <= ceilf(aabbSize[X] / 2); x++) {
        for (int z = -ceilf(aabbSize[Z] / 2); z <= ceilf(aabbSize[Z] / 2); z++) {
            vec3 blockPos = {roundf(pos[X] + x), roundf(pos[Y] - 1), roundf(pos[Z] + z)};
            Block* block = world_get_block_at(blockPos[X], blockPos[Y], blockPos[Z]);
            if (block == nullptr || block->type == BLOCK_AIR)
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
    for (int x = -ceil(aabbSize[X] / 2); x <= ceil(aabbSize[X] / 2); x++) {
        for (int y = -ceil(aabbSize[Y] / 2); y <= ceil(aabbSize[Y] / 2); y++) {
            for (int z = -ceil(aabbSize[Z] / 2); z <= ceil(aabbSize[Z] / 2); z++) {
                vec3 blockPos = {round(pos[X] + x), round(pos[Y] + y), round(pos[Z] + z)};
                Block* block = world_get_block_at(blockPos[X], blockPos[Y], blockPos[Z]);
                if (block == nullptr || block->type == BLOCK_AIR)
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

void recalculate_vectors() {
    front[0] = cosf(glm_rad(rotation[YAW])) * cosf(glm_rad(rotation[PITCH]));
    front[1] = sinf(glm_rad(rotation[PITCH]));
    front[2] = sinf(glm_rad(rotation[YAW])) * cosf(glm_rad(rotation[PITCH]));
    glm_normalize(front);

    glm_cross(front, WorldUp, right);
    glm_normalize(right);

    glm_cross(right, front, up);
    glm_normalize(up);
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
    vec3 currentBlock = {roundf(eye[X]), roundf(eye[Y]), roundf(eye[Z])};
    blockPos[X] = -1;
    blockPos[Y] = -1;
    blockPos[Z] = -1;
    float minDistance = INFINITY;
    int xDir = (front[X] < 0 ? -1 : 1);
    int yDir = (front[Y] < 0 ? -1 : 1);
    int zDir = (front[Z] < 0 ? -1 : 1);
    for (int x = 0; x >= -MAX_RANGE && x <= MAX_RANGE; x += xDir) {
        for (int y = 0; y >= -MAX_RANGE && y <= MAX_RANGE; y += yDir) {
            for (int z = 0; z >= -MAX_RANGE && z <= MAX_RANGE; z += zDir) {
                vec3 blockCoords = {currentBlock[X] + x, currentBlock[Y] + y, currentBlock[Z] + z};
                Block* block = world_get_block_at(blockCoords[X], blockCoords[Y], blockCoords[Z]);
                if (block == nullptr || block->type == BLOCK_AIR) continue;

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
    glm_vec3_scale(right, input[X] * speed, horizontalMovement);
    glm_vec3_scale(front, input[Y] * speed, forwardMovement);
    glm_vec3_add(horizontalMovement, forwardMovement, totalMovement);

    glm_vec3_add(position, totalMovement, position);
}

void player_physics_update() {
    vec3 oldVelocity = {velocity[0], velocity[1], velocity[2]};
    vec3 oldPosition = {position[X], position[Y], position[Z]};

    float dt = fminf(Time.deltaTime, 1.0f / 60.0f);

    glm_vec3_muladds(Gravity, dt, oldVelocity);
    glm_vec3_muladds(oldVelocity, dt, oldPosition);

    AABB aabb;
    vec3 halfSize;
    glm_vec3_divs(aabbSize, 2, halfSize);
    glm_vec3_add(oldPosition, halfSize, aabb.max);
    glm_vec3_sub(oldPosition, halfSize, aabb.min);

    if (player_is_colliding_with_near_blocks(oldPosition)) {
        if (velocity[1] < 0 && Time.deltaTime > 1 / 60.f)
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
    vec3 forwardAxis = {front[X], 0, front[Z]};
    glm_normalize(forwardAxis);
    glm_vec3_scale(right, input[X] * speed, horizontalMovement);
    glm_vec3_scale(forwardAxis, input[Y] * speed, forwardMovement);
    glm_vec3_add(horizontalMovement, forwardMovement, totalMovement);

    vec3 newPos = {position[X], position[Y], position[Z]};
    newPos[X] += totalMovement[X];
    if (player_is_colliding_with_near_blocks(newPos))
        newPos[X] = position[X];

    newPos[Z] += totalMovement[Z];
    if (player_is_colliding_with_near_blocks(newPos))
        newPos[Z] = position[Z];

    if (im_get_key(controls->jump) && player_is_grounded() && velocity[1] == 0) {
        glm_vec3_add(velocity, jumpForce, velocity);
    }

    memcpy(position, newPos, sizeof(vec3));
    player_physics_update();
}

void player_update() {
    vec2 mouseDelta;
    char fps[32];
    sprintf(fps, "FPS: %d", (int)(1 / Time.deltaTime));
    UIText_set_text(fpsCounter, fps);
    im_get_mouse_delta(mouseDelta);
    if (UIInventory_is_enabled()) {
        if (im_get_key_down(GLFW_KEY_E))
            UIInventory_toggle();
        UIInventory_update();
        return;
    }

    look_around(rotation, mouseDelta);

    vec2 input = {0, 0};
    if (im_get_key(controls->forward))
        input[Y] += 1;
    if (im_get_key(controls->backward))
        input[Y] -= 1;
    if (im_get_key(controls->left))
        input[X] -= 1;
    if (im_get_key(controls->right))
        input[X] += 1;

    if (im_get_key_down(controls->hotbar_1)) {
        selectedBlockType = UIHotbar_move_selector_to_slot(0);
    }
    else if (im_get_key_down(controls->hotbar_2)) {
        selectedBlockType = UIHotbar_move_selector_to_slot(1);
    }
    else if (im_get_key_down(controls->hotbar_3)) {
        selectedBlockType = UIHotbar_move_selector_to_slot(2);
    }
    else if (im_get_key_down(controls->hotbar_4)) {
        selectedBlockType = UIHotbar_move_selector_to_slot(3);
    }
    else if (im_get_key_down(controls->hotbar_5)) {
        selectedBlockType = UIHotbar_move_selector_to_slot(4);
    }
    else if (im_get_key_down(controls->hotbar_6)) {
        selectedBlockType = UIHotbar_move_selector_to_slot(5);
    }
    else if (im_get_key_down(controls->hotbar_7)) {
        selectedBlockType = UIHotbar_move_selector_to_slot(6);
    }
    else if (im_get_key_down(controls->hotbar_8)) {
        selectedBlockType = UIHotbar_move_selector_to_slot(7);
    }
    else if (im_get_key_down(controls->hotbar_9)) {
        selectedBlockType = UIHotbar_move_selector_to_slot(8);
    }

    selectedBlockType = UIHotbar_move_selector_to_slot(UIHotbar_get_current_index() - im_get_scroll_direction());

    if (im_get_key_down(GLFW_KEY_E))
        UIInventory_toggle();

    if (im_get_key_down(controls->freecam)) {
        is_freecam_enabled = !is_freecam_enabled;
    }

    if (is_freecam_enabled)
        freecam_movement(input);
    else
        normal_movement(input);


    vec3 eye;
    FaceOrientation faceLookedAt;
    player_eye_position(eye);
    get_block_looked_at(eye, front, blockLookedAt, &faceLookedAt);

    destroyBlockCooldown += Time.deltaTime;
    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT) || (im_get_mouse_button(GLFW_MOUSE_BUTTON_LEFT) &&
        destroyBlockCooldown >= COOLDOWN_BLOCK_DESTRUCTION)) {
        destroyBlockCooldown = 0;
        BlockType destroyedBlock = world_destroy_block(blockLookedAt[X], blockLookedAt[Y], blockLookedAt[Z]);
        BlockStack stack = {destroyedBlock, 1, 10};
        inventory_add_block(stack);
    }

    placeBlockCooldown += Time.deltaTime;
    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_RIGHT) || (im_get_mouse_button(GLFW_MOUSE_BUTTON_RIGHT) &&
        placeBlockCooldown >= COOLDOWN_BLOCK_PLACEMENT)) {
        placeBlockCooldown = 0;
        vec3 newBlockPos = {blockLookedAt[X], blockLookedAt[Y], blockLookedAt[Z]};
        switch (faceLookedAt) {
        case FACE_TOP:
            newBlockPos[Y] += 1;
            break;
        case FACE_BOTTOM:
            newBlockPos[Y] -= 1;
            break;
        case FACE_LEFT:
            newBlockPos[X] += 1;
            break;
        case FACE_RIGHT:
            newBlockPos[X] -= 1;
            break;
        case FACE_FRONT:
            newBlockPos[Z] += 1;
            break;
        case FACE_BACK:
            newBlockPos[Z] -= 1;
            break;
        }

        if (is_freecam_enabled || !player_is_colliding_with_block(position, newBlockPos)) {
            bool success = world_place_block((int)newBlockPos[X], (int)newBlockPos[Y], (int)newBlockPos[Z], selectedBlockType);
            if (success) {
                inventory_use_block_from_hotbar();
                selectedBlockType = 0;
            }
        }
    }

    recalculate_vectors();
}

void player_position(vec3 pos) {
    memcpy(pos, position, sizeof(vec3));
}

void player_draw(mat4 projection) {
    glBindVertexArray(VAO);
    const Shader shader = sm_get_shader(SHADER_DEFAULT);
    shader_use(shader);
    mat4 bounds, view;
    glm_mat4_identity(bounds);
    glm_translate(bounds, blockLookedAt);
    vec3 scale = {1.01f, 1.01f, 1.01f};
    glm_scale(bounds, scale);
    player_get_view_matrix(view);
    glm_mat4_scale(bounds, 1.01f);
    mat4 finalMatrix;
    glm_mat4_mul(projection, view, finalMatrix);
    glm_mat4_mul(finalMatrix, bounds, finalMatrix);
    shader_set_mat4(shader, "finalMatrix", &finalMatrix);
    shader_set_int(shader, "TextureUnitId", 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);

    UIManager_begin_draw();
    UIText_draw(fpsCounter);
    UISprite_draw(crosshair);
    UIHotbar_draw();
    UIInventory_draw();
    UIManager_end_draw();
}

void player_get_view_matrix(mat4 outView) {
    vec3 eye;
    player_eye_position(eye);
    glm_look(eye, front, up, outView);
}
