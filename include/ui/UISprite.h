#ifndef UISPRITE_H
#define UISPRITE_H
#include <cglm/affine.h>
#include <cglm/mat4.h>

#include "UIManager.h"

typedef struct {
    mat4 transform;
    unsigned int texture;
} UISprite;

void UISprite_init(UISprite* sprite, const char* texture, vec2 position, vec2 size) {
    glm_mat4_identity(sprite->transform);
    glm_translate(sprite->transform, (vec3) {position[0], position[1], 0});
    glm_scale(sprite->transform, (vec3) {size[0], size[1], 1});

    sprite->texture = tm_get_texture_id(texture);
}

void UISprite_set_position(UISprite* sprite, vec2 position) {
    //glm_mat4_identity(sprite->transform);
    vec3 scale;
    mat4 rot;
    glm_decompose_rs(sprite->transform, rot, scale);
    glm_mat4_identity(sprite->transform);
    glm_translate(sprite->transform, (vec3) {position[0], position[1], 0});
    glm_scale(sprite->transform, scale);
}

void UISprite_draw(UISprite* sprite) {
    shader_set_mat4(sm_get_shader(SHADER_UI), "model", &sprite->transform);
    glBindTexture(GL_TEXTURE_2D, sprite->texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#endif
