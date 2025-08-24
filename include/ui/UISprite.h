#ifndef UISPRITE_H
#define UISPRITE_H
#include <cglm/affine.h>
#include <cglm/mat4.h>

typedef struct {
    mat4 transform;
    unsigned int type;
} UISprite;

float vertices[] = {
    0, 0,
    1, 1,
    0, 1,

    0, 0,
    1, 0,
    1, 1
};

void UISprite_init(UISprite* sprite, BlockType type, vec2 position, vec2 size) {
    glm_mat4_identity(sprite->transform);
    glm_translate(sprite->transform, (vec3) {position[0], position[1], 0});
    glm_scale(sprite->transform, (vec3) {size[0], size[1], 1});

    sprite->type = type;
}

#endif
