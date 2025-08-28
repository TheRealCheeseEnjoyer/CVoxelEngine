#ifndef UIMANAGER_H
#define UIMANAGER_H
#include "UISprite.h"

void UIManager_init();

void UIManager_register_interactable(UISprite* sprite);
int UIManager_check_hovered(UISprite* sprite, int size);

void UIManager_begin_draw();

void UIManager_end_draw();

void UIManager_destroy();

#endif
