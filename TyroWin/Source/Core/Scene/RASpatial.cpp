//
//  RASpatial.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RASpatial.h"
#include "RAVisibleSet.h"
#include "Wm5AVector.h"
#include "Wm5APoint.h"
#include "RAES2BBox.h"

namespace tyro
{
    ObjectID Spatial::mNextID = 1;
    
    void Spatial::Init()
    {
        LocalTransform.MakeIdentity();
        WorldTransform.MakeIdentity();
        Visible = true;
        MergeWithParentBBox = true;
        mObjectID = mNextID++;
        mName = "None";
        mBBox = nullptr;
        mShowBBox = false;
    }
    
    void Spatial::Update(bool initiator)
    {
        UpdateTransformations();
        UpdateBounds();
        if (initiator)
        {
            PropagateBoundToRoot();
        }
    }
    
    void Spatial::PropagateBoundToRoot()
    {
        SpatialSPtr mParentS = mParent.lock();
//        assert(mParentS);
        if (mParentS)
        {
            mParentS->UpdateBounds();
            mParentS->PropagateBoundToRoot();
        }
    }
    
    void Spatial::UpdateTransformations()
    {
        SpatialSPtr mParentS = mParent.lock();
//        assert(mParentS);
        if (mParentS)
        {
            WorldTransform = mParentS->WorldTransform * LocalTransform;
        }
        else
        {
            WorldTransform = LocalTransform;
        }
    }
    
    void Spatial::GetVisibleSet(VisibleSet* visibleSet)
    {
        if (mShowBBox)
        {
            //Update bbox
            if (mBBox == nullptr)
            {
                mBBox = ES2BBox::Create();
            }
            
            mBBox->SetupWithBBox(WorldBoundBox);
            
            visibleSet->Insert(mBBox.get());
        }
     }
    
    void Spatial::GetHitProxies(tyro::VisibleSet* visibleSet)
    {}
    
    Wm5::Vector3f Spatial::ConvertWorldToLocalPosition(const Wm5::Vector3f& pos) const
    {
        return WorldTransform.InverseTransform() * Wm5::APoint(pos);
    }
    
    Wm5::Vector3f Spatial::ConvertWorldToLocalOrientation(const Wm5::Vector3f& vec) const
    {
        return WorldTransform.GetRotate().Inverse() * Wm5::AVector(vec);
    }
    
    Wm5::Vector3f Spatial::ConvertLocalToWorldPosition(const Wm5::Vector3f& pos) const
    {
        return WorldTransform * Wm5::APoint(pos);
    }
    
    Wm5::Vector3f Spatial::ConvertLocalToWorldOrientation(const Wm5::Vector3f& vec) const
    {
        return WorldTransform.GetRotate() * Wm5::AVector(vec);
    }
}