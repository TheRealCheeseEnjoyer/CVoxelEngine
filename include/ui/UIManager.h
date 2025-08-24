#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <cglm/cam.h>
#include <glad/glad.h>

#include "../ShaderManager.h"
#include "../TextureManager.h"
#include "UISprite.h"

mat4 orthoMatrix;
static Shader shader;
static unsigned int VAO, vertexBuffer;
UISprite sprite;

void UIManager_init() {
    glm_ortho(0.f, 1920.f, 0.f, 1080.f, -1.f, 1.f, orthoMatrix);
    shader = sm_get_shader(SHADER_UI);
    UISprite_init(&sprite, BLOCK_ROCK, (vec2) {100, 100}, (vec2) {100, 100});
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
}

void UIManager_draw() {
    shader_use(shader);
    mat4 final;
    //glm_mat4_mul(sprite.transform, orthoMatrix, final);
    shader_set_mat4(shader, "ortho", &orthoMatrix);
    shader_set_mat4(shader, "model", &sprite.transform);
    shader_set_int(shader, "TextureUnitId", 0);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, tm_get_texture_id(sprite.type));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

#endif
