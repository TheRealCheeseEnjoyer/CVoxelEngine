#include "managers/GlyphManager.h"

#include <glad/glad.h>

#include <ft2build.h>

#include "ui/UIManager.h"

#include FT_FREETYPE_H

CharGlyph glyphs[128];
static unsigned int atlas;

void glyph_manager_init_glyphs(const char* ttfFile) {
    FT_Library ft;
    FT_Face face;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    FT_Init_FreeType(&ft);
    FT_New_Face(ft, ttfFile, 0, &face);
    int maxWidth = 256;
    int maxHeight = 256;
    FT_Set_Pixel_Sizes(face, maxWidth, maxHeight);
    glGenTextures(1, &atlas);
    glActiveTexture(GL_TEXTURE0 + UIManager_get_active_texture_id());
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, maxWidth, maxHeight, 128, 0, GL_RED, GL_UNSIGNED_BYTE,
                 nullptr);
    for (unsigned char c = 0; c < 128; c++) {
        FT_Load_Char(face, c, FT_LOAD_RENDER);
        CharGlyph glyph;
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, c, face->glyph->bitmap.width, face->glyph->bitmap.rows, 1, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        glyph.character = c;
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

unsigned int glyph_manager_get_atlas_id() {
    return atlas;
}

void glyph_manager_get_glyph(char c, CharGlyph* glyph) {
    *glyph = glyphs[c];
}
