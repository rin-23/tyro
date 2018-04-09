//
//  RATriOctree.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2014-10-15.
//  Copyright (c) 2014 Rinat Abdrashitov. All rights reserved.
//

#include "RATriOctree.h"
#include "Wm5IntrBox3Box3.h"
#include "Wm5APoint.h"
#include "Wm5IntrRay3Box3.h"

using Wm5::Vector3f;

namespace tyro
{

TriOctree::TriOctree(TriOctree* parent, const Wm5::Vector3f& origin, const Wm5::Vector3f& halfDimension, int maxDepth)
{
    mOrigin = origin;
    mHalfDimension = halfDimension;
    mMaxDepth = maxDepth;
    mDataIndicies = nullptr;
    mParent = parent;
    
    mLooseMin = Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
    mLooseMax = Vector3f(FLT_MIN, FLT_MIN, FLT_MIN);
    
    for (int i = 0; i < 8; ++i)
    {
        mChildren[i] = nullptr;
    }
}

TriOctree::~TriOctree()
{
    // Recursively destroy octants
    if (mDataIndicies != nullptr)
    {
        delete mDataIndicies;
    }
    
    for (int i = 0; i < 8; ++i)
    {
        if (mChildren[i] != nullptr)
        {
            delete mChildren[i];
        }
    }
}

const std::vector<TriOctree::IndexedTriangle>* TriOctree::GetDataIndicies() const
{
    return mDataIndicies;
}

void TriOctree::SetDataIndicies(std::vector<TriOctree::IndexedTriangle>* newIndicies)
{
    if (mDataIndicies)
    {
        delete mDataIndicies;
    }
    mDataIndicies = newIndicies;
}

bool TriOctree::PruneEmptySubtrees()
{
    if (mMaxDepth == 0)
    {
        return ((mDataIndicies == nullptr) || (mDataIndicies->empty())) ? true : false;
    }
    
    int numOfNonEmpty = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (mChildren[i] != nullptr)
        {
            if (!mChildren[i]->PruneEmptySubtrees())
            {
                numOfNonEmpty++;
            }
            else
            {
                delete mChildren[i];
                mChildren[i] = nullptr;
            }
        }
    }
    
    if (numOfNonEmpty > 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

// Determine which octant of the tree would contain 'point'
int TriOctree::_GetOctantContainingTriangle(const Wm5::Vector3f verticies[3]) const
{
    return _GetOctantContainingPoint((verticies[0] + verticies[1] + verticies[2])/3.0f);
}

int TriOctree::_GetOctantContainingPoint(Wm5::Vector3f&& point) const
{
    int oct = 0;
    if (point.X() >= mOrigin.X()) oct |= 4;
    if (point.Y() >= mOrigin.Y()) oct |= 2;
    if (point.Z() >= mOrigin.Z()) oct |= 1;
    return oct;
}

int TriOctree::_GetOctantContainingPoint(const Wm5::Vector3f& point) const
{
    int oct = 0;
    if (point.X() >= mOrigin.X()) oct |= 4;
    if (point.Y() >= mOrigin.Y()) oct |= 2;
    if (point.Z() >= mOrigin.Z()) oct |= 1;
    return oct;
}

void TriOctree::Insert(const Wm5::Vector3f vertecies[3], const TriOctree::IndexedTriangle& indexedTriangle)
{
    if (mMaxDepth == 0)
    {
        if (mDataIndicies == nullptr)
        {
            mDataIndicies = new std::vector<IndexedTriangle>();
            mLooseMin = vertecies[0];
            mLooseMax = vertecies[0];
        }
        mDataIndicies->push_back(indexedTriangle);
        
        //update loose bounds
        for (int i = 0; i < 3; ++i)
        {
            mLooseMin = mLooseMin.GetMin(vertecies[i]);
            mLooseMax = mLooseMax.GetMax(vertecies[i]);
        }
    }
    else
    {
        int octant = _GetOctantContainingTriangle(vertecies);
        
        if (mChildren[octant] == nullptr)
        {
            Vector3f newOrigin = mOrigin;
            newOrigin.X() += mHalfDimension.X() * (octant&4 ? .5f : -.5f);
            newOrigin.Y() += mHalfDimension.Y() * (octant&2 ? .5f : -.5f);
            newOrigin.Z() += mHalfDimension.Z() * (octant&1 ? .5f : -.5f);
            mChildren[octant] = new TriOctree(this, newOrigin, 0.5f * mHalfDimension, mMaxDepth - 1);
        }
        
        mChildren[octant]->Insert(vertecies, indexedTriangle);
    }
}

void TriOctree::UpdateLooseBounds()
{
    if (mMaxDepth == 0)
    {
        return;
    }
    
    for (int i = 0; i < 8; ++i)
    {
        if (mChildren[i] != nullptr)
        {
            mChildren[i]->UpdateLooseBounds();
        }
    }
    
    bool isFirst = true;
    for (int i = 0; i < 8; ++i)
    {
        if (mChildren[i] != nullptr)
        {
            if (isFirst)
            {
                isFirst = false;
                mLooseMin = mChildren[i]->GetMinLooseBound();
                mLooseMax = mChildren[i]->GetMaxLooseBound();
            }
            else
            {
                mLooseMin = mLooseMin.GetMin(mChildren[i]->GetMinLooseBound());
                mLooseMax = mLooseMax.GetMax(mChildren[i]->GetMaxLooseBound());
            }
        }
    }
}

Wm5::Box3f TriOctree::GetBox() const
{
    Vector3f selectionAxis[3] = {Vector3f::UNIT_X, Vector3f::UNIT_Y, Vector3f::UNIT_Z};
    return Wm5::Box3f(mOrigin, selectionAxis, mHalfDimension);
}

Wm5::Box3f TriOctree::GetLooseBox() const
{
    return Wm5::Box3f(mLooseMin, mLooseMax);
}

const Wm5::Vector3f& TriOctree::GetMaxLooseBound() const
{
    return mLooseMax;
}

const Wm5::Vector3f& TriOctree::GetMinLooseBound() const
{
    return mLooseMin;
}

void TriOctree::Print()
{
    if (mMaxDepth == 0 && mDataIndicies != nullptr)
    {
        printf("###############__New Cell__###############\n");
        for (int j = 0; j < mDataIndicies->size();++j)
        {
            IndexedTriangle indexedTriangle = (*mDataIndicies)[j];
            printf("%i %i %i\n",indexedTriangle.index1,indexedTriangle.index2, indexedTriangle.index3);
        }
        return;
    }
    
    for (int i = 0; i < 8; ++i)
    {
        if (mChildren[i] != nullptr)
        {
            mChildren[i]->Print();
        }
    }
}

bool TriOctree::DoesIntersectBox2(const Wm5::Vector2f& minBound, const Wm5::Vector2f& maxBound, const Wm5::HMatrix& matrix) const
{
    using namespace Wm5;
    //convert origin and axis to camera coordinates
    APoint a_mOrigin = APoint(mOrigin);
    APoint origin = matrix * a_mOrigin;
    AVector XAxis = matrix * (a_mOrigin + mHalfDimension[0]*AVector::UNIT_X) - origin;
    AVector YAxis = matrix * (a_mOrigin + mHalfDimension[1]*AVector::UNIT_Y) - origin;
    AVector ZAxis = matrix * (a_mOrigin + mHalfDimension[2]*AVector::UNIT_Z) - origin;
    
    Vector3f halfDimension(XAxis.Length(), YAxis.Length(), ZAxis.Length()) ;
    Vector3f axis[3] = {XAxis.GetNormalized(), YAxis.GetNormalized(), ZAxis.GetNormalized()};
    
    Box3f box1 = Box3f(origin, axis, halfDimension);
    
//    Box3f box1 = Box3f(matrix * APoint(mLooseMin), matrix*APoint(mLooseMax));
    
    //Convert 2d box into a 3d by making z-coordinates infinetly big
    Vector2f selectionCenter = (minBound + maxBound)/2;
    Vector3f selectionAxis[3] = {Vector3f::UNIT_X, Vector3f::UNIT_Y, Vector3f::UNIT_Z};
    Vector3f selectionDim(Mathf::FAbs(maxBound.X() - minBound.X())/2,
                          Mathf::FAbs(maxBound.Y() - minBound.Y())/2,
                          FLT_MAX/2);
    
    Box3f box2 = Box3f(Vector3f(selectionCenter.X(), selectionCenter.Y(), 0),
                       selectionAxis,
                       selectionDim);
    
    IntrBox3Box3<float> intrBox3(box1, box2);
    bool doesIntresect = intrBox3.Test();
    return doesIntresect;
}

void TriOctree::GetAllIntersectedSubtrees(const Wm5::Vector2f& minBound,
                                          const Wm5::Vector2f& maxBound,
                                          const Wm5::HMatrix& matrix,
                                          std::vector<TriOctree*>& intersected,
                                          std::vector<TriOctree*>& nonitersected)
{
    if (mMaxDepth == 0 && mDataIndicies != nullptr)
    {
        if (this->DoesIntersectBox2(minBound, maxBound, matrix))
        {
            intersected.push_back(this);
        }
        else
        {
            nonitersected.push_back(this);
        }
        return;
    }
    
    for (int i = 0; i < 8; ++i)
    {
        if (mChildren[i] != nullptr)
        {
            if (mChildren[i]->DoesIntersectBox2(minBound, maxBound, matrix))
            {
                mChildren[i]->GetAllIntersectedSubtrees(minBound, maxBound, matrix, intersected, nonitersected);
            }
            else
            {
                mChildren[i]->GetLeafsWithData(nonitersected);
            }
        }
    }
}

void TriOctree::GetLeafsWithData(std::vector<TriOctree*>& trees)
{
    if (mMaxDepth == 0 && mDataIndicies != nullptr)
    {
        trees.push_back(this);
        return;
    }
    
    for (int i = 0; i < 8; ++i)
    {
        if (mChildren[i] != nullptr)
        {
            mChildren[i]->GetLeafsWithData(trees);
        }
    }
}

void TriOctree::GetSubtrees(std::vector<const TriOctree*>& trees) const
{
    if (mMaxDepth == 0 && mDataIndicies != nullptr)
    {
        trees.push_back(this);
        return;
    }
    
    for (int i = 0; i < 8; ++i)
    {
        if (mChildren[i] != nullptr)
        {
            mChildren[i]->GetSubtrees(trees);
        }
    }
    
    trees.push_back(this);
}

const std::vector<TriOctree::IndexedTriangle>* TriOctree::GetDataFromSubtreesContainingPoint(const Wm5::Vector3f& point) const
{
    // If this node doesn't have a data point yet assigned
    // and it is a leaf, then we're done!
    if (mMaxDepth == 0)
    {
        if (mDataIndicies != nullptr && mDataIndicies->size() > 0)
        {
            return mDataIndicies;
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        int octant = _GetOctantContainingPoint(point);
        if (mChildren[octant] != nullptr)
        {
            return mChildren[octant]->GetDataFromSubtreesContainingPoint(point);
        }
        else
        {
            return nullptr;
        }
    }
}

void TriOctree::GetLeafsIntersectingRay(const Wm5::Ray3f& ray, std::vector<const TriOctree*>& leafs)
{
    std::vector<TriOctree*> trees;
    this->GetLeafsWithData(trees);
    
    for (TriOctree* octree : trees)
    {
        Wm5::IntrRay3Box3f intr(ray, octree->GetLooseBox());
        if (intr.Test())
        {
            leafs.push_back(octree);
        }
    }
}


//    void TriOctree::getAllIntersectedIndicies(const Wm5::Vector2f& minBound,
//                                              const Wm5::Vector2f& maxBound,
//                                              const Wm5::HMatrix& matrix,
//                                              std::vector<std::vector<TriOctree::IndexedTriangle>*>& intersected,
//                                              std::vector<std::vector<TriOctree::IndexedTriangle>*>& nonitersected) const
//    {
//        if (mMaxDepth == 0 && mDataIndicies != NULL) {
//            if (this->doesIntersectBox2(minBound, maxBound, matrix)) {
//                intersected.push_back(mDataIndicies);
//            } else {
//                nonitersected.push_back(mDataIndicies);
//            }
//            return;
//        }
//
//        for (int i = 0; i < 8; ++i)
//        {
//            if (mChildren[i] != NULL)
//            {
//                if (mChildren[i]->doesIntersectBox2(minBound, maxBound, matrix)) {
//                    mChildren[i]->getAllIntersectedIndicies(minBound, maxBound, matrix, intersected, nonitersected);
//                } else {
//                    mChildren[i]->getDataFromSubtrees(nonitersected);
//                }
//            }
//        }
//    }

//    void TriOctree::getDataFromSubtrees(std::vector<std::vector<TriOctree::IndexedTriangle>*>& data)
//    {
//        if (mMaxDepth == 0 && mDataIndicies != NULL) {
//            data.push_back(mDataIndicies);
//            return;
//        }
//
//        for (int i = 0; i < 8; ++i) {
//            if (mChildren[i] != NULL) {
//                mChildren[i]->getDataFromSubtrees(data);
//            }
//        }
//    }
}