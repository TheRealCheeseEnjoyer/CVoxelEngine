#include "../include/Player.h"

#include <stdio.h>

#include "../include/AABB.h"

#include <string.h>
#include <cglm/cam.h>
#include <cglm/vec3.h>

#include "../include/FaceOrientation.h"
#include "../include/Block.h"
#include "../include/Collisions.h"
#include "../include/InputManager.h"
#include "../include/World.h"

constexpr vec3 WorldUp = {0, 1, 0};
constexpr vec3 cameraOffset = {0, .25f, 0};

#define DEFAULT_YAW (90.0f)
#define DEFAULT_PITCH (0.0f)
#define YAW 0
#define PITCH 1
#define SENSITIVITY 0.1
#define X 0
#define Y 1
#define Z 2

#define MAX_RANGE 4

Controls *controls;

struct player_t {
    vec2 rotation; // yaw and pitch
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 position;
    float movementSpeed;
};

void player_eye_position(Player player, vec3 eye_pos) {
    glm_vec3_add(player->position, cameraOffset, eye_pos);
}

void recalculate_vectors(Player player) {
    vec3 front, right, up;
    front[0] = cosf(glm_rad(player->rotation[YAW])) * cosf(glm_rad(player->rotation[PITCH]));
    front[1] = sinf(glm_rad(player->rotation[PITCH]));
    front[2] = sinf(glm_rad(player->rotation[YAW])) * cosf(glm_rad(player->rotation[PITCH]));
    glm_normalize(front);

    glm_cross(front, WorldUp, right);
    glm_normalize(right);

    glm_cross(right, front, up);
    glm_normalize(up);

    memcpy(player->front, front, sizeof(vec3));
    memcpy(player->right, right, sizeof(vec3));
    memcpy(player->up, up, sizeof(vec3));
}

Player player_init(Controls *playerControls) {
    im_register_button(GLFW_MOUSE_BUTTON_LEFT);
    controls = playerControls;
    Player player = calloc(1, sizeof(struct player_t));
    player->movementSpeed = 15;
    player->rotation[YAW] = DEFAULT_YAW;
    player->rotation[PITCH] = DEFAULT_PITCH;
    recalculate_vectors(player);

    vec3 position = {0, 3, 0};
    memcpy(player->position, position, sizeof(vec3));
    return player;
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
                glm_vec3_subs(blockCoords, .5f, box.min);
                glm_vec3_adds(blockCoords, .5f, box.max);
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

void player_update(Player player, float deltaTime) {
    vec2 mouseDelta;
    im_get_mouse_delta(mouseDelta);
    look_around(player->rotation, mouseDelta);

    vec2 input = {0, 0};
    if (im_get_key(controls->forward))
        input[Y] += 1;
    if (im_get_key(controls->backward))
        input[Y] -= 1;
    if (im_get_key(controls->left))
        input[X] -= 1;
    if (im_get_key(controls->right))
        input[X] += 1;

    float speed = player->movementSpeed * deltaTime;
    vec3 horizontalMovement, forwardMovement, totalMovement;
    glm_vec3_scale(player->right, input[X] * speed, horizontalMovement);
    glm_vec3_scale(player->front, input[Y] * speed, forwardMovement);
    glm_vec3_add(horizontalMovement, forwardMovement, totalMovement);
    glm_vec3_add(player->position, totalMovement, player->position);

    vec3 eye, blockPos;
    FaceOrientation face;
    player_eye_position(player, eye);
    get_block_looked_at(eye, player->front, blockPos, &face);
    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
        printf("Position: %f %f %f\n", blockPos[X], blockPos[Y], blockPos[Z]);
    }

    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
        world_destroy_block(blockPos[X], blockPos[Y], blockPos[Z]);
    }

    recalculate_vectors(player);
}

void player_get_view_matrix(Player player, mat4 outView) {
    vec3 eye;
    player_eye_position(player, eye);
    vec3 center;
    glm_vec3_add(eye, player->front, center);
    glm_lookat(eye, center, player->up, outView);
}

void player_free(Player player) {
    free(player);
}
