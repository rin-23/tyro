//
//  RANormalMapProxy.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-08-07.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RANormalMapProxy.h"
#include "RAES2CoreVisualEffects.h"
#include "RACamera.h"
#include "RAES2VisualEffect.h"
#include "RAES2ShaderUniforms.h"

namespace tyro
{
void NormalProxy::Init(ES2TriMeshSPtr mesh)
{
    ES2TriMesh::Init();
    SetVertexBuffer(mesh->GetVertexBuffer());
    SetIndexBuffer(mesh->GetIndexBuffer());
    SetVisualEffect(ES2CoreVisualEffects::NormalBuffer());
    LocalBoundBox = mesh->LocalBoundBox;
}
    
NormalProxySPtr NormalProxy::Create(ES2TriMeshSPtr mesh)
{
    NormalProxySPtr ptr = std::make_shared<NormalProxy>();
    ptr->Init(mesh);
    return ptr;
}

void NormalProxy::UpdateUniformsWithCamera(const tyro::Camera* camera)
{
    Wm5::HMatrix modelViewMatrix = camera->GetViewMatrix() * WorldTransform.Matrix();
    Wm5::HMatrix MVPMatrix =  camera->GetProjectionMatrix() * modelViewMatrix;
    Wm5::Matrix3f NMatrix = WorldTransform.Matrix().GetNormalMatrix();
    
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, MVPMatrix.Transpose());
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, NMatrix.Transpose());
}
}