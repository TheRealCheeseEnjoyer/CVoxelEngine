#include "ui/UIInventory.h"

#include <stdio.h>

#include "managers/InputManager.h"
#include "Inventory.h"
#include "assets/ui/UIInventorySlot.h"
#include "managers/WindowManager.h"
#include "ui/BlockStack.h"
#include "ui/UIHotbar.h"
#include "ui/UISprite.h"
#include "ui/UIText.h"

static constexpr vec2 slotBackgroundSize = {100, 100};
static constexpr vec2 slotSpriteSize = {90, 90};
static constexpr vec2 slotBackgroundSpacerSize = {6, 6};

static UISprite background;
static UIInventorySlot slotSprites[NUM_SLOTS];
static UISprite slotBackgrounds[NUM_SLOTS];
static UISprite itemPickedUp;
static bool isPickingUp = false;
static BlockStack blockPickedUp;
static bool enabled;

int check_hovered_slot() {
    vec2 mousePos;
    im_get_mouse_position(mousePos);
    for (int i = 0; i < NUM_SLOTS; i++) {
        vec2 spritePos;
        vec2 spriteSize;
        UIInventorySlot_get_size(&slotSprites[i], spriteSize);
        UIInventorySlot_get_position(&slotSprites[i], spritePos);
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
    vec2 screenSize;
    window_get_size(screenSize);
    background = UISprite_init("assets/ui/inventory_bg.png", (vec2){screenSize[0] / 2, screenSize[1] / 2}, (vec2){
                      (slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) * NUM_SLOTS_X,
                      (slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) * NUM_SLOTS_Y
                  });
    itemPickedUp = UISprite_init(nullptr, (vec2){0, 0}, (vec2){90, 90});
    for (int y = 0; y < NUM_SLOTS_Y; y++) {
        for (int x = 0; x < NUM_SLOTS_X; x++) {
            UIInventorySlot_init(&slotSprites[y * NUM_SLOTS_X + x],
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
            UIInventorySlot_draw(&slotSprites[i]);
    }
    if (isPickingUp)
        UISprite_draw(itemPickedUp);
}

void UIInventory_update() {
    if (!enabled) return;
    if (isPickingUp) {
        vec2 mousePos;
        im_get_mouse_position(mousePos);
        UISprite_set_position(itemPickedUp, mousePos);
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
                blockPickedUp = stack;
                UISprite_set_texture(itemPickedUp, blocktype_to_texture_path(blockPickedUp.type));
                inventory_set_stack_in_slot(hovered % NUM_SLOTS_X, hovered / NUM_SLOTS_X, STACK_EMPTY);
                if (hovered / NUM_SLOTS_X == 0)
                    UIHotbar_reload_slot(hovered % 9, nullptr, 0);
            } else if (isPickingUp) {
                isPickingUp = stack.type != 0;
                UIInventorySlot_set_texture(&slotSprites[hovered], blocktype_to_texture_path(blockPickedUp.type));
                inventory_set_stack_in_slot(hovered % NUM_SLOTS_X, hovered / NUM_SLOTS_X, blockPickedUp);
                UISprite_set_texture(itemPickedUp, blocktype_to_texture_path(stack.type));
                blockPickedUp = stack;
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

void UIInventory_reload_slot(int x, int y, BlockType type, int amount) {
    if (y == 0) {
        UIHotbar_reload_slot(x, blocktype_to_texture_path(type), amount);
    }

    UIInventorySlot_set_texture(&slotSprites[y * NUM_SLOTS_X + x], blocktype_to_texture_path(type));
    UIInventorySlot_set_amount(&slotSprites[y * NUM_SLOTS_X + x], amount);
}
