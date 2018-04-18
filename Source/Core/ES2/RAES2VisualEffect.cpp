//
//  RAES2VisualEffect.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-03.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2VisualEffect.h"

namespace tyro
{

ES2VisualEffect::ES2VisualEffect()
:
mShaderProgram(nullptr),
mVertexFormat(nullptr),
mUniforms(nullptr),
mAlphaState(nullptr),
mCullState(nullptr),
mDepthState(nullptr),
mTexture(nullptr),
mPolygonOffset(nullptr)
{
}

ES2VisualEffect::ES2VisualEffect(ES2ShaderProgram* shaderProgram, 
                                 ES2VertexFormat* vertexFormat, 
                                 ES2ShaderUniforms* uniforms, 
                                 ES2AlphaState* alphaState, 
                                 ES2CullState* cullState, 
                                 ES2DepthState* depthState,
                                 ES2PolygonOffset* poly_offset)
:
mShaderProgram(shaderProgram),
mVertexFormat(vertexFormat),
mUniforms(uniforms),
mAlphaState(alphaState),
mCullState(cullState),
mDepthState(depthState),
mTexture(nullptr),
mPolygonOffset(poly_offset)
{
}

ES2VisualEffect::~ES2VisualEffect()
{
    if (mShaderProgram != nullptr)
    {
        delete mShaderProgram;
    }
    
    if (mVertexFormat != nullptr)
    {
        delete mVertexFormat;
    }
    
    if (mUniforms != nullptr)
    {
        delete mUniforms;
    }
    
    if (mAlphaState != nullptr)
    {
        delete mAlphaState;
    }
    
    if (mCullState != nullptr)
    {
        delete mCullState;
    }
    
    if (mDepthState != nullptr)
    {
        delete mDepthState;
    }

    if (mPolygonOffset != nullptr)
    {
        delete mPolygonOffset;
    }
}

}