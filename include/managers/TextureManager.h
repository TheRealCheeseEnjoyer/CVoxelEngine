#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

void tm_init();

unsigned int tm_get_texture_id(const char* texture);
unsigned int tm_get_atlas();

void tm_destroy();
#endif
