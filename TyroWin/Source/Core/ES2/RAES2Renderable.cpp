//
//  RAVisual.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2Renderable.h"
#include "Wm5TransformBox3.h"
#include "RAVisibleSet.h"

namespace RAEngine
{
    void ES2Renderable::Init(ES2PrimitiveType type)
    {
        Spatial::Init();
        mPrimitiveType = type;
    }
    
//    ES2RenderableSPtr ES2Renderable::Create(ES2PrimitiveType type)
//    {
//        ES2RenderableSPtr sptr = std::make_shared<ES2Renderable>();
//        sptr->Init(type);
//        return sptr;
//    }
		
    void ES2Renderable::UpdateBounds()
    {
        WorldBoundBox = AxisAlignedBBox::TransformAffine(WorldTransform, LocalBoundBox);
    }
    
    void ES2Renderable::GetVisibleSet (RAEngine::VisibleSet* visibleSet)
    {
        if (this->Visible)
        {
            Spatial::GetVisibleSet(visibleSet);
            visibleSet->Insert(this);
        }
    }
    
    void ES2Renderable::GetHitProxies(RAEngine::VisibleSet* visibleSet)
    {}
}
