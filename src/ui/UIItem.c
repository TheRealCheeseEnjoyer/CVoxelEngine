#include "UIItem.h"

#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <glad/glad.h>

#include "CommonVertices.h"
#include "Player.h"
#include "Shader.h"
#include "CVector/Vector.h"
#include "managers/ShaderManager.h"
#include "ui/UIManager.h"
#include "VoxelEngine/BlockId.h"
#include "VoxelEngine/VoxelEngine.h"

struct ui_item {
    mat4 model;
    BlockId type;
};

static unsigned int vao, vbo;
static struct ui_item* items = nullptr;

UIItem UIItem_init(const BlockId type, vec2 position, vec2 size) {
    if (items == nullptr) {
        items = vec_init(sizeof(struct ui_item));
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    struct ui_item item;
    item.type = type;
    glm_mat4_identity(item.model);
    glm_translate(item.model, (vec3){position[0], position[1], 0});
    // TODO: rotation (like minecraft)
    glm_scale(item.model, (vec3){size[0], size[1], 1});

    vec_append(&items, &item);

    return vec_size(items) - 1;
}

void UIItem_set_type(const UIItem item, const BlockId type) {
    items[item].type = type;
}

void UIItem_set_position(const UIItem item, vec2 position) {
    mat4 rotation;
    vec3 scale;

    glm_decompose_rs(items[item].model, rotation, scale);
    glm_mat4_identity(items[item].model);
    glm_translate(items[item].model, (vec3){position[0], position[1], 0});
    glm_scale(items[item].model, (vec3){scale[0], scale[1], 1});
}

void UIItem_get_size(const UIItem item, vec2 size) {
    vec3 scale;
    glm_decompose_scalev(items[item].model, scale);
    size[0] = scale[0];
    size[1] = scale[1];
}

void UIItem_get_position(const UIItem item, vec2 position) {
    vec3 scale;
    vec4 pos;
    mat4 rotation;
    glm_decompose(items[item].model, pos, rotation, scale);
    position[0] = pos[0];
    position[1] = pos[1];
}

void UIItem_draw(const UIItem item) {
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D_ARRAY, VoxelEngine_get_atlas_id());
    shader_use(sm_get_shader(SHADER_UI_3D));
    mat4 ortho;
    UIManager_get_ortho_matrix(ortho);
    shader_set_mat4(sm_get_shader(SHADER_UI_3D), "ortho", &ortho);

    shader_set_int(sm_get_shader(SHADER_UI_3D), "atlas", 0);
    shader_set_mat4(sm_get_shader(SHADER_UI_3D), "model", &items[item].model);

    shader_set_int(sm_get_shader(SHADER_UI_3D), "atlasIndex", g_blockData[items[item].type].sideTextures[0]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
