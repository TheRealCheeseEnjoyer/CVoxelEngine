#include "managers/GlyphManager.h"

#include <glad/glad.h>

#include <ft2build.h>
#include FT_FREETYPE_H

CharGlyph glyphs[128];

void glyph_manager_init_glyphs(const char* ttfFile) {
    FT_Library ft;
    FT_Face face;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    FT_Init_FreeType(&ft);
    FT_New_Face(ft, ttfFile, 0, &face);
    FT_Set_Pixel_Sizes(face, 0, 48);
    for (unsigned char c = 0; c < 128; c++) {
        FT_Load_Char(face, c, FT_LOAD_RENDER);
        CharGlyph glyph;
        glGenTextures(1, &glyph.texture);
        glBindTexture(GL_TEXTURE_2D, glyph.texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glyph.size[0] = face->glyph->bitmap.width;
        glyph.size[1] = face->glyph->bitmap.rows;
        glyph.bearing[0] = face->glyph->bitmap_left;
        glyph.bearing[1] = face->glyph->bitmap_top;
        glyph.advance = face->glyph->advance.x;
        glyphs[c] = glyph;
    }

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void glyph_manager_get_glyph(char c, CharGlyph* glyph) {
    *glyph = glyphs[c];
}
