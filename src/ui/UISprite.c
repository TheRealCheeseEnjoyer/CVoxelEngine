#include "ui/UISprite.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include <cglm/affine.h>
#include <glad/glad.h>

void UISprite_init(UISprite *sprite, const char *texture, vec2 position, vec2 size, bool enabled) {
    glm_mat4_identity(sprite->transform);
    glm_translate(sprite->transform, (vec3) {position[0] - size[0] / 2, position[1] - size[1] / 2, 0});
    glm_scale(sprite->transform, (vec3) {size[0], size[1], 1});

    if (texture != nullptr)
        sprite->texture = tm_get_texture_id(texture);

    sprite->enabled = enabled;
}

void UISprite_set_position(UISprite *sprite, vec2 position) {
    vec3 scale;
    mat4 rot;
    glm_decompose_rs(sprite->transform, rot, scale);
    glm_mat4_identity(sprite->transform);
    glm_translate(sprite->transform, (vec3) {position[0] - scale[0] / 2, position[1] - scale[1] / 2, 0});
    glm_scale(sprite->transform, scale);
}

void UISprite_set_texture(UISprite *sprite, const char *texture) {
    sprite->texture = tm_get_texture_id(texture);
}

void UISprite_set_enabled(UISprite *sprite, bool enabled) {
    sprite->enabled = enabled;
}

void UISprite_get_size(UISprite *sprite, vec2 size) {
    vec3 scale;
    glm_decompose_scalev(sprite->transform, scale);
    size[0] = scale[0];
    size[1] = scale[1];
}

void UISprite_get_position(UISprite *sprite, vec2 position) {
    vec2 size;
    UISprite_get_size(sprite, size);
    position[0] = sprite->transform[3][0] + size[0] / 2;
    position[1] = sprite->transform[3][1] + size[1] / 2;
}

void UISprite_draw(UISprite *sprite) {
    if (!sprite->enabled) return;
    shader_set_mat4(sm_get_shader(SHADER_UI), "model", &sprite->transform);
    glBindTexture(GL_TEXTURE_2D, sprite->texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
