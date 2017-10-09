//
//  RAES2CoreEffects.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-10.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2CoreVisualEffects.h"
#include "RAFileManager.h"
#include "GLStructures.h"
#include "RAES2VisualEffect.h"

namespace RAEngine
{

ES2VisualEffectSPtr ES2CoreVisualEffects::ColorPicking()
{
    //Load shaders
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("NoLightUniformColor", "vsh"), GetFilePath("NoLightUniformColor", "fsh"));

    ES2VertexFormat* vertexFormat = new ES2VertexFormat(1);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);

    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(2);
    uniforms->SetUniform(0, shader->GetUniformLocation("uMVPMatrix"), 1, "uMVPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
    uniforms->SetUniform(1, shader->GetUniformLocation("uColor"), 1, "uColor", ES2ShaderUniforms::Uniform4fv);
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState()));
    
    return effect;
}

ES2VisualEffectSPtr ES2CoreVisualEffects::GourandDirectional()
{
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("GourandDirectional", "vsh"), GetFilePath("GourandDirectional", "fsh"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(2);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
    vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aNormal"), vertexFormat->GetOffsetForNextAttrib(0), ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_NORMAL);
    
    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(3);
    uniforms->SetUniform(0, shader->GetUniformLocation("uMVPMatrix"), 1, "uMVPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
    uniforms->SetUniform(1, shader->GetUniformLocation("uNMatrix"), 1, "uNMatrix", ES2ShaderUniforms::UniformMatrix3fv);
    uniforms->SetUniform(2, shader->GetUniformLocation("uColor"), 1, "uColor", ES2ShaderUniforms::Uniform4fv);
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState()));

    return effect;
}

ES2VisualEffectSPtr ES2CoreVisualEffects::NormalBuffer()
{
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("NormalBuffer", "vsh"), GetFilePath("NormalBuffer", "fsh"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(2);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
    vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aPackedNormal"), vertexFormat->GetOffsetForNextAttrib(0), ES2VertexFormat::AT_FLOAT1, ES2VertexFormat::AU_PACKED_NORMAL);
    
    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(2);
    uniforms->SetUniform(0, shader->GetUniformLocation("uMVPMatrix"), 1, "uMVPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
    uniforms->SetUniform(1, shader->GetUniformLocation("uNMatrix"), 1, "uNMatrix", ES2ShaderUniforms::UniformMatrix3fv);
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState()));
    
    return effect;
}

ES2VisualEffectSPtr ES2CoreVisualEffects::DepthBuffer()
{
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("DepthBuffer", "vsh"), GetFilePath("DepthBuffer", "fsh"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(1);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);

    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(1);
    uniforms->SetUniform(0, shader->GetUniformLocation("uMVPMatrix"), 1, "uMVPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState()));
    
    return effect;
}

ES2VisualEffectSPtr ES2CoreVisualEffects::NoLightUniformColor()
{
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("NoLightUniformColor", "vsh"), GetFilePath("NoLightUniformColor", "fsh"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(1);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
    
    //Create uniforms
    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(3);
    uniforms->SetUniform(0, shader->GetUniformLocation("uMVPMatrix"), 1, "uMVPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
    uniforms->SetUniform(1, shader->GetUniformLocation("uColor"), 1, "uColor", ES2ShaderUniforms::Uniform4fv);
    uniforms->SetUniform(2, shader->GetUniformLocation("uPointSize"), 1, "uPointSize", ES2ShaderUniforms::Uniform1fv);
    const float pSize = 1.0f;
    uniforms->UpdateFloatUniform(2, &pSize);
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState()));

    return effect;
}

ES2VisualEffectSPtr ES2CoreVisualEffects::BitmapText()
{
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("Text", "vsh"), GetFilePath("Text", "fsh"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(2);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
    vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aTexcoord"), vertexFormat->GetOffsetForNextAttrib(0), ES2VertexFormat::AT_FLOAT2, ES2VertexFormat::AU_TEXCOORD);

    //Create uniforms
    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(3);
    uniforms->SetUniform(0, shader->GetUniformLocation("uColor"), 1, "uColor", ES2ShaderUniforms::Uniform4fv);
    uniforms->SetUniform(1, shader->GetUniformLocation("uSampler"), 1, "uSampler", ES2ShaderUniforms::Uniform1iv);
    uniforms->SetUniform(2, shader->GetUniformLocation("uNDCTransform"), 1, "uNDCTransform", ES2ShaderUniforms::UniformMatrix4fv);

    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState()));
    effect->GetAlphaState()->Enabled = true;
    effect->GetDepthState()->Enabled = false;
    
    return effect;
}

}