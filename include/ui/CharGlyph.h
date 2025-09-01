#ifndef CHARGLYPH_H
#define CHARGLYPH_H
#include <cglm/types.h>

typedef struct {
    unsigned int texture;
    vec2 size;
    vec2 bearing;
    unsigned int advance;
} CharGlyph;

#endif
