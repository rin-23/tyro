    //
//  RAES2VisualEffect.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-03.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once
#include "RAEnginePrerequisites.h"
#include "RAES2VertexFormat.h"
#include "RAES2ShaderProgram.h"
#include "RAES2ShaderUniforms.h"
#include "RAES2VisualStates.h"
#include "RAES2Texture2D.h"
#include "RAES2BufferTexture.h"
#include <vector>

namespace tyro
{
    class ES2VisualEffect
    {
    public:
    
        ES2VisualEffect();
        
        ES2VisualEffect(ES2ShaderProgram* shaderProgram, 
                        ES2VertexFormat* vertexFormat, 
                        ES2ShaderUniforms* uniforms);        
        
        ~ES2VisualEffect();
        
        //setters delete previous object that was pointed to
        inline void SetShader(ES2ShaderProgram* shaderProgram);
        inline const ES2ShaderProgram* GetShader() const;
        inline ES2ShaderProgram* GetShader();
        
        inline void SetVertexFormat(ES2VertexFormat* vertexFormat);
        inline const ES2VertexFormat* GetVertexFormat() const;
        inline ES2VertexFormat* GetVertexFormat();
        
        inline void SetUniforms(ES2ShaderUniforms* uniforms);
        inline const ES2ShaderUniforms* GetUniforms() const;
        inline ES2ShaderUniforms* GetUniforms();
        
        inline void SetTexture2D(ES2Texture2DSPtr texture);
        inline ES2Texture2DSPtr GetTexture2D(); 

        inline void AddBufferTexture(ES2BufferTextureSPtr texture);
        inline int NumBufferTextures() const;
        inline ES2BufferTextureSPtr GetBufferTexture(int i) const;

        bool AlphaStateEnabled = false;
        bool CullStateEnabled = true;
        bool DepthStateEnabled = true;
        bool DepthMaskEnabled = true;

        //Polygon offset 
        bool PolygonOffsetEnabled = false;
        float PolygonOffsetFactor = 0.0;
        float PolygonOffsetUnits = -5.0;
        bool PolygonOffsetIsSolid = false;
        
    private:
        ES2ShaderProgram* mShaderProgram;
        ES2VertexFormat* mVertexFormat;
        ES2ShaderUniforms* mUniforms;
        ES2Texture2DSPtr mTexture;
        std::vector<ES2BufferTextureSPtr> mBufferTextures;
    };
    
    /****INLINE FUNCTIONS****/
    
    inline void ES2VisualEffect::SetShader(ES2ShaderProgram* shaderProgram)
    {
        if (mShaderProgram != nullptr)
        {
            delete mShaderProgram;
        }
        mShaderProgram = shaderProgram;
    }
    
    inline const ES2ShaderProgram* ES2VisualEffect::GetShader() const
    {
        return mShaderProgram;
    }
    
    inline ES2ShaderProgram* ES2VisualEffect::GetShader()
    {
        return mShaderProgram;
    }
    
    inline void ES2VisualEffect::SetVertexFormat(ES2VertexFormat* vertexFormat)
    {
        if (mVertexFormat != nullptr)
        {
            delete mVertexFormat;
        }
        mVertexFormat = vertexFormat;
    }
    
    inline const ES2VertexFormat* ES2VisualEffect::GetVertexFormat() const
    {
        return mVertexFormat;
    }
    
    inline ES2VertexFormat* ES2VisualEffect::GetVertexFormat()
    {
        return mVertexFormat;
    }
    
    inline void ES2VisualEffect::SetUniforms(ES2ShaderUniforms* uniforms)
    {
        if (mUniforms != nullptr)
        {
            delete mUniforms;
        }
        mUniforms = uniforms;
    }
    
    inline const ES2ShaderUniforms* ES2VisualEffect::GetUniforms() const
    {
        return mUniforms;
    }
    
    inline ES2ShaderUniforms* ES2VisualEffect::GetUniforms()
    {
        return mUniforms;
    }

    inline void ES2VisualEffect::SetTexture2D(ES2Texture2DSPtr texture)
    {
        mTexture = texture;
    }
    
    inline ES2Texture2DSPtr ES2VisualEffect::GetTexture2D()
    {
        return mTexture;
    }

    inline void ES2VisualEffect::AddBufferTexture(ES2BufferTextureSPtr texture) 
    {
        mBufferTextures.push_back(texture);
    }

    inline int ES2VisualEffect::NumBufferTextures() const 
    {
        return mBufferTextures.size();
    } 

    inline ES2BufferTextureSPtr ES2VisualEffect::GetBufferTexture(int i)  const
    {
        if (i < NumBufferTextures())
            return mBufferTextures[i];
        
        return nullptr;
    }
    
    using ES2VisualEffectSPtr = std::shared_ptr<ES2VisualEffect>;
}
