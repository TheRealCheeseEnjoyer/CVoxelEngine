#include "ui/UISprite.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include <cglm/affine.h>
#include <glad/glad.h>

void UISprite_init(UISprite *sprite, const char *texture, vec2 position, vec2 size) {
    glm_mat4_identity(sprite->transform);
    glm_translate(sprite->transform, (vec3) {position[0] - size[0] / 2, position[1] - size[1] / 2, 0});
    glm_scale(sprite->transform, (vec3) {size[0], size[1], 1});

    if (texture != NULL)
        sprite->texture = tm_get_texture_id(texture);
}

void UISprite_set_position(UISprite *sprite, vec2 position) {
    vec3 scale;
    mat4 rot;
    glm_decompose_rs(sprite->transform, rot, scale);
    glm_mat4_identity(sprite->transform);
    glm_translate(sprite->transform, (vec3) {position[0] - scale[0] / 2, position[1] - scale[1] / 2, 0});
    glm_scale(sprite->transform, scale);
}

void UISprite_draw(UISprite *sprite) {
    shader_set_mat4(sm_get_shader(SHADER_UI), "model", &sprite->transform);
    glBindTexture(GL_TEXTURE_2D, sprite->texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
