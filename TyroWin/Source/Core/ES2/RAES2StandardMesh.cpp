//
//  RAES2StandardMesh.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-09.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2StandardMesh.h"
#include <vector>
#include "Wm5Vector3.h"
#include "GLStructures.h"
#include "RAFileManager.h"
#include "RACamera.h"
#include "RAVertexHardwareBuffer.h"
#include "RAES2CoreVisualEffects.h"
#include "RAVertexBufferAccessor.h"
#include "Wm5Float2.h"
#include "RAMath.h"

using Wm5::Vector3f;

namespace RAEngine
{

void ES2Cylinder::Init(int axisSamples, int radialSamples, float topRadius, float btmRadius, float height, bool open)
{
    ES2DefaultTriMesh::Init();
    
    mAxisSamples = axisSamples;
    mRadialSamples = radialSamples;
    mTopRadius = topRadius;
    mBtmRadius = btmRadius;
    mHeight = height;
    float halfHeight = height/2;
    mOpen = open;
    
    float angleStep = 2.0f * M_PI / radialSamples;
    float prevAngle = 0.0f;
    
    Vector3f* topRing = new Vector3f[radialSamples];
    Vector3f* btmRing = new Vector3f[radialSamples];
    
    //create points for top and bottom rings
    for (int i = 0; i < radialSamples; ++i)
    {
        float zTop = sinf(prevAngle) * mTopRadius;
        float xTop = cosf(prevAngle) * mTopRadius;
        topRing[i] = Vector3f(xTop, halfHeight,zTop);
        
        float zBtm = sinf(prevAngle) * mBtmRadius;
        float xBtm = cosf(prevAngle) * mBtmRadius;
        btmRing[i] = Vector3f(xBtm, -halfHeight, zBtm);
        
        prevAngle += angleStep;
    }
    
    //create triangular vertex/index data
    std::vector<VertexNorm> vData;
    std::vector<unsigned int> iData;
    Vector3f btmCenter(0, -halfHeight, 0);
    Vector3f topCenter(0, halfHeight, 0);
    
    for (int i = 0; i < radialSamples; ++i)
    {
        Vector3f v1top, v2top, v1btm, v2btm;
        
        v1top = topRing[i];
        v1btm = btmRing[i];
        if (i == radialSamples - 1)
        {
            v2top = topRing[0];
            v2btm = btmRing[0];
        }
        else
        {
            v2top = topRing[i+1];
            v2btm = btmRing[i+1];
        }
        
        Vector3f edge1 = v1btm - v1top;
        Vector3f edge2 = v2top - v1top;
        Vector3f normal = edge2.UnitCross(edge1);
        
        vData.push_back(VertexNorm(v1top, normal));
        vData.push_back(VertexNorm(v1btm, normal));
        vData.push_back(VertexNorm(v2top, normal));
        vData.push_back(VertexNorm(v2btm, normal));
        vData.push_back(VertexNorm(topCenter, Vector3f::UNIT_Y));
        vData.push_back(VertexNorm(btmCenter, -Vector3f::UNIT_Y));
        
        int index = 6*i;
        //2 side triangles
        iData.push_back(index);
        iData.push_back(index+2);
        iData.push_back(index+1);
        
        iData.push_back(index+1);
        iData.push_back(index+2);
        iData.push_back(index+3);
        
        //top triangle
        iData.push_back(index+4);
        iData.push_back(index+2);
        iData.push_back(index);
        
        //btm triangles
        iData.push_back(index+5);
        iData.push_back(index+1);
        iData.push_back(index+3);
        //close bottom ring
    }
    
    SetVertexBuffer(std::make_shared<ES2VertexHardwareBuffer>(sizeof(VertexNorm), vData.size(), vData.data()));
    SetIndexBuffer(std::make_shared<ES2IndexHardwareBuffer>(iData.size(), iData.data()));
    float maxRad = mTopRadius > mBtmRadius ? mTopRadius : mBtmRadius;
    LocalBoundBox = AxisAlignedBBox(Vector3f(-maxRad, -halfHeight, -maxRad), Vector3f(maxRad, halfHeight, maxRad));
}

ES2CylinderSPtr ES2Cylinder::Create(int axisSamples, int radialSamples, float topRadius, float btmRadius, float height, bool open)
{
    ES2CylinderSPtr ptr = std::make_shared<ES2Cylinder>();
    ptr->Init(axisSamples, radialSamples, topRadius, btmRadius, height, open);
    return ptr;
}

float ES2Cylinder::GetHeight() const
{
    return mHeight;
}

void ES2Cylinder::SetHeight(float height)
{
    mHeight = height;
}

//void ES2Cylinder::UpdateHeightTo(float height)
//{
////    assert(height > 0.0f);
//    if (height <= 0.0f)
//    {
//        return;
//    }
//    
//    float scaleRatio = height / mHeight;
//    mHeight = height;
//    RA_LOG_INFO("HEIGGGG %f", mHeight);
//    Transform sTransform;
//    sTransform.SetScale(APoint(1, scaleRatio, 1));
//    RA_LOG_INFO("Scle %f", scaleRatio);
//    LocalTransform = sTransform * LocalTransform;
//}
//
//void ES2Cylinder::UpdateHeightBy(float heightDelta)
//{
//    float height = mHeight + heightDelta;
//    UpdateHeightTo(height);
//}

//----------------------------------------------------------------------------------
void ES2Sphere::Init(ES2SphereSPtr sphere)
{
    ES2DefaultTriMesh::Init();
    SetVertexBuffer(sphere->GetVertexBuffer());
    SetIndexBuffer(sphere->GetIndexBuffer());
    LocalBoundBox = sphere->LocalBoundBox;
    mRadius = sphere->GetRadius();
    mColor = sphere->mColor;
}
    
void ES2Sphere::Init(int zSamples, int radialSamples, float radius)
{
    ES2DefaultTriMesh::Init();
    
    using namespace Wm5;
    
    mRadius = radius;
    int zsm1 = zSamples-1, zsm2 = zSamples-2, zsm3 = zSamples-3;
    int rsp1 = radialSamples+1;
    int numVertices = zsm2*rsp1 + 2;
    int numTriangles = 2*zsm2*radialSamples;
    int numIndices = 3*numTriangles;
    int stride = sizeof(VertexNorm);
    bool mHasNormals = true;
    bool mInside = false;
    bool mHasTCoords = false;
    
    // Create a vertex buffer.
    ES2VertexHardwareBufferSPtr vbuffer = std::make_shared<RAEngine::ES2VertexHardwareBuffer>(stride, numVertices, nullptr, HardwareBuffer::BU_STATIC);
    
    VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), vbuffer.get());
    vba.Map(HardwareBuffer::BL_READ_WRITE);
    
    // Generate geometry.
    float invRS = 1.0f/(float)radialSamples;
    float zFactor = 2.0f/(float)zsm1;
    int r, z, zStart, i;
    Wm5::Float2 tcoord;
    
    // Generate points on the unit circle to be used in computing the mesh
    // points on a cylinder slice.
    float* sn = new1<float>(rsp1);
    float* cs = new1<float>(rsp1);
    for (r = 0; r < radialSamples; ++r)
    {
        float angle = Wm5::Mathf::TWO_PI*invRS*r;
        cs[r] = Wm5::Mathf::Cos(angle);
        sn[r] = Wm5::Mathf::Sin(angle);
    }
    sn[radialSamples] = sn[0];
    cs[radialSamples] = cs[0];
    
    // Generate the cylinder itself.
    for (z = 1, i = 0; z < zsm1; ++z)
    {
        float zFraction = -1.0f + zFactor*z;  // in (-1,1)
        float zValue = radius*zFraction;
        
        // Compute center of slice.
        APoint sliceCenter(0.0f, 0.0f, zValue);
        
        // Compute radius of slice.
        float sliceRadius = Mathf::Sqrt(Mathf::FAbs(
                                                    radius*radius - zValue*zValue));
        
        // Compute slice vertices with duplication at endpoint.
        AVector normal;
        int save = i;
        for (r = 0; r < radialSamples; ++r)
        {
            float radialFraction = r*invRS;  // in [0,1)
            AVector radial(cs[r], sn[r], 0.0f);
            
            vba.Position<Float3>(i) = sliceCenter + sliceRadius*radial;
            
            if (mHasNormals)
            {
                normal = vba.Position<Float3>(i);
                normal.Normalize();
                if (mInside)
                {
                    vba.Normal<Float3>(i) = -normal;
                }
                else
                {
                    vba.Normal<Float3>(i) = normal;
                }
            }
            
            if (mHasTCoords)
            {
                tcoord[0] = radialFraction;
                tcoord[1] = 0.5f*(zFraction + 1.0f);
                vba.TCoord<Float2>(i) = tcoord;
            }
            
            ++i;
        }
        
        vba.Position<Float3>(i) = vba.Position<Float3>(save);
        if (mHasNormals)
        {
            vba.Normal<Float3>(i) = vba.Normal<Float3>(save);
        }
        
        if (mHasTCoords)
        {
            tcoord[0] = 1.0f;
            tcoord[1] = 0.5f*(zFraction + 1.0f);
            vba.TCoord<Float2>(i) = tcoord;
        }
        ++i;
    }
    
    // south pole
    vba.Position<Float3>(i) = Float3(0.0f, 0.0f, -radius);
    if (mHasNormals)
    {
        if (mInside)
        {
            vba.Normal<Float3>(i) = Float3(0.0f, 0.0f, 1.0f);
        }
        else
        {
            vba.Normal<Float3>(i) = Float3(0.0f, 0.0f, -1.0f);
        }
    }

    if (mHasTCoords)
    {
        tcoord = Float2(0.5f, 0.5f);
        vba.TCoord<Float2>(i) = tcoord;
    }
    ++i;
    
    // north pole
    vba.Position<Float3>(i) = Float3(0.0f, 0.0f, radius);
    if (mHasNormals)
    {
        if (mInside)
        {
            vba.Normal<Float3>(i) = Float3(0.0f, 0.0f, -1.0f);
        }
        else
        {
            vba.Normal<Float3>(i) = Float3(0.0f, 0.0f, 1.0f);
        }
    }

    if (mHasTCoords)
    {
        tcoord = Float2(0.5f, 1.0f);
        vba.TCoord<Float2>(i) = tcoord;
    }
    ++i;
    
    vba.Unmap();

    // Generate indices.
    ES2IndexHardwareBufferSPtr ibuffer(std::make_shared<ES2IndexHardwareBuffer>(numIndices, nullptr, HardwareBuffer::BU_STATIC));
    uint32_t* indices = static_cast<uint32_t*>(ibuffer->MapWrite());
    
    for (z = 0, zStart = 0; z < zsm3; ++z)
    {
        int i0 = zStart;
        int i1 = i0 + 1;
        zStart += rsp1;
        int i2 = zStart;
        int i3 = i2 + 1;
        for (i = 0; i < radialSamples; ++i, indices += 6)
        {
            if (mInside)
            {
                indices[0] = i0++;
                indices[1] = i2;
                indices[2] = i1;
                indices[3] = i1++;
                indices[4] = i2++;
                indices[5] = i3++;
            }
            else  // inside view
            {
                indices[0] = i0++;
                indices[1] = i1;
                indices[2] = i2;
                indices[3] = i1++;
                indices[4] = i3++;
                indices[5] = i2++;
            }
        }
    }
    
    // south pole triangles
    int numVerticesM2 = numVertices - 2;
    for (i = 0; i < radialSamples; ++i, indices += 3)
    {
        if (mInside)
        {
            indices[0] = i;
            indices[1] = i + 1;
            indices[2] = numVerticesM2;
        }
        else
        {
            indices[0] = i;
            indices[1] = numVerticesM2;
            indices[2] = i + 1;
        }
    }
    
    // north pole triangles
    int numVerticesM1 = numVertices-1, offset = zsm3*rsp1;
    for (i = 0; i < radialSamples; ++i, indices += 3)
    {
        if (mInside)
        {
            indices[0] = i + offset;
            indices[1] = numVerticesM1;
            indices[2] = i + 1 + offset;
        }
        else
        {
            indices[0] = i + offset;
            indices[1] = i + 1 + offset;
            indices[2] = numVerticesM1;
        }
    }
    
    delete1(cs);
    delete1(sn);
    ibuffer->Unmap();

    LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
    vbuffer->Unmap();
    SetVertexBuffer(vbuffer);
	ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	SetVertexArray(varray);
    SetIndexBuffer(ibuffer);
}

ES2SphereSPtr ES2Sphere::Create(ES2SphereSPtr sphere)
{
    ES2SphereSPtr sptr = std::make_shared<ES2Sphere>();
    sptr->Init(sphere);
    return sptr;
}
    
ES2SphereSPtr ES2Sphere::Create(int zSamples, int radialSamples, float radius)
{
    ES2SphereSPtr sptr = std::make_shared<ES2Sphere>();
    sptr->Init(zSamples, radialSamples, radius);
    return sptr;
}

float ES2Sphere::GetRadius() const
{
    return mRadius;
}

//----------------------------------------------------------------------------------
    
void ES2Box::Init (float xyzExtent)
{
    Init(xyzExtent, xyzExtent, xyzExtent);
}

void ES2Box::Init (float xExtent, float yExtent, float zExtent)
{
    ES2DefaultTriMesh::Init();
    
    int numVertices = 8;
    int numTriangles = 12;
    int numIndices = 3*numTriangles;
    int stride = sizeof(VertexNorm);
    bool mHasNormals = true;
//    bool mInside = false;
    bool mHasTCoords = false;

    // Create a vertex buffer.
    ES2VertexHardwareBufferSPtr vbuffer = std::make_shared<ES2VertexHardwareBuffer>(stride, numVertices, nullptr, HardwareBuffer::BU_STATIC);
    VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), vbuffer.get());
    vba.Map(HardwareBuffer::BL_READ_WRITE);
    
    // Generate geometry.
    using Wm5::Float3;
    using Wm5::Float2;
    
    vba.Position<Float3>(0) = Float3(-xExtent, -yExtent, -zExtent);
    vba.Position<Float3>(1) = Float3(+xExtent, -yExtent, -zExtent);
    vba.Position<Float3>(2) = Float3(+xExtent, +yExtent, -zExtent);
    vba.Position<Float3>(3) = Float3(-xExtent, +yExtent, -zExtent);
    vba.Position<Float3>(4) = Float3(-xExtent, -yExtent, +zExtent);
    vba.Position<Float3>(5) = Float3(+xExtent, -yExtent, +zExtent);
    vba.Position<Float3>(6) = Float3(+xExtent, +yExtent, +zExtent);
    vba.Position<Float3>(7) = Float3(-xExtent, +yExtent, +zExtent);

    if (mHasNormals)
    {
        vba.Normal<Float3>(0) = Float3(-xExtent, -yExtent, -zExtent);
        vba.Normal<Float3>(1) = Float3(+xExtent, -yExtent, -zExtent);
        vba.Normal<Float3>(2) = Float3(+xExtent, +yExtent, -zExtent);
        vba.Normal<Float3>(3) = Float3(-xExtent, +yExtent, -zExtent);
        vba.Normal<Float3>(4) = Float3(-xExtent, -yExtent, +zExtent);
        vba.Normal<Float3>(5) = Float3(+xExtent, -yExtent, +zExtent);
        vba.Normal<Float3>(6) = Float3(+xExtent, +yExtent, +zExtent);
        vba.Normal<Float3>(7) = Float3(-xExtent, +yExtent, +zExtent);
    }

    if (mHasTCoords)
    {
        vba.TCoord<Float2>(0) = Float2(0.25f, 0.75f);
        vba.TCoord<Float2>(1) = Float2(0.75f, 0.75f);
        vba.TCoord<Float2>(2) = Float2(0.75f, 0.25f);
        vba.TCoord<Float2>(3) = Float2(0.25f, 0.25f);
        vba.TCoord<Float2>(4) = Float2(0.0f, 1.0f);
        vba.TCoord<Float2>(5) = Float2(1.0f, 1.0f);
        vba.TCoord<Float2>(6) = Float2(1.0f, 0.0f);
        vba.TCoord<Float2>(7) = Float2(0.0f, 0.0f);
    }
    
    vba.Unmap();
    
    // Generate indices (outside view).
    ES2IndexHardwareBufferSPtr ibuffer(std::make_shared<ES2IndexHardwareBuffer>(numIndices, nullptr, HardwareBuffer::BU_STATIC));
    uint32_t* indices = static_cast<uint32_t*>(ibuffer->MapWrite());

    indices[ 0] = 0;  indices[ 1] = 2;  indices[ 2] = 1;
    indices[ 3] = 0;  indices[ 4] = 3;  indices[ 5] = 2;
    indices[ 6] = 0;  indices[ 7] = 1;  indices[ 8] = 5;
    indices[ 9] = 0;  indices[10] = 5;  indices[11] = 4;
    indices[12] = 0;  indices[13] = 4;  indices[14] = 7;
    indices[15] = 0;  indices[16] = 7;  indices[17] = 3;
    indices[18] = 6;  indices[19] = 4;  indices[20] = 5;
    indices[21] = 6;  indices[22] = 7;  indices[23] = 4;
    indices[24] = 6;  indices[25] = 5;  indices[26] = 1;
    indices[27] = 6;  indices[28] = 1;  indices[29] = 2;
    indices[30] = 6;  indices[31] = 2;  indices[32] = 3;
    indices[33] = 6;  indices[34] = 3;  indices[35] = 7;
    
    ibuffer->Unmap();

//    LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
//    vbuffer->Unmap();
    
    LocalBoundBox = AxisAlignedBBox(Vector3f(xExtent, yExtent, zExtent), Vector3f(-xExtent, -yExtent, -zExtent));

    SetVertexBuffer(vbuffer);
    SetIndexBuffer(ibuffer);
}
    
ES2BoxSPtr ES2Box::Create(float xyzExtent)
{
    ES2BoxSPtr ptr = std::make_shared<ES2Box>();
    ptr->Init(xyzExtent);
    return ptr;
}
    
ES2BoxSPtr ES2Box::Create(float xExtent, float yExtent, float zExtent)
{
    ES2BoxSPtr ptr = std::make_shared<ES2Box>();
    ptr->Init(xExtent, yExtent, zExtent);
    return ptr;
}
    
//----------------------------------------------------------------------------------

void ES2Line::Init(bool isContegious)
{
    ES2DefaultPolyline::Init(isContegious);
}

void ES2Line::Init(const Wm5::Vector3f* points, int numOfPoints, bool isContegious)
{
    ES2DefaultPolyline::Init(isContegious);
    float stride = sizeof(Vector3f);
    ES2VertexHardwareBufferSPtr vbuffer = std::make_shared<ES2VertexHardwareBuffer>(stride, numOfPoints, points, HardwareBuffer::BU_STATIC);
    SetVertexBuffer(vbuffer);
	ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	SetVertexArray(varray);
    LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
    vbuffer->Unmap();
}
    
ES2LineSPtr ES2Line::Create(bool isContegious)
{
    ES2LineSPtr ptr =  std::make_shared<ES2Line>();
    ptr->Init(isContegious);
    return ptr;
}
    
ES2LineSPtr ES2Line::Create(const Wm5::Vector3f* points, int numOfPoints, bool isContegious)
{
    ES2LineSPtr ptr =  std::make_shared<ES2Line>();
    ptr->Init(points, numOfPoints, isContegious);
    return ptr;
}

void ES2Line::SetPoints(const Wm5::Vector3f* points, int numOfPoints)
{
    float stride = sizeof(Vector3f);
    ES2VertexHardwareBufferSPtr vbuffer = std::make_shared<ES2VertexHardwareBuffer>(stride, numOfPoints, points, HardwareBuffer::BU_STATIC);
    SetVertexBuffer(vbuffer);
    
    LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
    vbuffer->Unmap();
}

//-------------------------------------------------------------------

void ES2Rectangle::Init(int xSamples, int ySamples, float xExtent, float yExtent)
{
    ES2DefaultTriMesh::Init();
    int numVertices = xSamples*ySamples;
    int numTriangles = 2*(xSamples-1)*(ySamples-1);
    int numIndices = 3*numTriangles;
    int stride = sizeof(VertexNorm);
    bool mHasNormals = true;
//    bool mInside = false;
    bool mHasTCoords = false;
    
    // Create a vertex buffer.
    ES2VertexHardwareBufferSPtr vbuffer(std::make_shared<ES2VertexHardwareBuffer>(stride, numVertices, nullptr, HardwareBuffer::BU_STATIC));
    VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), vbuffer.get());
    vba.Map(HardwareBuffer::BL_READ_WRITE);
    
    // Generate geometry.
    float inv0 = 1.0f/(xSamples - 1.0f);
    float inv1 = 1.0f/(ySamples - 1.0f);
    float u, v, x, y;
    int i, i0, i1;
    
    using Wm5::Float3;
    using Wm5::Float2;
    
    for (i1 = 0, i = 0; i1 < ySamples; ++i1)
    {
        v = i1*inv1;
        y = (2.0f*v - 1.0f)*yExtent;
        for (i0 = 0; i0 < xSamples; ++i0, ++i)
        {
            u = i0*inv0;
            x = (2.0f*u - 1.0f)*xExtent;
            
            vba.Position<Float3>(i) = Float3(x, y, 0.0f);
            
            if (mHasNormals)
            {
                vba.Normal<Float3>(i) = Float3(0.0f, 0.0f, 1.0f);
            }
            
            Float2 tcoord(u, v);
            if (mHasTCoords)
            {
                vba.TCoord<Float2>(i) = tcoord;
            }
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
    
    LocalBoundBox = AxisAlignedBBox(Vector3f(xExtent, yExtent, 0), Vector3f(-xExtent, -yExtent, 0));
    
    SetVertexBuffer(vbuffer);
    SetIndexBuffer(ibuffer);
}

ES2RectangleSPtr ES2Rectangle::Create(int xSamples, int ySamples, float xExtent, float yExtent)
{
    ES2RectangleSPtr ptr = std::make_shared<ES2Rectangle>();
    ptr->Init(xSamples, ySamples, xExtent, yExtent);
    return ptr;
}
    
//-------------------------------------------------------------------
    
void ES2Vector::Init(const Wm5::Vector3f& point, const Wm5::Vector3f& vector, float length)
{
    ES2Line::Init(false);
    Vector3f points[2];
    points[0] = point;
    points[1] = point + length*vector.GetNormalized();
    SetPoints(points, 2);
}

ES2VectorSPtr ES2Vector::Create(const Wm5::Vector3f& point, const Wm5::Vector3f& vector, float length)
{
    ES2VectorSPtr ptr = std::make_shared<ES2Vector>();
    ptr->Init(point, vector, length);
    return ptr;
}

}