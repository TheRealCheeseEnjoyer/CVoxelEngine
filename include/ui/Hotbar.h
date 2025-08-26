#ifndef HOTBAR_H
#define HOTBAR_H
#include "UISprite.h"

UISprite sprites[9];
UISprite selector;

void Hotbar_init() {
    for (int i = 0; i < 9; i++) {
        UISprite_init(&(sprites[i]), blocktype_to_texture_path(i % BLOCK_NUM_BLOCK_TYPES), (vec2) {100 + i * 100, 100}, (vec2) {100, 100});
    }
    UISprite_init(&selector, "assets/ui/hotbar_selector.png", (vec2) {100, 100}, (vec2) {100, 100});
}

void Hotbar_change_selection(int slotSelected) {
    if (slotSelected > 9) {
        slotSelected = 0;
    } else if (slotSelected < 0) {
        slotSelected = 9;
    }

    UISprite_set_position(&selector, (vec2) {100 + slotSelected * 100, 100});
}

void Hotbar_draw() {
    for (int i = 0; i < 9; i++) {
        UISprite_draw(&(sprites[i]));
    }
    UISprite_draw(&selector);
}

#endif
