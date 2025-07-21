#include "../include/Player.h"

#include <string.h>
#include <cglm/cam.h>
#include <cglm/vec3.h>

#include "../include/InputManager.h"

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

Controls* controls;

struct player_t {
    vec2 rotation; // yaw and pitch
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 position;
    float movementSpeed;
};

void eye_position(Player player, vec3 eye_pos) {
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

Player player_init(Controls* playerControls) {
    controls = playerControls;
    Player player = calloc(1, sizeof(struct player_t));
    player->movementSpeed = 15;
    player->rotation[0] = DEFAULT_YAW;
    player->rotation[1] = DEFAULT_PITCH;
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

void player_update(Player player, float deltaTime) {
    vec2 mouseDelta;
    im_get_mouse_delta(mouseDelta);
    look_around(player->rotation, mouseDelta);

    vec2 input = {0, 0};
    if (im_get_key(controls->forward))
        input[X] += 1;
    if (im_get_key(controls->backward))
        input[X] -= 1;
    if (im_get_key(controls->left))
        input[Y] -= 1;
    if (im_get_key(controls->right))
        input[Y] += 1;

    float speed = player->movementSpeed * deltaTime;
    vec3 rightMovement, forwardMovement, movement;
    glm_vec3_scale(player->right, input[Y] * speed, rightMovement);
    glm_vec3_scale(player->front, input[X] * speed, forwardMovement);
    glm_vec3_add(rightMovement, forwardMovement, movement);
    glm_vec3_add(player->position, movement, player->position);

    recalculate_vectors(player);
}

void player_get_view_matrix(Player player, mat4 outView) {
    vec3 eye;
    eye_position(player, eye);
    vec3 center;
    glm_vec3_add(eye, player->front, center);
    glm_lookat(eye, center, player->up, outView);
}

void player_free(Player player) {
    free(player);
}
