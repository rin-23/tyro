// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

//----------------------------------------------------------------------------
template <typename Real>
AxisAlignedBox3<Real>::AxisAlignedBox3 ()
{
    mExtent = EXTENT_NULL;
}
//----------------------------------------------------------------------------
template <typename Real>
AxisAlignedBox3<Real>::AxisAlignedBox3 (Real xmin, Real xmax, Real ymin,
    Real ymax, Real zmin, Real zmax)
{
    Min[0] = xmin;
    Max[0] = xmax;
    Min[1] = ymin;
    Max[1] = ymax;
    Min[2] = zmin;
    Max[2] = zmax;
    mExtent = EXTENT_FINITE;
}
//----------------------------------------------------------------------------
template <typename Real>
AxisAlignedBox3<Real>::AxisAlignedBox3 (const Vector3<Real>& minBound,
                                        const Vector3<Real>& maxBound)
{
    Min[0] = minBound[0];
    Max[0] = maxBound[0];
    Min[1] = minBound[1];
    Max[1] = maxBound[1];
    Min[2] = minBound[2];
    Max[2] = maxBound[2];
    mExtent = EXTENT_FINITE;
}
//----------------------------------------------------------------------------
template <typename Real>
AxisAlignedBox3<Real>::AxisAlignedBox3 (const Real minBound[3], const Real maxBound[3])
{
    Min[0] = minBound[0];
    Max[0] = maxBound[0];
    Min[1] = minBound[1];
    Max[1] = maxBound[1];
    Min[2] = minBound[2];
    Max[2] = maxBound[2];
    mExtent = EXTENT_FINITE;
}
//----------------------------------------------------------------------------
template <typename Real>
AxisAlignedBox3<Real>::~AxisAlignedBox3 ()
{
}
//----------------------------------------------------------------------------
template <typename Real>
Vector3<Real> AxisAlignedBox3<Real>::GetCenter () const
{
    assert(mExtent == EXTENT_FINITE);
    Vector3<Real> center;
    center[0] = ((Real)0.5)*(Max[0] + Min[0]);
    center[1] = ((Real)0.5)*(Max[1] + Min[1]);
    center[2] = ((Real)0.5)*(Max[2] + Min[2]);
    return center;
}
//----------------------------------------------------------------------------
template <typename Real>
Wm5::Vector3<Real> AxisAlignedBox3<Real>::GetSize() const
{
    switch (mExtent)
    {
        case EXTENT_NULL:
            return Vector3f::ZERO;
            
        case EXTENT_FINITE:
            return Max - Min;
        
        default:
            return Vector3f::ZERO;
    }
}
//----------------------------------------------------------------------------
template <typename Real>
Wm5::Vector3<Real> AxisAlignedBox3<Real>::GetHalfSize() const
{
    switch (mExtent)
    {
        case EXTENT_NULL:
            return Vector3f::ZERO;
            
        case EXTENT_FINITE:
            return (Max - Min)*0.5;
            
        default:
            return Vector3f::ZERO;
    }
}
//----------------------------------------------------------------------------
template <typename Real>
Real AxisAlignedBox3<Real>::GetRadius() const
{
    assert(mExtent == EXTENT_FINITE);
    Vector3<Real> size = 0.5f * GetSize();
    return size.Length();
}
//----------------------------------------------------------------------------
template <typename Real>
void AxisAlignedBox3<Real>::SetMinimum(const Wm5::Vector3<Real>& vec)
{
    Min = vec;
}
//----------------------------------------------------------------------------
template <typename Real>
void AxisAlignedBox3<Real>::SetMaximum(const Wm5::Vector3<Real>& vec)
{
    Max = vec;
}
//----------------------------------------------------------------------------
template <typename Real>
void AxisAlignedBox3<Real>::SetExtents(const Wm5::Vector3<Real>& min, const Wm5::Vector3<Real>& max)
{
    Min = min;
    Max = max;
    mExtent = EXTENT_FINITE;
}
//----------------------------------------------------------------------------
template <typename Real>
const Wm5::Vector3<Real>& AxisAlignedBox3<Real>::GetMinimum() const
{
    return Min;
}
//----------------------------------------------------------------------------
template <typename Real>
const Wm5::Vector3<Real>& AxisAlignedBox3<Real>::GetMaximum() const
{
    return Max;
}
//----------------------------------------------------------------------------
template <typename Real>
void AxisAlignedBox3<Real>::MakeNull()
{
    mExtent = EXTENT_NULL;
}
//----------------------------------------------------------------------------
template <typename Real>
bool AxisAlignedBox3<Real>::isNull() const
{
   return (mExtent == EXTENT_NULL);
}
//----------------------------------------------------------------------------
template <typename Real>
void AxisAlignedBox3<Real>::Merge(const AxisAlignedBox3<Real>& rhs)
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
//----------------------------------------------------------------------------
template <typename Real>
bool AxisAlignedBox3<Real>::HasXOverlap (const AxisAlignedBox3& box) const
{
    return (Max[0] >= box.Min[0] && Min[0] <= box.Max[0]);
}
//----------------------------------------------------------------------------
template <typename Real>
bool AxisAlignedBox3<Real>::HasYOverlap (const AxisAlignedBox3& box) const
{
    return (Max[1] >= box.Min[1] && Min[1] <= box.Max[1]);
}
//----------------------------------------------------------------------------
template <typename Real>
bool AxisAlignedBox3<Real>::HasZOverlap (const AxisAlignedBox3& box) const
{
    return (Max[2] >= box.Min[2] && Min[2] <= box.Max[2]);
}
//----------------------------------------------------------------------------
template <typename Real>
bool AxisAlignedBox3<Real>::TestIntersection (const AxisAlignedBox3& box)
    const
{
    for (int i = 0; i < 3; i++)
    {
        if (Max[i] < box.Min[i] || Min[i] > box.Max[i])
        {
            return false;
        }
    }
    return true;
}
//----------------------------------------------------------------------------
template <typename Real>
bool AxisAlignedBox3<Real>::FindIntersection (const AxisAlignedBox3& box,
    AxisAlignedBox3& intersection) const
{
    int i;
    for (i = 0; i < 3; i++)
    {
        if (Max[i] < box.Min[i] || Min[i] > box.Max[i])
        {
            return false;
        }
    }

    for (i = 0; i < 3; i++)
    {
        if (Max[i] <= box.Max[i])
        {
            intersection.Max[i] = Max[i];
        }
        else
        {
            intersection.Max[i] = box.Max[i];
        }

        if (Min[i] <= box.Min[i])
        {
            intersection.Min[i] = box.Min[i];
        }
        else
        {
            intersection.Min[i] = Min[i];
        }
    }
    return true;
}
//----------------------------------------------------------------------------
template <typename Real>
void AxisAlignedBox3<Real>::ComputeVertices(Vector3<Real> vertex[8]) const
{
    assert(mExtent == EXTENT_FINITE);
    
    Vector3<Real> halfsize = GetHalfSize();
    Vector3<Real> extAxis0 = halfsize[0]*Vector3<Real>(1,0,0);
    Vector3<Real> extAxis1 = halfsize[1]*Vector3<Real>(0,1,0);
    Vector3<Real> extAxis2 = halfsize[2]*Vector3<Real>(0,0,1);
    
    Vector3<Real> Center = GetCenter();
    
    vertex[0] = Center - extAxis0 - extAxis1 - extAxis2;
    vertex[1] = Center + extAxis0 - extAxis1 - extAxis2;
    vertex[2] = Center + extAxis0 + extAxis1 - extAxis2;
    vertex[3] = Center - extAxis0 + extAxis1 - extAxis2;
    vertex[4] = Center - extAxis0 - extAxis1 + extAxis2;
    vertex[5] = Center + extAxis0 - extAxis1 + extAxis2;
    vertex[6] = Center + extAxis0 + extAxis1 + extAxis2;
    vertex[7] = Center - extAxis0 + extAxis1 + extAxis2;
}



