//
//  RAES2LineMesh.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-27.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2Polyline.h"

namespace RAEngine
{
    void ES2Polyline::Init(bool isContigious)
    {
        ES2Renderable::Init(isContigious ? PT_LINE_STRIP : PT_LINES);
        mIsContigious = isContigious;
    }
    
//    ES2PolylineSPtr ES2Polyline::Create(bool isContigious)
//    {
//        ES2PolylineSPtr ptr = std::make_shared<ES2Polyline>();
//        ptr->Init(isContigious);
//        return ptr;
//    }
    
    int ES2Polyline::GetNumberOfSegments() const
    {
        int numOfElements = GetVertexBuffer()->GetNumOfVerticies();
        assert(numOfElements >= 2);

        if (mIsContigious)
        {
            return numOfElements - 1;
        }
        else
        {
            assert(numOfElements % 2 == 0);
            return numOfElements/2;
        }
    }
}