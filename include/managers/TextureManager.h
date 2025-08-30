#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include "../BlockType.h"

void tm_init();

unsigned int tm_get_texture_id(const char* texture);

void tm_destroy();
#endif
