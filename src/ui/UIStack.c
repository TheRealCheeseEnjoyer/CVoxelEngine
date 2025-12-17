#include "ui/UIStack.h"
#include "ui/UIText.h"

#include <stdio.h>

#include "UIItem.h"
#include "CVector/Vector.h"

typedef struct {
    UIItem uiItem;
    UIText amountText;
    int amount;
} uiinventoryslot_t;

static uiinventoryslot_t* slots = nullptr;

UIStack UIStack_init(BlockId type, int amount, vec2 position, vec2 size) {
    if (slots == nullptr) {
        slots = vec_init(sizeof(uiinventoryslot_t));
    }
    uiinventoryslot_t slot;
    slot.uiItem = UIItem_init(type, position, size);
    char amountStr[10];
    sprintf(amountStr, "%d", amount);
    slot.amountText = UIText_init(amountStr, (vec2) {position[0] + 0.2 * size[0], position[1] + 0.7f * size[1]}, false);
    slot.amount = amount;
    vec_append(&slots, &slot);
    return vec_size(slots) - 1;
}

void UIStack_set_amount(UIStack slotIndex, int amount) {
    char amountStr[10];
    sprintf(amountStr, "%d", amount);
    UIText_set_text(slots[slotIndex].amountText, amountStr);
}

void UIStack_set_stack(UIStack slotIndex, BlockStack stack) {
    UIItem_set_type(slots[slotIndex].uiItem, stack.type);
    char amountStr[10];
    sprintf(amountStr, "%d", stack.size);
    UIText_set_text(slots[slotIndex].amountText, amountStr);
    if (stack.size == stack.maxSize)
        UIText_set_color(slots[slotIndex].amountText, (vec3){255, 0, 0});
    else
        UIText_set_color(slots[slotIndex].amountText, (vec3){0, 0, 0});
}

void UIStack_set_type(UIStack slotIndex, BlockId type) {
    UIItem_set_type(slots[slotIndex].uiItem, type);
}

void UIStack_set_position(UIStack slotIndex, vec2 position) {
    UIItem_set_position(slots[slotIndex].uiItem, position);
    vec2 size;
    UIItem_get_size(slots[slotIndex].uiItem, size);
    UIText_set_position(slots[slotIndex].amountText, (vec2){position[0] + size[0] / 4, position[1] + 0.7f * size[1]});
}

void UIStack_get_position(UIStack slotIndex, vec2 position) {
    UIItem_get_position(slots[slotIndex].uiItem, position);
}

void UIStack_get_size(UIStack slotIndex, vec2 size) {
    UIItem_get_size(slots[slotIndex].uiItem, size);
}

void UIStack_draw(UIStack slotIndex) {
    UIItem_draw(slots[slotIndex].uiItem);
    UIText_draw();
}
