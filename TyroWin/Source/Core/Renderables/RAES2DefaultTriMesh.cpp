//
//  RAES2DefaultTriMesh.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-09.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2DefaultTriMesh.h"
#include "RAES2CoreVisualEffects.h"
#include "RACamera.h"

namespace RAEngine
{

void ES2DefaultTriMesh::Init()
{
    ES2TriMesh::Init();
    SetVisualEffect(ES2CoreVisualEffects::GourandDirectional());
    SetColor(Wm5::Vector4f(1.0f, 0.0f, 0.0f, 1.0f));
}
    
void ES2DefaultTriMesh::InitSerialized()
{
    SetVisualEffect(ES2CoreVisualEffects::GourandDirectional());
}
    
ES2DefaultTriMeshSPtr ES2DefaultTriMesh::Create()
{
    ES2DefaultTriMeshSPtr ptr = std::make_shared<ES2DefaultTriMesh>();
    ptr->Init();
    return ptr;
}

void ES2DefaultTriMesh::SetColor(Wm5::Vector4f color)
{
    mColor = color;
}

void ES2DefaultTriMesh::UpdateUniformsWithCamera(const RAEngine::Camera* camera)
{
    Wm5::HMatrix viewMatrix = camera->GetViewMatrix();
    Wm5::HMatrix modelViewMatrix = viewMatrix * WorldTransform.Matrix();
    Wm5::HMatrix modelViewProjectionMatrix =  camera->GetProjectionMatrix() * modelViewMatrix;
    Wm5::Matrix3f normalMatrix = modelViewMatrix.GetNormalMatrix();

    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, modelViewProjectionMatrix.Transpose());
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, normalMatrix.Transpose());
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(2, mColor);
}

}