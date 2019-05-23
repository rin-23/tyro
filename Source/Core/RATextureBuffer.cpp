//
//  RATextureBuffer.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-17.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RATextureBuffer.h"
#include <cstdlib>
#include "RALogManager.h"
#include "Wm5Vector2.h"
//#include <easyppm/easyppm.h>

namespace tyro
{
    TextureBuffer::TextureBuffer(int format, int type, void* data, int width, int height)
    {
        mFormat = format;
        mType = type;
        mData = data;
        mWidth = width;
        mHeight = height;
        mNumOfBytes = calculateNumOfBytes();
    }
    
    TextureBuffer::~TextureBuffer()
    {
        if (mData) {
            free(mData);
        }
    };

    // bool TextureBuffer::toPPM(const std::string& path) 
    // {
    //     PPM ppm = easyppm_create(mWidth, mHeight, IMAGETYPE_PPM);
        
    //     for (int i =0; i < mWidth; ++i) 
    //     {
    //         for (int j = 0; j < mHeight; ++j) 
    //         {       
    //             Wm5::Vector4uc c = GetColor(Wm5::Vector2i(i,j));
    //             easyppm_set(&ppm, i, j, easyppm_rgb(c[0], c[1], c[2]));
    //         }
    //     }
    //     easyppm_write(&ppm, path.c_str());        
    // }
    
    Wm5::Vector4uc TextureBuffer::GetColor(const Wm5::Vector2i& touchPoint) const
    {
        if (mType != TB_UNSIGNED_BYTE || mFormat != TF_RGBA) 
        {
            RA_LOG_WARN("Unsuported texture format");
            return Wm5::Vector4uc(0, 0, 0, 0);
        }
        
        int x = touchPoint.X();
        int y = touchPoint.Y();
    
        int index = y * mWidth + x;
        int index4 = index * 4;
        
        uint8* pixels = (uint8*)mData;
        if (index4 + 3 < mNumOfBytes && index4 > 0) {
            
            uint8 r = pixels[index4];
            uint8 g = pixels[index4 + 1];
            uint8 b = pixels[index4 + 2];
            uint8 a = pixels[index4 + 3];
            
            return Wm5::Vector4uc(r, g, b, a);
        }
        RA_LOG_WARN("color index is out of bounds");
        return Wm5::Vector4uc(0, 0, 0, 0);
    }
    
    
    int TextureBuffer::calculateNumOfBytes()
    {
        int typeSize = 0;
        if (mType == TB_UNSIGNED_BYTE) {
            typeSize = 1;
        }
        
        int formatSize = 0;
        if (mFormat == TF_RGBA) {
            formatSize = 4;
        }
        
        return typeSize * formatSize * mWidth * mHeight;
    }
}