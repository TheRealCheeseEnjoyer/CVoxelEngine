#ifndef GLYPHMANAGER_H
#define GLYPHMANAGER_H

#include "ui/CharGlyph.h"

void glyph_manager_init_glyphs(const char* ttfFile);
unsigned int glyph_manager_get_atlas_id();
void glyph_manager_get_glyph(char c, CharGlyph* glyph);

#endif
