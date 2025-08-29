#include "ui/UIInventory.h"

#include <stdio.h>

#include "BlockType.h"
#include "InputManager.h"
#include "Inventory.h"
#include "WindowManager.h"
#include "ui/UIHotbar.h"
#include "ui/UIManager.h"
#include "ui/UISprite.h"

static constexpr vec2 slotBackgroundSize = {100, 100};
static constexpr vec2 slotSpriteSize = {90, 90};
static constexpr vec2 slotBackgroundSpacerSize = {6, 6};

static UISprite background;
static UISprite slotSprites[NUM_SLOTS_X * NUM_SLOTS_Y];
static UISprite slotBackgrounds[NUM_SLOTS_X * NUM_SLOTS_Y];
static UISprite pickedUp;
static bool isPickingUp = false;
static BlockType blockPickedUp = 0;
static bool enabled;


void UIInventory_init() {
    inventory_init();
    UISprite_init(&background, "assets/ui/inventory_bg.png", (vec2){960, 540}, (vec2){
        (slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) * NUM_SLOTS_X,
        (slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) * NUM_SLOTS_Y},
        false);
    UISprite_init(&pickedUp, nullptr, (vec2) {0, 0}, (vec2) {90, 90}, false);
    for (int y = 0; y < NUM_SLOTS_Y; y++) {
        for (int x = 0; x < NUM_SLOTS_X; x++) {
            UISprite_init(&slotSprites[y * NUM_SLOTS_X + x], blocktype_to_texture_path(inventory_get_block_from_slot(x, y)),
                (vec2) {
                    960 + (x - NUM_SLOTS_X / 2.f) * (slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) + (slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) / 2,
                    540 - (y - NUM_SLOTS_Y / 2.f) * (slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) - (slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) / 2},
                    (vec2) {90, 90}, false);
            UISprite_init(&slotBackgrounds[y * NUM_SLOTS_X + x], "assets/ui/hotbar_bg.png",
                (vec2) {
                    960 + (x - NUM_SLOTS_X / 2.f) * (slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) + (slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) / 2,
                    540 - (y - NUM_SLOTS_Y / 2.f) * (slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) - (slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) / 2},
                slotBackgroundSize, false);
        }
    }
}

void UIInventory_draw() {
    if (!enabled) return;

    UISprite_draw(&background);
    for (int i = 0; i < NUM_SLOTS_X * NUM_SLOTS_Y; i++) {
        UISprite_draw(&slotBackgrounds[i]);
        if (inventory_get_block_from_slot(i % 9, i / 9) != 0)
            UISprite_draw(&slotSprites[i]);
    }
    UISprite_draw(&pickedUp);
}

void UIInventory_update() {
    if (!enabled) return;
    if (isPickingUp) {
        vec2 mousePos;
        im_get_mouse_position(mousePos);
        UISprite_set_position(&pickedUp, mousePos);
    }

    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
        int hovered = UIManager_check_hovered(slotSprites, NUM_SLOTS);
        if (hovered != -1) {
            BlockType block = inventory_get_block_from_slot(hovered % 9, hovered / 9);
            if (!isPickingUp && block != 0) {
                isPickingUp = true;
                blockPickedUp = block;
                UISprite_set_enabled(&pickedUp, true);
                UISprite_set_texture(&pickedUp, blocktype_to_texture_path(block));
                inventory_set_block_in_slot(hovered % 9, hovered / 9, 0);
                if (hovered / NUM_SLOTS_X == 0)
                    UIHotbar_set_slot_item_texture(hovered % 9, nullptr);
            } else if (isPickingUp && block == 0) {
                isPickingUp = false;
                UISprite_set_enabled(&pickedUp, false);
                UISprite_set_texture(&slotSprites[hovered], blocktype_to_texture_path(blockPickedUp));
                inventory_set_block_in_slot(hovered % 9, hovered / 9, blockPickedUp);
                if (hovered / NUM_SLOTS_X == 0)
                    UIHotbar_set_slot_item_texture(hovered % 9, blocktype_to_texture_path(blockPickedUp));
                blockPickedUp = 0;
            }
        }
    }
}


bool UIInventory_is_enabled() {
    return enabled;
}

void UIInventory_toggle() {
    enabled = !enabled;
    if (enabled) {
        glfwSetInputMode(window_get_handler(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        UISprite_set_enabled(&background, true);
        for (int i = 0; i < NUM_SLOTS; i++) {
            UISprite_set_enabled(&slotBackgrounds[i], true);
            UISprite_set_enabled(&slotSprites[i], true);
        }
    }
    else {
        glfwSetInputMode(window_get_handler(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        UISprite_set_enabled(&background, false);
        for (int i = 0; i < NUM_SLOTS; i++) {
            UISprite_set_enabled(&slotBackgrounds[i], false);
            UISprite_set_enabled(&slotSprites[i], false);
        }
    }

}
