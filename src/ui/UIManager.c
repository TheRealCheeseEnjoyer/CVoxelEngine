#include "ui/UIManager.h"

#include <stdio.h>
#include <cglm/cam.h>
#include <glad/glad.h>

#include "Settings.h"
#include "../../include/managers/InputManager.h"
#include "../../include/managers/ShaderManager.h"
#include "managers/GlyphManager.h"
#include "managers/SettingsManager.h"
#include "managers/WindowManager.h"
#include "ui/UIInventory.h"

static mat4 orthoMatrix;
static Shader shader;
static unsigned int VAO, vertexBuffer;

static float vertices[] = {
    0, 1,
    1, 1,
    0, 0,

    1, 0,
    0, 0,
    1, 1
};


void UIManager_init() {
    vec2 screenSize;
    window_get_size(screenSize);
    glm_ortho(0.f, screenSize[0], screenSize[1], 0.f, -1.f, 1.f, orthoMatrix);
    shader = sm_get_shader(SHADER_UI);
    glyph_manager_init_glyphs("assets/fonts/COMIC.ttf");
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
}

int UIManager_check_hovered(UISprite *sprite, int size) {
    vec2 mousePos;
    im_get_mouse_position(mousePos);
    for (int i = 0; i < size; i++) {
        if (sprite[i].enabled) {
            vec2 spritePos;
            vec2 spriteSize;
            UISprite_get_size(&sprite[i], spriteSize);
            UISprite_get_position(&sprite[i], spritePos);
            if (mousePos[0] > spritePos[0] - spriteSize[0] / 2 &&
                mousePos[0] < spritePos[0] + spriteSize[0] / 2 &&
                mousePos[1] > spritePos[1] - spriteSize[1] / 2 &&
                mousePos[1] < spritePos[1] + spriteSize[1] / 2) {
                return i;
            }
        }
    }
    return -1;
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
