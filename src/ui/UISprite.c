#include "ui/UISprite.h"

#include "managers/ShaderManager.h"
#include "managers/TextureManager.h"
#include <cglm/affine.h>
#include <glad/glad.h>

#include "CommonVertices.h"
#include "CVector/Vector.h"
#include "ui/UIManager.h"

#define INITIAL_SIZE 32
#define RESIZE_FACTOR 2

typedef struct {
    mat4 transform;
    unsigned int texture;
} uisprite_t;

static uisprite_t* sprites = nullptr;
static unsigned int vao, vbo;

UISprite UISprite_init(const char *texture, vec2 position, vec2 size) {
    if (!sprites) {
        sprites = vec_init(sizeof(uisprite_t));
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    uisprite_t sprite;

    glm_mat4_identity(sprite.transform);
    glm_translate(sprite.transform, (vec3) {position[0], position[1], 0});
    glm_scale(sprite.transform, (vec3) {size[0], size[1], 1});
    sprite.texture = tm_get_texture_id(texture);
    vec_append(&sprites, &sprite);
    return vec_size(sprites) - 1;
}

void UISprite_set_position(UISprite spriteIndex, vec2 position) {
    vec3 scale;
    mat4 rot;
    glm_decompose_rs(sprites[spriteIndex].transform, rot, scale);
    glm_mat4_identity(sprites[spriteIndex].transform);
    glm_translate(sprites[spriteIndex].transform, (vec3) {position[0], position[1], 0});
    glm_scale(sprites[spriteIndex].transform, scale);
}

void UISprite_set_texture(UISprite spriteIndex, const char *texture) {
    sprites[spriteIndex].texture = tm_get_texture_id(texture);
}

void UISprite_get_size(UISprite spriteIndex, vec2 size) {
    vec3 scale;
    glm_decompose_scalev(sprites[spriteIndex].transform, scale);
    size[0] = scale[0];
    size[1] = scale[1];
}

void UISprite_get_position(UISprite spriteIndex, vec2 position) {
    vec2 size;
    UISprite_get_size(spriteIndex, size);

    position[0] = sprites[spriteIndex].transform[3][0];
    position[1] = sprites[spriteIndex].transform[3][1];
}

void UISprite_draw(UISprite spriteIndex) {
    shader_use(sm_get_shader(SHADER_UI));
    mat4 ortho;
    UIManager_get_ortho_matrix(ortho);
    shader_set_mat4(sm_get_shader(SHADER_UI), "ortho", &ortho);
    shader_set_int(sm_get_shader(SHADER_UI), "TextureUnitId", 0);
    shader_set_mat4(sm_get_shader(SHADER_UI), "model", &sprites[spriteIndex].transform);
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, sprites[spriteIndex].texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
