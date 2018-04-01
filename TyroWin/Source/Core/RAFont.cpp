//
//  RAFont.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-29.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAFont.h"
#include "RAFileManager.h"
#include "RALogManager.h"
#include <assert.h>
#include <algorithm>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace tyro
{

ES2Font::ES2Font()
:
mAtlas(nullptr),
mGlyphs(nullptr)
{}

ES2Font::~ES2Font()
{
    if (mGlyphs != nullptr)
    {
        delete [] mGlyphs;
    }
}

int ES2Font::Init(const std::string& tffFontPath, float sizePnts, int ppi)
{
    mSize = sizePnts;
    
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        fprintf(stderr, "Could not init freetype library\n");
        return 0;
    }
    
    FT_Face face;
    if (FT_New_Face(ft, tffFontPath.data(), 0, &face))
    {
        RA_LOG_ERROR("Could not open font\n");
        return 0;
    }

    if (FT_Set_Char_Size(face, 0, mSize*64, ppi, 0))
    {
        RA_LOG_ERROR("Could set char size \n");
        return 0;
    }
    
    FT_GlyphSlot glyph = face->glyph;

    //Get texture dimensions
    unsigned int atlas_width = 0;
    unsigned int atlas_height = 0;
    for (int i = 32; i < 128; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
        {
            RA_LOG_ERROR_ASSERT("Loading character %c failed!\n", i);
            continue;
        }
        
        atlas_width += glyph->bitmap.width;
        atlas_height = std::max(atlas_height, glyph->bitmap.rows);
    }
    
    //Generate empty texture atlas
    mAtlas = std::make_shared<ES2Texture2D>(Texture::TF_A8, atlas_width, atlas_height);
    mAtlas->LoadData(nullptr);
    mGlyphs = new GlyphInfo[128];
    
    //Copy glyph bitmap data into texture atlas
    int x = 0;
    for(unsigned int i = 32; i < 128; i++)
    {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER))
        {
            RA_LOG_ERROR_ASSERT("Loading character %c failed!\n", i);
            continue;
        }

        mAtlas->LoadSubData(x, 0, glyph->bitmap.width, glyph->bitmap.rows, glyph->bitmap.buffer);
        
        mGlyphs[i].character = static_cast<char>(i);
        
        mGlyphs[i].ax = glyph->advance.x >> 6;
        mGlyphs[i].ay = glyph->advance.y >> 6;
        
        mGlyphs[i].bw = glyph->bitmap.width;
        mGlyphs[i].bh = glyph->bitmap.rows;
        
        mGlyphs[i].bl = glyph->bitmap_left;
        mGlyphs[i].bt = glyph->bitmap_top;
        
        using Wm5::Vector2f;
        float tx = (float)x / atlas_width;
        mGlyphs[i].uv_lt = Vector2f(tx, (float)glyph->bitmap.rows/atlas_height);
        mGlyphs[i].uv_lb = Vector2f(tx, 0);
        mGlyphs[i].uv_rt = Vector2f(tx + (float)glyph->bitmap.width/atlas_width, (float)glyph->bitmap.rows/atlas_height);
        mGlyphs[i].uv_rb = Vector2f(tx + (float)glyph->bitmap.width/atlas_width, 0);
        
        x += glyph->bitmap.width;
    }
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    return 1;
}

const ES2Font::GlyphInfo* ES2Font::GetGlyphInfo(char character) const
{
    GlyphInfo* glyphInfo = &mGlyphs[static_cast<int>(character)];
    return glyphInfo;
}

ES2Texture2DSPtr ES2Font::GetTextureAtlas() const
{
    return mAtlas;
}

FontManager* FontManager::mInstance = 0;

FontManager::FontManager()
{
}

FontManager::~FontManager()
{}

FontManager* FontManager::GetSingleton()
{
    if (!mInstance)
    {
        mInstance = new FontManager();
    }
    return mInstance;
}

void FontManager::Setup(int ppi, float scale)
{
    mPPI = ppi;
    mSystemFont12 = std::make_shared<ES2Font>();
    std::string fontPath = GetFilePath("times", "ttf");
    int result = mSystemFont12->Init(fontPath, 12, mPPI);
    mScale = scale;
    assert(result);
}

ES2FontSPtr FontManager::GetSystemFontOfSize12() const
{
    return mSystemFont12;
}

int FontManager::GetPPI() const
{
    return mPPI;
}

float FontManager::GetScreenScale() const
{
    return mScale;
}

}