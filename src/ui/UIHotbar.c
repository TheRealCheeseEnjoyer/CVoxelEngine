#include "ui/UIHotbar.h"

#include "Inventory.h"
#include "Settings.h"
#include "ui/UIStack.h"
#include "ui/UISprite.h"

static UIStack sprites[NUM_SLOTS_X];
static UISprite selector;
static UISprite background;
static int currentIndex = 0;

void UIHotbar_init() {
    ivec2 screenSize = {
        Settings.window.width,
        Settings.window.height
    };
    for (int i = 0; i < NUM_SLOTS_X; i++) {
        BlockStack stack = inventory_get_stack_from_slot(i, 0);
        sprites[i] = UIStack_init(stack.type, stack.size, (vec2){screenSize[0] / 2 + (i - 4) * 100, screenSize[1] - 100},(vec2){90, 90});
    }
    background = UISprite_init("assets/ui/hotbar_bg.png", (vec2){screenSize[0] / 2, screenSize[1] - 100}, (vec2) {900, 100});
    selector = UISprite_init("assets/ui/hotbar_selector.png", (vec2){screenSize[0] / 2 - 4 * 100, screenSize[1] - 100}, (vec2){100, 100});
}

int UIHotbar_get_current_index() {
    return currentIndex;
}

void UIHotbar_reload_slot(int slotIndex, BlockStack stack) {
    UIStack_set_stack(sprites[slotIndex], stack);
}

BlockType UIHotbar_move_selector_to_slot(int slotSelected) {
    if (slotSelected >= 9) {
        slotSelected = 0;
    } else if (slotSelected < 0) {
        slotSelected = 8;
    }

    currentIndex = slotSelected;
    vec2 screenSize = {Settings.window.width, Settings.window.height};

    UISprite_set_position(selector, (vec2){screenSize[0] / 2 + (currentIndex - 4) * 100, screenSize[1] - 100});
    return inventory_get_stack_from_slot(currentIndex, 0).type;
}

void UIHotbar_draw() {
    UISprite_draw(background);
    for (int i = 0; i < 9; i++) {
        if (inventory_get_stack_from_slot(i, 0).type != 0)
            UIStack_draw(sprites[i]);
    }
    UISprite_draw(selector);
}
