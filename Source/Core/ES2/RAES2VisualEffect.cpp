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
mTexture(nullptr)
{
}

ES2VisualEffect::ES2VisualEffect(ES2ShaderProgram* shaderProgram, 
                                 ES2VertexFormat* vertexFormat, 
                                 ES2ShaderUniforms* uniforms)
:
mShaderProgram(shaderProgram),
mVertexFormat(vertexFormat),
mUniforms(uniforms),
mTexture(nullptr)
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

}

}