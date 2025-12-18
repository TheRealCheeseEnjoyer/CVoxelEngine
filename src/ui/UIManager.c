#include "ui/UIManager.h"

#include <string.h>
#include <cglm/cam.h>
#include <glad/glad.h>

#include "Settings.h"
#include "UIItem.h"
#include "../../include/managers/InputManager.h"
#include "ui/UIText.h"

static mat4 orthoMatrix;
static int activeTextureIndex = 1;

void UIManager_init() {
    glm_ortho(0.f, Settings.window.width, Settings.window.height, 0.f, -1.f, 1.f, orthoMatrix);
}

void UIManager_get_ortho_matrix(mat4 outMat) {
    memcpy(outMat, orthoMatrix, sizeof(mat4));
}

int UIManager_get_active_texture_id() {
    return activeTextureIndex;
}

int UIManager_check_hovered(UISprite *sprite, int size) {
    vec2 mousePos;
    im_get_mouse_position(mousePos);
    for (int i = 0; i < size; i++) {
        vec2 spritePos;
        vec2 spriteSize;
        UISprite_get_size(sprite[i], spriteSize);
        UISprite_get_position(sprite[i], spritePos);
        if (mousePos[0] > spritePos[0] - spriteSize[0] / 2 &&
            mousePos[0] < spritePos[0] + spriteSize[0] / 2 &&
            mousePos[1] > spritePos[1] - spriteSize[1] / 2 &&
            mousePos[1] < spritePos[1] + spriteSize[1] / 2) {
            return i;
        }
    }

    return -1;
}

void UIManager_draw() {
    glDisable(GL_DEPTH_TEST);

    UISprite_draw();
    UIItem_draw();
    UIText_draw();

    glEnable(GL_DEPTH_TEST);
}