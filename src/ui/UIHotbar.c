#include "ui/UIHotbar.h"

#include "Inventory.h"
#include "ui/UISprite.h"

static UISprite sprites[9];
static UISprite selector;
static UISprite background;
static int currentIndex = 0;

void UIHotbar_init() {
    for (int i = 0; i < 9; i++) {
        UISprite_init(&(sprites[i]), blocktype_to_texture_path(inventory_get_stack_from_slot(i, 0).type), (vec2){960 + (i - 4) * 100, 980},(vec2){90, 90}, true);
    }
    UISprite_init(&background, "assets/ui/hotbar_bg.png", (vec2){960, 980}, (vec2) {900, 100}, true);
    UISprite_init(&selector, "assets/ui/hotbar_selector.png", (vec2){960 - 4 * 100, 980}, (vec2){100, 100}, true);
}

int UIHotbar_get_current_index() {
    return currentIndex;
}

void UIHotbar_set_slot_item_texture(int slotIndex, const char *texture) {
    UISprite_set_texture(&(sprites[slotIndex]), texture);
}

BlockType UIHotbar_move_selector_to_slot(int slotSelected) {
    if (slotSelected >= 9) {
        slotSelected = 0;
    } else if (slotSelected < 0) {
        slotSelected = 8;
    }

    currentIndex = slotSelected;

    UISprite_set_position(&selector, (vec2){960 + (currentIndex - 4) * 100, 980});
    return inventory_get_stack_from_slot(currentIndex, 0).type;
}

void UIHotbar_draw() {
    UISprite_draw(&background);
    for (int i = 0; i < 9; i++) {
        if (inventory_get_stack_from_slot(i, 0).type != 0)
            UISprite_draw(&(sprites[i]));
    }
    UISprite_draw(&selector);
}
