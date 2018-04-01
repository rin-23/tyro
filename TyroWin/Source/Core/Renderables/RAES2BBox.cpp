//
//  RAES2BBox.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-06.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2BBox.h"
#include "GLStructures.h"
#include "Wm5Box3.h"
#include "RAES2CoreVisualEffects.h"
#include "RAAxisAlignedBBox.h"

namespace tyro
{
    
void ES2BBox::Init()
{
    ES2DefaultPolyline::Init(false);

    const int numOfVerticies = 8;
    const int numOfIndicies = 8*3;
    
    //Create vertex buffer
    SetVertexBuffer(std::make_shared<ES2VertexHardwareBuffer>(sizeof(VertexRGBA), numOfVerticies, nullptr, HardwareBuffer::BU_DYNAMIC));
    SetIndexBuffer(std::make_shared<ES2IndexHardwareBuffer>(numOfIndicies, nullptr, HardwareBuffer::BU_DYNAMIC));

    SetName("ES2 BBox");
}

ES2BBoxSPtr ES2BBox::Create()
{
    ES2BBoxSPtr ptr = std::make_shared<ES2BBox>();
    ptr->Init();
    return ptr;
}

void ES2BBox::SetupWithBBox(const AxisAlignedBBox& aabb)
{
    const int numOfVerticies = 8;
    const int numOfIndicies = 8*3;
    
    Wm5::Vector3f verticies[numOfVerticies];
    
    Wm5::Box3f bbox(aabb.GetMinimum(), aabb.GetMaximum());
    bbox.ComputeVertices(verticies);

    VertexRGBA* vertexPositions = (VertexRGBA*) GetVertexBuffer()->Map(HardwareBuffer::BL_WRITE);
    for (int j = 0; j < 8; j++)
    {
        verticies[j].copyTuple(vertexPositions[j].position);
        vertexPositions[j].color = Wm5::Vector4uc(1,0,0,1);
    }
    GetVertexBuffer()->Unmap();

    unsigned int ind0 = 0;
    unsigned int ind1 = ind0 + 1;
    unsigned int ind2 = ind1 + 1;
    unsigned int ind3 = ind2 + 1;
    unsigned int ind4 = ind3 + 1;
    unsigned int ind5 = ind4 + 1;
    unsigned int ind6 = ind5 + 1;
    unsigned int ind7 = ind6 + 1;
    
    unsigned int indicies[numOfIndicies] = {ind0,ind1,ind1,ind2,ind2,ind3,ind3,ind0,  //bottom faces
                                            ind4,ind5,ind5,ind6,ind6,ind7,ind7,ind4,  //top faces
                                            ind0,ind4,ind1,ind5,ind2,ind6,ind3,ind7}; //sides

    unsigned int* iData = (unsigned int*) GetIndexBuffer()->Map(HardwareBuffer::BL_WRITE);
    memcpy(iData, indicies, sizeof(unsigned int)*numOfIndicies);
    GetIndexBuffer()->Unmap();
    
    LocalBoundBox = aabb;
}
}