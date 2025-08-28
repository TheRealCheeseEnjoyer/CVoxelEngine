#include "ui/Inventory.h"

#include <stdio.h>

#include "BlockType.h"
#include "InputManager.h"
#include "WindowManager.h"
#include "ui/UIManager.h"
#include "ui/UISprite.h"

static UISprite background;
static UISprite slotSprites[27];
static UISprite slotBackgrounds[27];
static UISprite pickedUp;
static bool isPickingUp = false;
static BlockType blockPickedUp = 0;
static BlockType blocks[27];
static bool enabled;


void inventory_init() {
    UISprite_init(&background, "assets/ui/inventory_bg.png", (vec2){960, 540}, (vec2){950, 320}, false);
    UISprite_init(&pickedUp, nullptr, (vec2) {0, 0}, (vec2) {90, 90}, false);
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 9; x++) {
            blocks[y * 9 + x] = 0;
            UISprite_init(&slotSprites[y * 9 + x], nullptr, (vec2) {960 + (x - 4) * 105, 540 + (y - 1) * 105}, (vec2) {90, 90}, false);
            UISprite_init(&slotBackgrounds[y * 9 + x], "assets/ui/hotbar_bg.png", (vec2) {960 + (x - 4) * 105, 540 + (y - 1) * 105}, (vec2) {100, 100}, false);
        }
    }

    blocks[0] = BLOCK_GRASS;
    UISprite_set_texture(&slotSprites[0], "assets/grass.png");
}

void inventory_draw() {
    if (!enabled) return;

    UIManager_begin_draw();
    UISprite_draw(&background);
    for (int i = 0; i < 27; i++) {
        UISprite_draw(&slotBackgrounds[i]);
        if (blocks[i] != 0)
            UISprite_draw(&slotSprites[i]);
    }
    UISprite_draw(&pickedUp);
    UIManager_end_draw();
}

void inventory_update() {
    if (!enabled) return;
    if (isPickingUp) {
        vec2 pos;
        im_get_mouse_position(pos);
        UISprite_set_position(&pickedUp, pos);
    }
    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
        int hoveredIndex = UIManager_check_hovered(slotBackgrounds, 27);
        if (hoveredIndex != -1) {
            if (!isPickingUp && blocks[hoveredIndex] != 0) {
                isPickingUp = true;
                blockPickedUp = blocks[hoveredIndex];
                UISprite_set_enabled(&slotSprites[hoveredIndex], false);
                UISprite_set_texture(&pickedUp, blocktype_to_texture_path(blocks[hoveredIndex]));
                UISprite_set_enabled(&pickedUp, true);
                blocks[hoveredIndex] = 0;
            } else if (isPickingUp && blocks[hoveredIndex] == 0) {
                isPickingUp = false;
                UISprite_set_texture(&slotSprites[hoveredIndex], blocktype_to_texture_path(blockPickedUp));
                UISprite_set_enabled(&slotSprites[hoveredIndex], true);
                UISprite_set_enabled(&pickedUp, false);
                blocks[hoveredIndex] = blockPickedUp;
            }
        }
    }
}


bool inventory_is_enabled() {
    return enabled;
}

void inventory_toggle() {
    enabled = !enabled;
    if (enabled) {
        glfwSetInputMode(window_get_handler(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        UISprite_set_enabled(&background, true);
        for (int i = 0; i < 27; i++) {
            UISprite_set_enabled(&slotBackgrounds[i], true);
            UISprite_set_enabled(&slotSprites[i], true);
        }
    }
    else {
        glfwSetInputMode(window_get_handler(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        UISprite_set_enabled(&background, false);
        for (int i = 0; i < 27; i++) {
            UISprite_set_enabled(&slotBackgrounds[i], false);
            UISprite_set_enabled(&slotSprites[i], false);
        }
    }

}
