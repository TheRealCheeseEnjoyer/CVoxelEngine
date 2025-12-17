#include "ui/UISprite.h"

#include "managers/ShaderManager.h"
#include "managers/TextureManager.h"
#include <cglm/affine.h>
#include <glad/glad.h>

#include "CommonVertices.h"
#include "CVector/Vector.h"
#include "ui/UIManager.h"
#include "VoxelEngine/VoxelEngine.h"

#define INITIAL_SIZE 32
#define RESIZE_FACTOR 2JQ

static constexpr int MaxSpriteNum = 128;

typedef struct {
    unsigned int vao, vbo;
    int atlasId;
    int spriteNum;
    mat4 transform[MaxSpriteNum];
    unsigned int texture[MaxSpriteNum];
    int enabled[MaxSpriteNum];
} uisprites_t;

static uisprites_t sprites;

UISprite UISprite_init(const char *texture, vec2 position, vec2 size, bool enabled) {
    if (sprites.spriteNum == MaxSpriteNum)
        return -1;
    if (sprites.spriteNum == 0) {
        //sprites = vec_init(sizeof(uisprite_t));
        glGenVertexArrays(1, &sprites.vao);
        glBindVertexArray(sprites.vao);
        glGenBuffers(1, &sprites.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, sprites.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices_strip), quadVertices_strip, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(float), (void*) (3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        sprites.atlasId = tm_begin_dynamic_atlas(128, 1000, 1000);

        shader_use(sm_get_shader(SHADER_UI));
        mat4 ortho;
        UIManager_get_ortho_matrix(ortho);
        shader_set_mat4(sm_get_shader(SHADER_UI), "ortho", &ortho);
        shader_use(0);
    }

    sprites.enabled[sprites.spriteNum] = enabled;
    glm_mat4_identity(sprites.transform[sprites.spriteNum]);
    glm_translate(sprites.transform[sprites.spriteNum], (vec3) {position[0], position[1], 0});
    glm_scale(sprites.transform[sprites.spriteNum], (vec3) {size[0], size[1], 1});
    tm_dynamic_atlas_add_texture(sprites.atlasId, texture, sprites.spriteNum);
    return sprites.spriteNum++;
}

void UISprite_set_position(UISprite spriteIndex, vec2 position) {
    vec3 scale;
    mat4 rot;
    glm_decompose_rs(sprites.transform[spriteIndex], rot, scale);
    glm_mat4_identity(sprites.transform[spriteIndex]);
    glm_translate(sprites.transform[spriteIndex], (vec3) {position[0], position[1], 0});
    glm_scale(sprites.transform[spriteIndex], scale);
}

void UISprite_set_texture(UISprite spriteIndex, const char *texture) {
    sprites.texture[spriteIndex] = tm_get_texture_id(texture);
}

void UISprite_set_enabled(UISprite sprite, bool enabled) {
    sprites.enabled[sprite] = enabled;
}

void UISprite_get_size(UISprite spriteIndex, vec2 size) {
    vec3 scale;
    glm_decompose_scalev(sprites.transform[spriteIndex], scale);
    size[0] = scale[0];
    size[1] = scale[1];
}

void UISprite_get_position(UISprite spriteIndex, vec2 position) {
    vec2 size;
    UISprite_get_size(spriteIndex, size);

    position[0] = sprites.transform[spriteIndex][3][0];
    position[1] = sprites.transform[spriteIndex][3][1];
}

void UISprite_draw() {
    shader_use(sm_get_shader(SHADER_UI));
    shader_set_int(sm_get_shader(SHADER_UI), "TextureUnitId", 0);
    glBindVertexArray(sprites.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sprites.atlasId);
    glUniform1iv(glGetUniformLocation(sm_get_shader(SHADER_UI), "enabled"), sprites.spriteNum, sprites.enabled);
    glUniformMatrix4fv(glGetUniformLocation(sm_get_shader(SHADER_UI), "model"), sprites.spriteNum, GL_FALSE, sprites.transform[0][0]);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, sprites.spriteNum);
}
