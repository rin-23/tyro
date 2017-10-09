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

namespace RAEngine
{
    class ES2VisualEffect
    {
    public:

        ES2VisualEffect();
        
        ES2VisualEffect(ES2ShaderProgram* shaderProgram, ES2VertexFormat* vertexFormat, ES2ShaderUniforms* uniforms, ES2AlphaState* alphaState, ES2CullState* cullState, ES2DepthState* depthState);
        
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

        inline void SetAlphaState(ES2AlphaState* alphaState);
        inline ES2AlphaState* GetAlphaState();
        
        inline void SetCullState(ES2CullState* cullState);
        inline ES2CullState* GetCullState();

        inline void SetDepthState(ES2DepthState* depthState);
        inline ES2DepthState* GetDepthState();
        
        inline void SetTexture2D(ES2Texture2DSPtr texture);
        inline ES2Texture2DSPtr GetTexture2D();
        
    private:
        ES2ShaderProgram* mShaderProgram;
        ES2VertexFormat* mVertexFormat;
        ES2ShaderUniforms* mUniforms;
        ES2AlphaState* mAlphaState;
        ES2CullState* mCullState;
        ES2DepthState* mDepthState;
        ES2Texture2DSPtr mTexture;
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
    
    inline void ES2VisualEffect::SetAlphaState(ES2AlphaState* alphaState)
    {
        if (mAlphaState != nullptr)
        {
            delete mAlphaState;
        }
        mAlphaState = alphaState;
    }
    
    inline ES2AlphaState* ES2VisualEffect::GetAlphaState()
    {
        return mAlphaState;
    }
    
    inline void ES2VisualEffect::SetCullState(ES2CullState* cullState)
    {
        if (mCullState != nullptr)
        {
            delete mCullState;
        }
        mCullState = cullState;
    }
    
    inline ES2CullState* ES2VisualEffect::GetCullState()
    {
        return mCullState;
    }
    
    inline void ES2VisualEffect::SetDepthState(ES2DepthState* depthState)
    {
        if (mDepthState != nullptr)
        {
            delete mDepthState;
        }
        mDepthState = depthState;
    }
    
    inline ES2DepthState* ES2VisualEffect::GetDepthState()
    {
        return mDepthState;
    }
    
    inline void ES2VisualEffect::SetTexture2D(ES2Texture2DSPtr texture)
    {
        mTexture = texture;
    }
    
    inline ES2Texture2DSPtr ES2VisualEffect::GetTexture2D()
    {
        return mTexture;
    }
    
    using ES2VisualEffectSPtr = std::shared_ptr<ES2VisualEffect>;
}
