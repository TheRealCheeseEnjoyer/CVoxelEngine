#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <cglm/cam.h>
#include <glad/glad.h>

#include "../ShaderManager.h"
#include "../TextureManager.h"
#include "UISprite.h"

mat4 orthoMatrix;
static Shader shader;
static unsigned int VAO, vertexBuffer, textureCoodsBuffer, EBO;
UISprite sprite;

void UIManager_init() {
    glm_ortho(0.f, 1920.f, 0.f, 1080.f, 0.1f, 100.f, orthoMatrix);
    shader = sm_get_shader(SHADER_UI);
    UISprite_init(&sprite, BLOCK_ROCK, (vec2) {1, 1}, (vec2) {200, 200});
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &textureCoodsBuffer);
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sprite.vertices), sprite.vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, textureCoodsBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indeces), indeces, GL_STATIC_DRAW);
}

void UIManager_draw() {
    shader_use(shader);
    mat4 final;
    glm_mat4_mul(sprite.transform, orthoMatrix, final);
    shader_set_mat4(shader, "finalMatrix", &final);
    shader_set_int(shader, "TextureUnitId", 0);
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, tm_get_texture_id(sprite.type));
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, textureCoodsBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

#endif
