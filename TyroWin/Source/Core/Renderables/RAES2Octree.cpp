//
//  RAES2Octree.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-27.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2Octree.h"
#include "RAES2CoreVisualEffects.h"
#include "RATriOctree.h"

namespace RAEngine
{
    
void ES2Octree::Init()
{
    ES2DefaultPolyline::Init(false);
    SetName("ES2Octree");
}

ES2OctreeSPtr ES2Octree::Create()
{
    ES2OctreeSPtr ptr = std::make_shared<ES2Octree>();
    ptr->Init();
    return ptr;
}

int ES2Octree::CreateWithTriOctree(const TriOctree* triOctree)
{
    assert(triOctree);
    if (triOctree == nullptr)
    {
        return 0;
    }
    
    std::vector<const TriOctree*> trees = std::vector<const TriOctree*>();
    triOctree->GetSubtrees(trees);
    
    int numVertices = 8*trees.size();
    int numIndicies = 8*3*trees.size();
    
    VertexRGBA* vertexPositions = (VertexRGBA*)malloc(numVertices * sizeof(VertexRGBA));
    unsigned int* vertexIndicies = (unsigned int*)malloc(numIndicies * sizeof(unsigned int));

    for (int i = 0; i < trees.size(); i++)
    {
        const TriOctree* octr = trees[i];
        Wm5::Box3f box3 = octr->GetBox();
        Wm5::Vector3f verticies[8];
        box3.ComputeVertices(verticies);
        
        for (int j = 0; j < 8; j++)
        {
            int index = i*8 + j;
            verticies[j].copyTuple(vertexPositions[index].position);
            vertexPositions[index].color = Wm5::Vector4uc(1,0,0,1);
        }
        
        unsigned int ind0 = i*8;
        unsigned int ind1 = ind0 + 1;
        unsigned int ind2 = ind1 + 1;
        unsigned int ind3 = ind2 + 1;
        unsigned int ind4 = ind3 + 1;
        unsigned int ind5 = ind4 + 1;
        unsigned int ind6 = ind5 + 1;
        unsigned int ind7 = ind6 + 1;
        
        unsigned int indicies[8*3] = {ind0,ind1,ind1,ind2,ind2,ind3,ind3,ind0,  //bottom faces
                             ind4,ind5,ind5,ind6,ind6,ind7,ind7,ind4,  //top faces
                             ind0,ind4,ind1,ind5,ind2,ind6,ind3,ind7}; //sides
        memcpy(vertexIndicies + i*8*3, indicies, 8*3*sizeof(unsigned int));
    }
    
    
    //Load vertex buffer
    ES2VertexHardwareBufferSPtr vertexBuffer(std::make_shared<ES2VertexHardwareBuffer>(sizeof(VertexRGBA), numVertices, vertexPositions));
    SetVertexBuffer(vertexBuffer);
    free(vertexPositions);

    ES2IndexHardwareBufferSPtr indexBuffer(std::make_shared<ES2IndexHardwareBuffer>(numIndicies, vertexIndicies));
    SetIndexBuffer(indexBuffer);
    free(vertexIndicies);

    Wm5::Box3f box = triOctree->GetBox();
    Wm5::Vector3f center = box.Center;
    Wm5::Vector3f extents = box.Extent;
    LocalBoundBox = AxisAlignedBBox(center - extents, center + extents);
    
    return 1;
}
}