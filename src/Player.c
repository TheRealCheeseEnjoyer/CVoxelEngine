#include "../include/Player.h"

#include <stdio.h>
#include <string.h>
#include <cglm/cam.h>
#include <cglm/vec3.h>

#include "../include/AABB.h"
#include "../include/FaceOrientation.h"
#include "../include/Block.h"
#include "../include/Collisions.h"
#include "../include/InputManager.h"
#include "../include/World.h"
#include "../include/Constants.h"

constexpr vec3 WorldUp = {0, 1, 0};
constexpr vec3 cameraOffset = {0, .75f, 0};

#define DEFAULT_YAW (90.0f)
#define DEFAULT_PITCH (0.0f)
#define YAW 0
#define PITCH 1
#define SENSITIVITY 0.1

#define MAX_RANGE 4

Controls* controls;

vec3 aabbSize = {.5f, 2, .5f};
vec2 rotation = {DEFAULT_YAW, DEFAULT_PITCH}; // yaw and pitch
vec3 position = {0, 3, 0};
vec3 front;
vec3 up;
vec3 right;
float movementSpeed = 15;
float fallSpeed = 2;
BlockType selectedBlock = BLOCK_GRASS;

void recalculate_vectors();

void player_init(Controls *playerControls) {
    controls = playerControls;
    recalculate_vectors();
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

void get_block_looked_at(vec3 eye, vec3 front, vec3 blockPos, FaceOrientation *faceHit) {
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
                Block *block = world_get_block_at(blockCoords[X], blockCoords[Y], blockCoords[Z]);
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

void player_update(float deltaTime) {
    vec2 mouseDelta;
    im_get_mouse_delta(mouseDelta);
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

    float speed = movementSpeed * deltaTime;
    vec3 horizontalMovement, forwardMovement, totalMovement;
    glm_vec3_scale(right, input[X] * speed, horizontalMovement);
    glm_vec3_scale(front, input[Y] * speed, forwardMovement);
    glm_vec3_add(horizontalMovement, forwardMovement, totalMovement);

    vec3 newPos = {position[X], position[Y], position[Z]};
    newPos[X] += totalMovement[X];
    if (player_is_colliding_with_near_blocks(newPos))
        newPos[X] = position[X];

    newPos[Z] += totalMovement[Z];
    if (player_is_colliding_with_near_blocks(newPos))
        newPos[Z] = position[Z];

    newPos[Y] -= fallSpeed * deltaTime;
    if (player_is_colliding_with_near_blocks(newPos))
        newPos[Y] = position[Y];

    memcpy(position, newPos, sizeof(vec3));

    vec3 eye, blockLookedAt;
    FaceOrientation faceLookedAt;
    player_eye_position(eye);
    get_block_looked_at(eye, front, blockLookedAt, &faceLookedAt);
    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
        printf("Position: %f %f %f\n", blockLookedAt[X], blockLookedAt[Y], blockLookedAt[Z]);
    }

    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
        world_destroy_block(blockLookedAt[X], blockLookedAt[Y], blockLookedAt[Z]);
    }

    if (im_get_key_down(GLFW_KEY_1))
        selectedBlock = BLOCK_GRASS;
    else if (im_get_key_down(GLFW_KEY_2))
        selectedBlock = BLOCK_ROCK;

    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_RIGHT)) {
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

        if (!player_is_colliding_with_block(position, newBlockPos))
            world_place_block(newBlockPos[X], newBlockPos[Y], newBlockPos[Z], selectedBlock);
    }

    recalculate_vectors();
}

void player_get_view_matrix(mat4 outView) {
    vec3 eye;
    player_eye_position(eye);
    vec3 center;
    glm_vec3_add(eye, front, center);
    glm_lookat(eye, center, up, outView);
}
