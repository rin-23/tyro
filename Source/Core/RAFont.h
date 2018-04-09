//
//  RAFont.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-29.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2Texture2D.h"
#include "Wm5Vector2.h"
#include "RAEnginePrerequisites.h"
#include <memory>

//Ref:
//http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
//http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_02

namespace tyro
{
    class ES2Font
    {
    public:
        ES2Font();

        ~ES2Font();
        
        int Init(const std::string& tffFontPath, float sizePnts, int ppi);
        
        struct GlyphInfo
        {
            char character;
            
            float ax; // advance.x
            float ay; // advance.y
            
            float bw; // bitmap.width;
            float bh; // bitmap.rows;
            
            float bl; // bitmap_left;
            float bt; // bitmap_top;
            
            Wm5::Vector2f uv_lt; // uv coord left top corner
            Wm5::Vector2f uv_lb; // uv coord left bottom corner
            Wm5::Vector2f uv_rt; // uv coord right top corner
            Wm5::Vector2f uv_rb; // uv coord right bottom corner
            
        };
        
        const GlyphInfo* GetGlyphInfo(char character) const;
        
        ES2Texture2DSPtr GetTextureAtlas() const;

    private:
        float mSize;
        
        GlyphInfo* mGlyphs;
        ES2Texture2DSPtr mAtlas;
    };

    class FontManager
    {
    public:
        static FontManager* GetSingleton();

        ~FontManager();
        
        ES2FontSPtr GetSystemFontOfSize12() const;
        
        void Setup(int ppi, float scale);
        
//#warning - HACK. Need to create a separate class RenderTarget to manage this
        int GetPPI() const;
        float GetScreenScale() const;

    private:
        FontManager();
        static FontManager* mInstance;
        ES2FontSPtr mSystemFont12;
        int mPPI;
        float mScale;
    };
}




