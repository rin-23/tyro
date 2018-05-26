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

namespace tyro
{

ES2VisualEffectSPtr ES2CoreVisualEffects::VideoTexture() 
{
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("Video", "vsh"), GetFilePath("Video", "fsh"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(2);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
    vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aTexcoord"), vertexFormat->GetOffsetForNextAttrib(0), ES2VertexFormat::AT_FLOAT2, ES2VertexFormat::AU_TEXCOORD);

    //Create uniforms
    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(1);
    uniforms->SetUniform(0, shader->GetUniformLocation("uSampler"), 1, "uSampler", ES2ShaderUniforms::Uniform1iv);
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState(), new ES2PolygonOffset()));
    effect->GetAlphaState()->Enabled = true;
    effect->GetDepthState()->Enabled = false;
    
    return effect;
} 


ES2VisualEffectSPtr ES2CoreVisualEffects::PBR2() 
{
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("1.1.pbr_ao", "vs"), GetFilePath("1.1.pbr_ao", "fs"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(3);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPos"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
    vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aNormal"), vertexFormat->GetOffsetForNextAttrib(0), ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_NORMAL);
    vertexFormat->SetAttribute(2, shader->GetAttributeLocation("aColor"), vertexFormat->GetOffsetForNextAttrib(1), ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_COLOR);
    
    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(13);
    uniforms->SetUniform(0, shader->GetUniformLocation("projection"), 1, "projection", ES2ShaderUniforms::UniformMatrix4fv);
    uniforms->SetUniform(1, shader->GetUniformLocation("view"), 1, "view", ES2ShaderUniforms::UniformMatrix4fv);
    uniforms->SetUniform(2, shader->GetUniformLocation("model"), 1, "model", ES2ShaderUniforms::UniformMatrix4fv);
    
    uniforms->SetUniform(3, shader->GetUniformLocation("albedo"), 1, "albedo", ES2ShaderUniforms::Uniform3fv);
    uniforms->SetUniform(4, shader->GetUniformLocation("metallic"), 1, "metallic", ES2ShaderUniforms::Uniform1fv);
    uniforms->SetUniform(5, shader->GetUniformLocation("roughness"), 1, "roughness", ES2ShaderUniforms::Uniform1fv);
    uniforms->SetUniform(6, shader->GetUniformLocation("ao"), 1, "ao", ES2ShaderUniforms::Uniform1fv);

    uniforms->SetUniform(7, shader->GetUniformLocation("lightPositions[0]"), 1, "lightPositions[0]", ES2ShaderUniforms::Uniform3fv);
    uniforms->SetUniform(8, shader->GetUniformLocation("lightPositions[1]"), 1, "lightPositions[1]", ES2ShaderUniforms::Uniform3fv);
    uniforms->SetUniform(9, shader->GetUniformLocation("lightPositions[2]"), 1, "lightPositions[2]", ES2ShaderUniforms::Uniform3fv);
    uniforms->SetUniform(10, shader->GetUniformLocation("lightPositions[3]"), 1, "lightPositions[3]", ES2ShaderUniforms::Uniform3fv);

    uniforms->SetUniform(11, shader->GetUniformLocation("lightColors"), 1, "lightColors", ES2ShaderUniforms::Uniform3fv);
    uniforms->SetUniform(12, shader->GetUniformLocation("camPos"), 1, "camPos", ES2ShaderUniforms::Uniform3fv);
    
    //uniforms->SetUniform(10, shader->GetUniformLocation("norm"), 1, "norm", ES2ShaderUniforms::UniformMatrix3fv);


    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState(), new ES2PolygonOffset()));

    return effect;
}

ES2VisualEffectSPtr ES2CoreVisualEffects::PBR() 
{
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("1.1.pbr", "vs"), GetFilePath("1.1.pbr", "fs"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(3);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPos"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
    vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aNormal"), vertexFormat->GetOffsetForNextAttrib(0), ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_NORMAL);
    vertexFormat->SetAttribute(2, shader->GetAttributeLocation("aColor"), vertexFormat->GetOffsetForNextAttrib(1), ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_COLOR);
    
    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(12);
    uniforms->SetUniform(0, shader->GetUniformLocation("projection"), 1, "projection", ES2ShaderUniforms::UniformMatrix4fv);
    uniforms->SetUniform(1, shader->GetUniformLocation("view"), 1, "view", ES2ShaderUniforms::UniformMatrix4fv);
    uniforms->SetUniform(2, shader->GetUniformLocation("model"), 1, "model", ES2ShaderUniforms::UniformMatrix4fv);
    
    //uniforms->SetUniform(3, shader->GetUniformLocation("albedo"), 1, "albedo", ES2ShaderUniforms::Uniform3fv);
    uniforms->SetUniform(3, shader->GetUniformLocation("metallic"), 1, "metallic", ES2ShaderUniforms::Uniform1fv);
    uniforms->SetUniform(4, shader->GetUniformLocation("roughness"), 1, "roughness", ES2ShaderUniforms::Uniform1fv);
    uniforms->SetUniform(5, shader->GetUniformLocation("ao"), 1, "ao", ES2ShaderUniforms::Uniform1fv);

    uniforms->SetUniform(6, shader->GetUniformLocation("lightPositions[0]"), 1, "lightPositions[0]", ES2ShaderUniforms::Uniform3fv);
    uniforms->SetUniform(7, shader->GetUniformLocation("lightPositions[1]"), 1, "lightPositions[1]", ES2ShaderUniforms::Uniform3fv);
    uniforms->SetUniform(8, shader->GetUniformLocation("lightPositions[2]"), 1, "lightPositions[2]", ES2ShaderUniforms::Uniform3fv);
    uniforms->SetUniform(9, shader->GetUniformLocation("lightPositions[3]"), 1, "lightPositions[3]", ES2ShaderUniforms::Uniform3fv);

    uniforms->SetUniform(10, shader->GetUniformLocation("lightColors"), 1, "lightColors", ES2ShaderUniforms::Uniform3fv);
    uniforms->SetUniform(11, shader->GetUniformLocation("camPos"), 1, "camPos", ES2ShaderUniforms::Uniform3fv);
    
    //uniforms->SetUniform(10, shader->GetUniformLocation("norm"), 1, "norm", ES2ShaderUniforms::UniformMatrix3fv);


    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState(), new ES2PolygonOffset()));

    return effect;
}

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
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, 
                                                                 vertexFormat, 
                                                                 uniforms, 
                                                                 new ES2AlphaState(), 
                                                                 new ES2CullState(), 
                                                                 new ES2DepthState(),
                                                                 new ES2PolygonOffset()));
    
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
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState(), new ES2PolygonOffset()));

    return effect;
}

ES2VisualEffectSPtr ES2CoreVisualEffects::GourandDirectionalWithVColor()
{
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("GourandDirectionalWithVColor", "vsh"), GetFilePath("GourandDirectionalWithVColor", "fsh"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(3);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
    vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aNormal"), vertexFormat->GetOffsetForNextAttrib(0), ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_NORMAL);
    vertexFormat->SetAttribute(2, shader->GetAttributeLocation("aColor"), vertexFormat->GetOffsetForNextAttrib(1), ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_COLOR);
    
    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(2);
    uniforms->SetUniform(0, shader->GetUniformLocation("uMVPMatrix"), 1, "uMVPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
    uniforms->SetUniform(1, shader->GetUniformLocation("uNMatrix"), 1, "uNMatrix", ES2ShaderUniforms::UniformMatrix3fv);
    //uniforms->SetUniform(2, shader->GetUniformLocation("uColor"), 1, "uColor", ES2ShaderUniforms::Uniform4fv);
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState(), new ES2PolygonOffset()));

    return effect;
}

ES2VisualEffectSPtr ES2CoreVisualEffects::WireframeColor() 
{
    ES2ShaderProgram* shader = new ES2ShaderProgram();
    shader->LoadProgram(GetFilePath("WireframeColor", "vsh"), GetFilePath("WireframeColor", "fsh"));
    
    ES2VertexFormat* vertexFormat = new ES2VertexFormat(2);
    vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
    vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aColor"), vertexFormat->GetOffsetForNextAttrib(0), ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_COLOR);
    
    ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(1);
    uniforms->SetUniform(0, shader->GetUniformLocation("uMVPMatrix"), 1, "uMVPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
    //uniforms->SetUniform(1, shader->GetUniformLocation("uNMatrix"), 1, "uNMatrix", ES2ShaderUniforms::UniformMatrix3fv);
    //uniforms->SetUniform(2, shader->GetUniformLocation("uColor"), 1, "uColor", ES2ShaderUniforms::Uniform4fv);
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState(), new ES2PolygonOffset()));

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
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState(), new ES2PolygonOffset()));
    
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
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState(), new ES2PolygonOffset()));
    
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
    
    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState(), new ES2PolygonOffset()));

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

    ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState(), new ES2PolygonOffset()));
    effect->GetAlphaState()->Enabled = true;
    effect->GetDepthState()->Enabled = false;
    
    return effect;
}

}