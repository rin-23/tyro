//
//  RAES2Polypoint.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-02-25.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2Polypoint.h"

namespace RAEngine
{
    void ES2Polypoint::Init()
    {
        ES2Renderable::Init(PT_POINTS);
    }
    
//    ES2PolypointSPtr ES2Polypoint::Create()
//    {
//        ES2PolypointSPtr ptr = std::make_shared<ES2Polypoint>();
//        ptr->Init();
//        return ptr;
//    }

    int ES2Polypoint::GetNumberOfPoints()
    {
        if (GetIndexBuffer())
        {
            return GetIndexBuffer()->GetNumIndexes();
        }
        else
        {
            return GetVertexBuffer()->GetNumOfVerticies();
        }
    }
}