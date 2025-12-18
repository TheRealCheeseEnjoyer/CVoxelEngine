#include "ui/UIInventory.h"

#include "managers/InputManager.h"
#include "Inventory.h"
#include "Settings.h"
#include "../../include/ui/UIStack.h"
#include "managers/WindowManager.h"
#include "ui/BlockStack.h"
#include "ui/UIHotbar.h"
#include "ui/UISprite.h"
#include "ui/UIText.h"
#include "VoxelEngine/VoxelEngine.h"

static constexpr vec2 slotBackgroundSize = {100, 100};
static constexpr vec2 slotSpriteSize = {90, 90};
static constexpr vec2 slotBackgroundSpacerSize = {6, 6};

static UISprite background;
static UIStack stacks[NUM_SLOTS];
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
        UIStack_get_size(stacks[i], spriteSize);
        UIStack_get_position(stacks[i], spritePos);
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
                               }, false);

    itemPickedUp = UIStack_init(0, 0, (vec2){0, 0}, (vec2){90, 90}, false);
    for (int y = 0; y < NUM_SLOTS_Y; y++) {
        for (int x = 0; x < NUM_SLOTS_X; x++) {
            stacks[y * NUM_SLOTS_X + x] = UIStack_init(
                inventory_get_stack_from_slot(x, y).type,
                0,
                (vec2){
                    screenSize[0] / 2 + (x - NUM_SLOTS_X / 2.f) * (
                        slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) + (
                        slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) / 2,
                    screenSize[1] / 2 - (y - NUM_SLOTS_Y / 2.f) * (
                        slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) - (
                        slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) / 2
                },
                (vec2){90, 90}, false);
            slotBackgrounds[y * NUM_SLOTS_X + x] = UISprite_init("assets/ui/hotbar_bg.png", (vec2){
                                                                     screenSize[0] / 2 + (x - NUM_SLOTS_X / 2.f) * (
                                                                         slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) + (
                                                                         slotBackgroundSize[0] + slotBackgroundSpacerSize[0]) / 2,
                                                                     screenSize[1] / 2 - (y - NUM_SLOTS_Y / 2.f) * (
                                                                         slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) - (
                                                                         slotBackgroundSize[1] + slotBackgroundSpacerSize[1]) / 2
                                                                 },
                                                                 slotBackgroundSize, false);
        }
    }
}

void UIInventory_draw() {
    if (!enabled) return;

    for (int i = 0; i < NUM_SLOTS_X * NUM_SLOTS_Y; i++) {
        if (inventory_get_stack_from_slot(i % NUM_SLOTS_X, i / NUM_SLOTS_X).type != 0)
            UIStack_draw(stacks[i]);
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
                UIStack_set_enabled(stacks[hovered], false);
                inventory_set_stack_in_slot(hovered % NUM_SLOTS_X, hovered / NUM_SLOTS_X, STACK_EMPTY);
                if (hovered / NUM_SLOTS_X == 0)
                    UIHotbar_reload_slot(hovered % 9, STACK_EMPTY);
            } else if (isPickingUp) {
                isPickingUp = stack.type != 0;
                UIStack_set_enabled(stacks[hovered], true);
                UIStack_set_stack(stacks[hovered], stack);
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

    UISprite_set_enabled(background, enabled);
    UIStack_set_enabled(itemPickedUp, enabled);
    for (int i = 0; i < NUM_SLOTS_X * NUM_SLOTS_Y; i++) {
        UISprite_set_enabled(slotBackgrounds[i], enabled);
        if (inventory_get_stack_from_slot(i % NUM_SLOTS_X, i / NUM_SLOTS_X).size != 0)
            UIStack_set_enabled(stacks[i], enabled);
    }

    glfwSetInputMode(window_get_handler(), GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void UIInventory_reload_slot(int x, int y, BlockStack stack) {
    if (y == 0) {
        UIHotbar_reload_slot(x, stack);
    }

    UIStack_set_stack(stacks[y * NUM_SLOTS_X + x], stack);
}
