#include "ui/UIInventory.h"

#include <stdio.h>

#include "managers/InputManager.h"
#include "Inventory.h"
#include "Settings.h"
#include "../../include/ui/UIStack.h"
#include "managers/WindowManager.h"
#include "ui/BlockStack.h"
#include "ui/UIHotbar.h"
#include "ui/UISprite.h"
#include "ui/UIText.h"

static constexpr vec2 slotBackgroundSize = {100, 100};
static constexpr vec2 slotSpriteSize = {90, 90};
static constexpr vec2 slotBackgroundSpacerSize = {6, 6};

static UISprite background;
static UIStack slotSprites[NUM_SLOTS];
static UISprite slotBackgrounds[NUM_SLOTS];
static UIStack itemPickedUp;
static bool isPickingUp = false;
static BlockStack stackPickedUp;
static bool enabled;

int check_hovered_slot() {
    vec2 mousePos;
    im_get_mouse_position(mousePos);
    for (int i = 0; i < NUM_SLOTS; i++) {
        vec2 spritePos;
        vec2 spriteSize;
        UIStack_get_size(slotSprites[i], spriteSize);
        UIStack_get_position(slotSprites[i], spritePos);
        if (mousePos[0] > spritePos[0] - spriteSize[0] / 2 &&
            mousePos[0] < spritePos[0] + spriteSize[0] / 2 &&
            mousePos[1] > spritePos[1] - spriteSize[1] / 2 &&
            mousePos[1] < spritePos[1] + spriteSize[1] / 2) {
            return i;
        }
    }
    return -1;
}

void UIInventory_init() {
    inventory_init();
    im_register_key(GLFW_KEY_Q);
    vec2 screenSize = {Settings.window.width, Settings.window.height};

    background = UISprite_init("assets/ui/inventory_bg.png", (vec2){screenSize[0] / 2, screenSize[1] / 2}, (vec2){
                      (slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) * NUM_SLOTS_X,
                      (slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) * NUM_SLOTS_Y
                  });
    itemPickedUp = UIStack_init(nullptr, 0, (vec2){0, 0}, (vec2){90, 90});
    for (int y = 0; y < NUM_SLOTS_Y; y++) {
        for (int x = 0; x < NUM_SLOTS_X; x++) {
            slotSprites[y * NUM_SLOTS_X + x] = UIStack_init(
                                 blocktype_to_texture_path(inventory_get_stack_from_slot(x, y).type), 0,
                                 (vec2){
                                     screenSize[0] / 2 + (x - NUM_SLOTS_X / 2.f) * (
                                         slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) + (
                                         slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) / 2,
                                     screenSize[1] / 2 - (y - NUM_SLOTS_Y / 2.f) * (
                                         slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) - (
                                         slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) / 2
                                 },
                                 (vec2){90, 90});
            slotBackgrounds[y * NUM_SLOTS_X + x] = UISprite_init("assets/ui/hotbar_bg.png", (vec2){
                              screenSize[0] / 2 + (x - NUM_SLOTS_X / 2.f) * (
                                  slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) + (
                                  slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) / 2,
                              screenSize[1] / 2 - (y - NUM_SLOTS_Y / 2.f) * (
                                  slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) - (
                                  slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) / 2
                          },
                          slotBackgroundSize);
        }
    }
}

void UIInventory_draw() {
    if (!enabled) return;

    UISprite_draw(background);
    for (int i = 0; i < NUM_SLOTS_X * NUM_SLOTS_Y; i++) {
        UISprite_draw(slotBackgrounds[i]);
        if (inventory_get_stack_from_slot(i % NUM_SLOTS_X, i / NUM_SLOTS_X).type != 0)
            UIStack_draw(slotSprites[i]);
    }
    if (isPickingUp)
        UIStack_draw(itemPickedUp);
}

void UIInventory_update() {
    if (!enabled) return;
    if (isPickingUp) {
        vec2 mousePos;
        im_get_mouse_position(mousePos);
        UIStack_set_position(itemPickedUp, mousePos);
    }

    if (im_get_key_down(GLFW_KEY_Q)) {
        int hovered = check_hovered_slot();
        if (hovered != -1) {
            inventory_set_stack_in_slot(hovered % NUM_SLOTS_X, hovered / NUM_SLOTS_Y, STACK_EMPTY);
        }
    }

    if (im_get_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)) {
        int hovered = check_hovered_slot();
        if (hovered != -1) {
            BlockStack stack = inventory_get_stack_from_slot(hovered % NUM_SLOTS_X, hovered / NUM_SLOTS_X);
            if (!isPickingUp && stack.type != 0) {
                isPickingUp = true;
                stackPickedUp = stack;
                UIStack_set_stack(itemPickedUp, stack);
                inventory_set_stack_in_slot(hovered % NUM_SLOTS_X, hovered / NUM_SLOTS_X, STACK_EMPTY);
                if (hovered / NUM_SLOTS_X == 0)
                    UIHotbar_reload_slot(hovered % 9, STACK_EMPTY);
            } else if (isPickingUp) {
                isPickingUp = stack.type != 0;
                UIStack_set_stack(slotSprites[hovered], stack);
                //UIStack_set_texture(slotSprites[hovered], blocktype_to_texture_path(blockPickedUp.type));
                inventory_set_stack_in_slot(hovered % NUM_SLOTS_X, hovered / NUM_SLOTS_X, stackPickedUp);
                UIStack_set_stack(itemPickedUp, stack);
                stackPickedUp = stack;
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
    } else {
        glfwSetInputMode(window_get_handler(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void UIInventory_reload_slot(int x, int y, BlockStack stack) {
    if (y == 0) {
        UIHotbar_reload_slot(x, stack);
    }

    UIStack_set_stack(slotSprites[y * NUM_SLOTS_X + x], stack);
}
