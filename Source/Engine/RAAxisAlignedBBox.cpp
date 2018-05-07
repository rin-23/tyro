//
//  RABoundBox.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-29.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAAxisAlignedBBox.h"
#include "Wm5Transform.h"

using Wm5::Vector3d;
using Wm5::Vector3f;
using Wm5::HMatrix;
using Wm5::APoint;

namespace tyro
{

AxisAlignedBBox::AxisAlignedBBox() : AxisAlignedBox3()
{}

AxisAlignedBBox::AxisAlignedBBox(const Wm5::Vector3f& min, const Wm5::Vector3f& max)
:
AxisAlignedBox3(min, max)
{
 
}

AxisAlignedBBox::AxisAlignedBBox(const float min[3], const float max[3])
:
AxisAlignedBox3(min, max)
{
}

AxisAlignedBBox::~AxisAlignedBBox()
{}

void AxisAlignedBBox::Merge(const AxisAlignedBBox& rhs) 
{
    // Do nothing if rhs null, or this is infinite
    if (rhs.mExtent == EXTENT_NULL)
    {
        return;
    }
    // Otherwise if current null, just take rhs
    else if (mExtent == EXTENT_NULL)
    {
        SetExtents(rhs.Min, rhs.Max);
    }
    // Otherwise merge
    else
    {
        Vector3f min = Min;
        Vector3f max = Max;
        max.MakeCeil(rhs.Max);
        min.MakeFloor(rhs.Min);
        
        SetExtents(min, max);
    }
}

AxisAlignedBBox AxisAlignedBBox::TransformAffine (const Wm5::Transform& transform, const AxisAlignedBBox& bound)
{
    assert(!bound.isNull());
    
    Vector3f centre = bound.GetCenter();
    Vector3f halfSize = bound.GetHalfSize();
    
    HMatrix m = transform.Matrix();
    Vector3f newCentre = transform * APoint(centre);
    Vector3f newHalfSize(
                        fabsf(m[0][0]) * halfSize.X() + fabsf(m[0][1]) * halfSize.Y() + fabsf(m[0][2]) * halfSize.Z(),
                        fabsf(m[1][0]) * halfSize.X() + fabsf(m[1][1]) * halfSize.Y() + fabsf(m[1][2]) * halfSize.Z(),
                        fabsf(m[2][0]) * halfSize.X() + fabsf(m[2][1]) * halfSize.Y() + fabsf(m[2][2]) * halfSize.Z());
    
    AxisAlignedBBox newBox(newCentre - newHalfSize, newCentre + newHalfSize);
    return newBox;
}

//----------------------------------------------------------------------------
void AxisAlignedBBox::ComputeExtremes (int numVectors, size_t stride, const void* vectors)
{
    assert(numVectors > 0 && vectors);
    Wm5::Vector3f vmin, vmax;
    
    for (int j = 0; j < numVectors; ++j)
    {
        const Vector3f& vec = *(Vector3f*)((const char*)vectors + j*stride);
        if (j == 0)
        {
            vmin = vec;
            vmax = vmin;
            continue;
        }
        
        for (int i = 0; i < 3; ++i)
        {
            if (vec[i] < vmin[i])
            {
                vmin[i] = vec[i];
            }
            else if (vec[i] > vmax[i])
            {
                vmax[i] = vec[i];
            }
        }
    }
    SetExtents(vmin, vmax);
}
//----------------------------------------------------------------------------
void AxisAlignedBBox::ComputeExtremesd (int numVectors, size_t stride, const void* vectors)
{
    assert(numVectors > 0 && vectors);
    Wm5::Vector3d vmin, vmax;
    
    for (int j = 0; j < numVectors; ++j)
    {
        const Vector3d& vec = *(Vector3d*)((const char*)vectors + j*stride);
        if (j == 0)
        {
            vmin = vec;
            vmax = vmin;
            continue;
        }
        
        for (int i = 0; i < 3; ++i)
        {
            if (vec[i] < vmin[i])
            {
                vmin[i] = vec[i];
            }
            else if (vec[i] > vmax[i])
            {
                vmax[i] = vec[i];
            }
        }
    }
    SetExtents(Vector3f(vmin[0], vmin[1], vmin[2]),Vector3f(vmax[0], vmax[1], vmax[2]));
}
}