#include "ui/Hotbar.h"
#include "ui/UISprite.h"

UISprite sprites[9];
UISprite selector;
UISprite background;
BlockType slots[9];
int currentIndex = 0;

void Hotbar_init() {
    for (int i = 0; i < 9; i++) {
        UISprite_init(&(sprites[i]), blocktype_to_texture_path(i % BLOCK_NUM_BLOCK_TYPES), (vec2){960 + (i - 4) * 100, 980},
                      (vec2){90, 90}, true);
        slots[i] = i % BLOCK_NUM_BLOCK_TYPES;
    }
    UISprite_init(&background, "assets/ui/hotbar_bg.png", (vec2){960, 980}, (vec2) {900, 100}, true);
    UISprite_init(&selector, "assets/ui/hotbar_selector.png", (vec2){960 - 4 * 100, 980}, (vec2){100, 100}, true);
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

    UISprite_set_position(&selector, (vec2){960 + (slotSelected - 4) * 100, 980});
    return slots[slotSelected];
}

void Hotbar_draw() {
    UISprite_draw(&background);
    for (int i = 0; i < 9; i++) {
        if (slots[i] != 0)
            UISprite_draw(&(sprites[i]));
    }
    UISprite_draw(&selector);
}
