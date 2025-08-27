#include "../include/Hotbar.h"
#include "../include/UISprite.h"

UISprite sprites[9];
UISprite selector;
BlockType slots[9];
int currentIndex = 0;

void Hotbar_init() {
    for (int i = 0; i < 9; i++) {
        UISprite_init(&(sprites[i]), blocktype_to_texture_path(i % BLOCK_NUM_BLOCK_TYPES), (vec2){100 + i * 100, 100},
                      (vec2){100, 100});
        slots[i] = i % BLOCK_NUM_BLOCK_TYPES;
    }
    UISprite_init(&selector, "assets/ui/hotbar_selector.png", (vec2){100, 100}, (vec2){100, 100});
}

int Hotbar_get_current_index() {
    return currentIndex;
}

BlockType Hotbar_change_selection(int slotSelected) {
    if (slotSelected >= 9) {
        slotSelected = 0;
    } else if (slotSelected < 0) {
        slotSelected = 8;
    }
    currentIndex = slotSelected;

    UISprite_set_position(&selector, (vec2){100 + slotSelected * 100, 100});
    return slots[slotSelected];
}

void Hotbar_draw() {
    for (int i = 0; i < 9; i++) {
        UISprite_draw(&(sprites[i]));
    }
    UISprite_draw(&selector);
}
