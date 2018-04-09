//
//  OssaSceneNode.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-02-26.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RANode.h"

namespace tyro
{

void Node::Init()
{
    Spatial::Init();
}
    
NodeSPtr Node::Create()
{
    NodeSPtr ptr = std::make_shared<Node>();
    ptr->Init();
    return ptr;
}

Node::NodeID Node::AddChild(SpatialSPtr child)
{
    if (child == nullptr)
    {
        RA_LOG_ERROR_ASSERT("Trying to add nil node as a child");
        return -1;
    }
    
    if (child->GetParent())
    {
        RA_LOG_ERROR_ASSERT("Child already has a parent");
        return -1;
    }
    
    child->SetParent(std::static_pointer_cast<Spatial>(shared_from_this()));
    
    // Insert the child in the first available slot (if any).
    auto iter = mChildren.begin();
    for (int i = 0; iter != mChildren.end(); ++iter, ++i)
    {
        if (*iter == nullptr)
        {
            *iter = child;
            return i;
        }
    }
    
    // All slots are used, so append the child to the array.
    const int numChildren = (int)mChildren.size();
    mChildren.push_back(child);
    return numChildren;
}

Node::NodeID Node::RemoveChild(SpatialSPtr child)
{
//#warning actuallly delete the object from the vector, not just null it
    if (child)
    {
        auto iter = mChildren.begin();
        for (int i = 0; iter != mChildren.end(); ++iter, ++i)
        {
            SpatialSPtr obj = *iter;
            if (obj)
            {
                if (obj->GetID() == child->GetID())
                {
                    *iter = nullptr;
                    obj = nullptr;
                    return i;
                }
            }
        }
    }
    return -1;
}

Node::NodeID Node::RemoveChild(ObjectID childID)
{
    auto iter = mChildren.begin();
    for (int i = 0; iter != mChildren.end(); ++iter, ++i)
    {
        SpatialSPtr obj = *iter;
        if (obj)
        {
            if (obj->GetID() == childID)
            {
                *iter = nullptr;
                obj = nullptr;
                return i;
            }
        }
    }
    
    return -1;
}

void Node::RemoveChildAt(int i)
{
    if (0 <= i && i < GetNumChildren())
    {
        if (mChildren[i])
        {
            mChildren[i] = nullptr;
        }
        return;
    }
}

void Node::SetChild(Node::NodeID i, SpatialSPtr child)
{
    if (!child)
    {
        RA_LOG_WARN("Trying to add nil node as a child");
        assert(false);
        return;
    }
    
    if (child->GetParent())
    {
        RA_LOG_WARN("Child already has a parent");
        assert(false);
        return;
    }
    
    if (0 <= i && i < GetNumChildren())
    {
        mChildren[i] = nullptr;
        
        // Insert the new child in the slot.
        if (child)
        {
            child->SetParent(std::static_pointer_cast<Spatial>(shared_from_this()));
        }

        mChildren[i] = child;
        return;
    }
    
    // The index is out of range, so append the child to the array.
    if (child)
    {
        child->SetParent(std::static_pointer_cast<Spatial>(shared_from_this()));
    }
    mChildren.push_back(SpatialSPtr(child));
    return;
}

SpatialSPtr Node::GetChildAt(Node::NodeID index) const
{
    if (index >= 0 && index < GetNumChildren())
    {
        return mChildren[index];
    }
    return nullptr;
}

void Node::UpdateBounds()
{
    //create invalid bouding box (zero extents)
    WorldBoundBox.MakeNull();
    
    //expand bounding boxes
    for (auto& child : mChildren)
    {
        if (child && child->MergeWithParentBBox)
        {
            WorldBoundBox.Merge(child->WorldBoundBox);
        }
    }
}

void Node::UpdateTransformations()
{
    Spatial::UpdateTransformations();
    
    for (auto child : mChildren)
    {
        if (child)
        {
            child->Update(false);
        }
    }
}

void Node::GetVisibleSet (tyro::VisibleSet* visibleSet)
{
    if (this->Visible)
    {
        Spatial::GetVisibleSet(visibleSet);
        
        for (auto& child : mChildren)
        {
            if (child != nullptr && child->Visible)
            {
                child->GetVisibleSet(visibleSet);
            }
        }
    }
}

void Node::GetHitProxies(tyro::VisibleSet* visibleSet)
{
    if (this->Visible)
    {
        Spatial::GetHitProxies(visibleSet);
        
        for (auto& child : mChildren)
        {
            if (child && child->Visible)
            {
                child->GetHitProxies(visibleSet);
            }
        }
    }
}
}