//
//  RAHitProxy.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-08-04.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAHitProxy.h"
#include "Wm5Float2.h"
#include "GLStructures.h"
#include "RAES2CoreVisualEffects.h"
#include "RATouchable.h"
#include "RAVertexBufferAccessor.h"
#include "RACamera.h"

namespace tyro
{

void HitProxy::Init(TouchableSPtr touchable, ObjectID touchable_id, HitProxyPriority priority)
{
    ES2TriMesh::Init();
    m_touchable = touchable;
    m_touchable_id = touchable_id;
    mPriority = priority;
    m_color_id = HitProxy::CreateColorID(GetID());
}

//HitProxySPtr HitProxy::Create(TouchableSPtr touchable, ObjectID touchable_id, HitProxyPriority priority)
//{
//    HitProxySPtr ptr = std::make_shared<HitProxy>();
//    ptr->Init(touchable, touchable_id, priority);
//    return ptr;
//}

Wm5::Vector3uc HitProxy::CreateColorID(ObjectID objectID)
{
    Wm5::Vector3uc hash;
    
    int r = (objectID & 0x000000FF) >>  0;
    int g = (objectID & 0x0000FF00) >>  8;
    int b = (objectID & 0x00FF0000) >> 16;
    
    hash = Wm5::Vector3uc(r,g,b);
    
    return hash;
}

ObjectID HitProxy::GetIDFromColor(const Wm5::Vector3uc& color)
{
    int pickedID = color[0] + color[1]*256 + color[2]*256*256;
    return pickedID;
}

const Wm5::Vector3uc& HitProxy::GetColorID() const
{
    return m_color_id;
}

TouchableSPtr HitProxy::GetTouchable() const
{
    return m_touchable.lock();
}

ObjectID HitProxy::GetTouchableID() const
{
    return m_touchable_id;
}

void HitProxy::SetPriority(HitProxyPriority priority)
{
    mPriority = priority;
}

HitProxy::HitProxyPriority HitProxy::GetPriority() const
{
    return mPriority;
}

//-------------------------------------------------------------------------------------

void RectHitProxy::Init(TouchableSPtr touchable, ObjectID touchable_id)
{
    HitProxy::Init(touchable, touchable_id, HitProxyPriority::UIELEMENT);
    int xSamples = 2;
    int ySamples = 2;
    float xExtent = 0.5;
    float yExtent = 0.5;
    
    int numVertices = xSamples*ySamples;
    int numTriangles = 2*(xSamples-1)*(ySamples-1);
    int numIndices = 3*numTriangles;
    int stride = sizeof(Wm5::Vector3f);
    
    SetVisualEffect(ES2CoreVisualEffects::ColorPicking());
    GetVisualEffect()->GetDepthState()->Enabled = false;
    
    // Create a vertex buffer.
    ES2VertexHardwareBufferSPtr vbuffer(std::make_shared<ES2VertexHardwareBuffer>(stride, numVertices, nullptr, HardwareBuffer::BU_STATIC));
    VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), vbuffer.get());
    vba.Map(HardwareBuffer::BL_READ_WRITE);
    
    // Generate geometry.
    float inv0 = 1.0f/(xSamples - 1.0f);
    float inv1 = 1.0f/(ySamples - 1.0f);
    float u, v, x, y;
    int i, i0, i1;
    for (i1 = 0, i = 0; i1 < ySamples; ++i1)
    {
        v = i1*inv1;
        y = (2.0f*v - 1.0f)*yExtent;
        for (i0 = 0; i0 < xSamples; ++i0, ++i)
        {
            u = i0*inv0;
            x = (2.0f*u - 1.0f)*xExtent;
            
            vba.Position<Wm5::Float3>(i) = Wm5::Float3(x, y, 0.0f);
        }
    }
    
    vba.Unmap();
    
    // Generate indices.
    ES2IndexHardwareBufferSPtr ibuffer(std::make_shared<ES2IndexHardwareBuffer>(numIndices, nullptr, HardwareBuffer::BU_STATIC));
    uint32_t* indices = static_cast<uint32_t*>(ibuffer->MapWrite());
    for (i1 = 0; i1 < ySamples - 1; ++i1)
    {
        for (i0 = 0; i0 < xSamples - 1; ++i0)
        {
            int v0 = i0 + xSamples * i1;
            int v1 = v0 + 1;
            int v2 = v1 + xSamples;
            int v3 = v0 + xSamples;
            *indices++ = v0;
            *indices++ = v1;
            *indices++ = v2;
            *indices++ = v0;
            *indices++ = v2;
            *indices++ = v3;
        }
    }
    
    ibuffer->Unmap();
    
    LocalBoundBox = AxisAlignedBBox(Wm5::Vector3f(xExtent, yExtent, 0), Wm5::Vector3f(-xExtent, -yExtent, 0));
    
    SetVertexBuffer(vbuffer);
    SetIndexBuffer(ibuffer);
}

RectHitProxySPtr RectHitProxy::Create(TouchableSPtr touchable, ObjectID touchable_id)
{
    RectHitProxySPtr ptr = std::make_shared<RectHitProxy>();
    ptr->Init(touchable, touchable_id);
    return ptr;
}
    
void RectHitProxy::UpdateUniformsWithCamera(const tyro::Camera* camera)
{
    Wm5::HMatrix MVPMatrix =  camera->GetProjectionMatrix() * camera->GetViewMatrix() * WorldTransform.Matrix();
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, MVPMatrix.Transpose());
    Wm5::Vector3uc uc_colorId = GetColorID();
    Wm5::Vector4f f_colorId(uc_colorId[0]/255.0f, uc_colorId[1]/255.0f, uc_colorId[2]/255.0f, 1.0f);
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, f_colorId);
}

//-------------------------------------------------------------------------------------
TriHitProxySPtr TriHitProxy::Create(ES2TriMeshSPtr mesh, TouchableSPtr touchable, ObjectID touchable_id)
{
    TriHitProxySPtr ptr = std::make_shared<TriHitProxy>();
    ptr->Init(mesh, touchable, touchable_id);
    return ptr;
}
    
void TriHitProxy::Init(ES2TriMeshSPtr mesh, TouchableSPtr touchable, ObjectID touchable_id)
{
    HitProxy::Init(touchable, touchable_id);
    SetVisualEffect(ES2CoreVisualEffects::ColorPicking());
//    GetVisualEffect()->GetDepthState()->Enabled = false;
    SetVertexBuffer(mesh->GetVertexBuffer());
    SetIndexBuffer(mesh->GetIndexBuffer());
    LocalBoundBox = mesh->LocalBoundBox;
}

void TriHitProxy::UpdateUniformsWithCamera(const tyro::Camera* camera)
{
    Wm5::HMatrix MVPMatrix =  camera->GetProjectionMatrix() * camera->GetViewMatrix() * WorldTransform.Matrix();
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, MVPMatrix.Transpose());
    Wm5::Vector3uc uc_colorId = GetColorID();
    Wm5::Vector4f f_colorId(uc_colorId[0]/255.0f, uc_colorId[1]/255.0f, uc_colorId[2]/255.0f, 1.0f);
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, f_colorId);

}

}