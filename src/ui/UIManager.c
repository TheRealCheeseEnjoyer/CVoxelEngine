#include "ui/UIManager.h"
#include <cglm/cam.h>
#include <glad/glad.h>

#include "ShaderManager.h"

static mat4 orthoMatrix;
static Shader shader;
static unsigned int VAO, vertexBuffer;

static float vertices[] = {
    0, 0,
    1, 1,
    0, 1,

    0, 0,
    1, 0,
    1, 1
};


void UIManager_init() {
    glm_ortho(0.f, 1920.f, 0.f, 1080.f, -1.f, 1.f, orthoMatrix);
    shader = sm_get_shader(SHADER_UI);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
}

void UIManager_begin_draw() {
    shader_use(shader);
    shader_set_mat4(shader, "ortho", &orthoMatrix);
    shader_set_int(shader, "TextureUnitId", 0);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(VAO);
}

void UIManager_end_draw() {
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

void UIManager_destroy() {
    glDeleteVertexArrays(1, &vertexBuffer);
    glDeleteVertexArrays(1, &VAO);
}
