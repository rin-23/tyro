//
//  RAES2DefaultPolyline.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-11.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2DefaultPolyline.h"
#include "RAES2CoreVisualEffects.h"
#include "RACamera.h"

namespace tyro
{
    
void ES2DefaultPolyline::Init(bool isContigious)
{
    ES2Polyline::Init(isContigious);
    mColor = Wm5::Vector4f(1,0,0,1);
    SetVisualEffect(ES2CoreVisualEffects::NoLightUniformColor());
}
    
void ES2DefaultPolyline::InitSerialized()
{
    SetVisualEffect(ES2CoreVisualEffects::NoLightUniformColor());
}
    
ES2DefaultPolylineSPtr ES2DefaultPolyline::Create(bool isContigious)
{
    ES2DefaultPolylineSPtr ptr = std::make_shared<ES2DefaultPolyline>();
    ptr->Init(isContigious);
    return ptr;
}

void ES2DefaultPolyline::SetColor(Wm5::Vector4f color)
{
    mColor = color;
}

void ES2DefaultPolyline::UpdateUniformsWithCamera(const Camera* camera)
{
    Wm5::HMatrix MVPMatrix = camera->GetProjectionMatrix() * camera->GetViewMatrix() * WorldTransform.Matrix();
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, MVPMatrix.Transpose());
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, mColor);
}
}