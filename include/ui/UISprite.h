#ifndef UISPRITE_H
#define UISPRITE_H
#include <cglm/affine.h>
#include <cglm/mat4.h>

typedef struct {
    mat4 transform;
    unsigned int type;
    float vertices[8];
} UISprite;

unsigned int indeces[6] = {
    2, 1, 0,
    2, 3, 1
};

float texCoords[8] = {
    0, 1,
    1, 1,
    0, 0,
    1, 0
};

void UISprite_init(UISprite* sprite, BlockType type, vec2 position, vec2 size) {
    glm_mat4_identity(sprite->transform);
    glm_translate(sprite->transform, (vec3) {position[0], position[1], 1});
    sprite->vertices[0] = position[0] - size[0] / 2;
    sprite->vertices[1] = position[1] + size[1] / 2;

    sprite->vertices[2] = position[0] + size[0] / 2;
    sprite->vertices[3] = position[1] + size[1] / 2;

    sprite->vertices[4] = position[0] - size[0] / 2;
    sprite->vertices[5] = position[1] - size[1] / 2;

    sprite->vertices[6] = position[0] + size[0] / 2;
    sprite->vertices[7] = position[1] - size[1] / 2;

    sprite->type = type;
}

#endif
