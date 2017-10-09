//
//  RATriOctree.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2014-10-15.
//  Copyright (c) 2014 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include <vector>
#include "Wm5Vector2.h"
#include "Wm5Vector3.h"
#include "Wm5Vector4.h"
#include "Wm5HMatrix.h"
#include "Wm5AVector.h"
#include "Wm5Box3.h"
#include "Wm5Ray3.h"
#include "RAAxisAlignedBBox.h"
#include "GLStructures.h"

namespace RAEngine
{
    class TriOctree
    {
    public:
        
        struct IndexedTriangle
        {
            unsigned int index1, index2, index3;
        };
        
        TriOctree(TriOctree* parent, const Wm5::Vector3f& origin, const Wm5::Vector3f& halfDimension, int maxDepth);

        ~TriOctree();
        
        const std::vector<IndexedTriangle>* GetDataIndicies() const;
        
        const Wm5::Vector3f& GetMaxLooseBound() const;
        
        const Wm5::Vector3f& GetMinLooseBound() const;
        
        void SetDataIndicies(std::vector<IndexedTriangle>* newIndicies);
        
        bool PruneEmptySubtrees();
        
        void Insert(const Wm5::Vector3f vertecies[3], const IndexedTriangle& indexedTriangle);

        void UpdateLooseBounds(); //no leaf
        
        bool DoesIntersectBox2(const Wm5::Vector2f& minBound, const Wm5::Vector2f& maxBound, const Wm5::HMatrix& matrix) const;
        
        void GetAllIntersectedSubtrees(const Wm5::Vector2f& minBound,
                                       const Wm5::Vector2f& maxBound,
                                       const Wm5::HMatrix& matrix,
                                       std::vector<TriOctree*>& intersected,
                                       std::vector<TriOctree*>& nonitersected);
        
        void GetLeafsWithData(std::vector<TriOctree*>& trees);

        void GetSubtrees(std::vector<const TriOctree*>& trees) const;

        const std::vector<IndexedTriangle>* GetDataFromSubtreesContainingPoint(const Wm5::Vector3f& point) const;
        
        void GetLeafsIntersectingRay(const Wm5::Ray3f& ray, std::vector<const TriOctree*>& leafs);
        
        Wm5::Box3f GetBox() const;
        
        Wm5::Box3f GetLooseBox() const;
        
        void Print();
        
    private:
        Wm5::Vector3f mOrigin;         //The physical center of this node
        Wm5::Vector3f mHalfDimension;  //Half the width/height/depth of this node
        int mMaxDepth;
        
        //Loose octree bound
        Wm5::Vector3f mLooseMin;
        Wm5::Vector3f mLooseMax;
        
        TriOctree* mParent;
        TriOctree* mChildren[8]; //! Pointers to child octants
        std::vector<IndexedTriangle>* mDataIndicies;
        
        // Determine which octant of the tree would contain 'point'
        int _GetOctantContainingPoint(const Wm5::Vector3f& point) const;
        int _GetOctantContainingPoint(Wm5::Vector3f&& point) const;
        int _GetOctantContainingTriangle(const Wm5::Vector3f verticies[3]) const;
        
        /*
         Children follow a predictable pattern to make accesses simple.
         Here, - means less than 'origin' in that dimension, + means greater than.
         child:	0 1 2 3 4 5 6 7
         x:      - - - - + + + +
         y:      - - + + - - + +
         z:      - + - + - + - +
         */
    };
}
