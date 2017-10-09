//
//  RAVisibleSet.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-22.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAVisibleSet.h"
#include "RASpatial.h"
#include <algorithm>

namespace RAEngine
{

VisibleSet::VisibleSet()
:
mNumVisible(0)
{
}

VisibleSet::VisibleSet(std::initializer_list<Spatial*> spatials)
:
mNumVisible(0)
{
    for (auto it = spatials.begin(); it != spatials.end(); ++it)
    {
        Insert(*it);
    }
}

VisibleSet::~VisibleSet ()
{}

void VisibleSet::Insert (Spatial* visible)
{
    assert(visible != nullptr);
    if (visible == nullptr)
    {
        return;
    }
    
    int size = (int)mVisible.size();
    if (mNumVisible < size)
    {
        mVisible[mNumVisible] = visible;
    }
    else
    {
        mVisible.push_back(visible);
    }
    ++mNumVisible;
}

// Remove a visible object from the set.
void VisibleSet::Remove (Spatial* visible)
{
    assert(visible != nullptr);
    if (visible == nullptr)
    {
        return;
    }
    
    for (int i = 0; i < mNumVisible; ++i)
    {
        Spatial* s = mVisible[i];
        if (s->GetID() == visible->GetID())
        {
            mVisible.erase(mVisible.begin() + i);
            mNumVisible--;
            return;
        }
    }
}

void VisibleSet::Sort(std::function<bool(Spatial* s1, Spatial* s2)> compare)
{
    std::sort(mVisible.begin(), mVisible.end(), compare);
}
}