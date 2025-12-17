#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

void tm_init();

void tm_begin_atlas();
unsigned int tm_add_texture_to_atlas(const char* texture);
unsigned int tm_end_atlas();

int tm_begin_dynamic_atlas(int maxTextureNum, int spriteWidth, int spriteHeight);
int tm_dynamic_atlas_add_texture(unsigned int atlasId, const char* texture, int index);

unsigned int tm_get_texture_id(const char* texture);
unsigned int tm_get_atlas();

void tm_destroy();
#endif
