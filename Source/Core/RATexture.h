//
//  RATexture.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-30.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

namespace tyro
{
    class Texture
    {
    public:
        
        // Currently supported formats.  More may be added as needed.
        enum  TextureFormat
        {
            TF_NONE,
            
            // Small-bit color formats.
            TF_R5G6B5,
            TF_A1R5G5B5,
            TF_A4R4G4B4,
            
            // 8-bit integer formats.
            TF_R8,
            TF_A8,
            TF_L8,
            TF_A8L8,
            TF_R8G8B8,
            TF_R8G8B8A8,
            TF_A8B8G8R8,
            
            // 16-bit integer formats.
            TF_L16,
            TF_G16R16,
            TF_A16B16G16R16,
            
            // 16-bit floating-point formats ('half float' channels).
            TF_R16F,
            TF_G16R16F,
            TF_A16B16G16R16F,
            
            // 32-bit floating-point formats ('float' channels).
            TF_R32F,
            TF_G32R32F,
            TF_A32B32G32R32F,
            
            // Depth-stencil format.
            TF_D24S8,
            TF_QUANTITY
        };
        
        enum  TextureType
        {
            TT_1D,
            TT_2D,
            TT_3D,
            TT_CUBE,
            TT_QUANTITY
        };
        
        Texture() {};

        Texture(TextureFormat format, TextureType type);

        virtual ~Texture();

        // Member access.
        inline TextureFormat GetFormat () const;
        inline TextureType GetType () const;
        inline int GetWidth() const;
        inline int GetHeight() const;
        inline int GetDepth() const;

    protected:
        TextureFormat mFormat;
        TextureType mType;
        int mWidth;
        int mHeight;
        int mDepth; //3D tex only
    };
    
    inline Texture::TextureFormat Texture::GetFormat () const
    {
        return mFormat;
    }
    
    inline Texture::TextureType Texture::GetType () const
    {
        return mType;
    }

    inline int Texture::GetWidth() const
    {
        return mWidth;
    }
    
    inline int Texture::GetHeight() const
    {
        return mHeight;
    }
    
    inline int Texture::GetDepth() const
    {
        return mDepth;
    }
}

