//
//  RATextureBuffer.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-17.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAEnginePrerequisites.h"
#include "Wm5Vector4.h"

namespace tyro
{
    class TextureBuffer
    {
    public:
        enum TextureBufferType
        {
            TB_UNSIGNED_BYTE
        };
        
        enum TextureBufferFormat
        {
            TF_RGBA
        };
        
        TextureBuffer(int format, int type, void* data, int width, int height);
        ~TextureBuffer();
        
        Wm5::Vector4uc GetColor(const Wm5::Vector2i& touchPoint) const;
        
        //bool toPPM(const std::string& path);

        inline void* GetData() const
        {
            return mData;
        }
        
        inline int GetWidth() const
        {
            return mWidth;
        }
        
        inline int GetHeight() const
        {
            return mHeight;
        }
        
        inline int GetNumOfBytes() const
        {
            return mNumOfBytes;
        }
        
    private:
        int mFormat;
        int mType;
        void* mData;
        int mWidth;
        int mHeight;
        int mNumOfBytes;
        
        int calculateNumOfBytes();
    };
}
