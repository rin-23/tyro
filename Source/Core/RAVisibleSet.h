//
//  RAVisibleSet.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-22.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include <vector>
#include <assert.h>
#include <functional>
#include "RAEnginePrerequisites.h"

namespace tyro
{
    class VisibleSet
    {
    public:
        VisibleSet();
        
        VisibleSet(std::initializer_list<Spatial*> spatials);
        
        ~VisibleSet();
        
        // Access to the elements of the visible set.
        inline int GetNumVisible () const;
        
        inline const std::vector<Spatial*>& GetAllVisible () const;

        inline Spatial* GetVisible (int i) const;
        
        void Sort(std::function<bool(Spatial* s1, Spatial* s2)> compare);
        
        // Insert a visible object into the set.
        void Insert (Spatial* visible);
        
        // Remove a visible object from the set.
        void Remove (Spatial* visible);

        // Set the number of elements to zero.
        inline void Clear ();
    public:
        int mNumVisible;
        std::vector<Spatial*> mVisible;
    };
    
    inline int VisibleSet::GetNumVisible () const
    {
        return mNumVisible;
    }
    
    inline const std::vector<Spatial*>& VisibleSet::GetAllVisible () const
    {
        return mVisible;
    }
    
    inline Spatial* VisibleSet::GetVisible (int i) const
    {
        assert(0 <= i && i < mNumVisible);
        if (0 <= i && i < mNumVisible)
        {
            return mVisible[i];
        }
        return nullptr;
    }
    
    inline void VisibleSet::Clear ()
    {
        mNumVisible = 0;
    }
}


