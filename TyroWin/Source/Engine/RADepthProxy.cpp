//
//  RADepthProxy.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-08-04.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RADepthProxy.h"
#include "RAES2CoreVisualEffects.h"
#include "RACamera.h"

namespace RAEngine
{

DepthProxySPtr DepthProxy::Create(ES2TriMeshSPtr mesh)
{
    DepthProxySPtr ptr = std::make_shared<DepthProxy>();
    ptr->Init(mesh);
    return ptr;
}
    
void DepthProxy::Init(ES2TriMeshSPtr mesh)
{
    ES2TriMesh::Init();
    SetVertexBuffer(mesh->GetVertexBuffer());
    SetIndexBuffer(mesh->GetIndexBuffer());
    SetVisualEffect(ES2CoreVisualEffects::DepthBuffer());
    LocalBoundBox = mesh->LocalBoundBox;
}

void DepthProxy::UpdateUniformsWithCamera(const RAEngine::Camera* camera)
{
    Wm5::HMatrix MVPMatrix = camera->GetProjectionMatrix() * camera->GetViewMatrix() * WorldTransform.Matrix();
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, MVPMatrix.Transpose());
}
}