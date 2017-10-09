// Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.0 (2010/01/01)

//----------------------------------------------------------------------------
template <typename Real>
Sphere3<Real>::Sphere3 ()
{
}
//----------------------------------------------------------------------------
template <typename Real>
Sphere3<Real>::~Sphere3 ()
{
}
//----------------------------------------------------------------------------
template <typename Real>
Sphere3<Real>::Sphere3 (const Vector3<Real>& center, Real radius)
    :
    Center(center),
    Radius(radius)
{
}
//----------------------------------------------------------------------------
template <typename Real>
Sphere3<Real>::Sphere3 (const Box3<Real>& box)
{
    Vector3<Real> extAxis0 = box.Extent[0]*box.Axis[0];
    Vector3<Real> extAxis1 = box.Extent[1]*box.Axis[1];
    Vector3<Real> extAxis2 = box.Extent[2]*box.Axis[2];
    
    Vector3<Real> point = box.Center + extAxis0 + extAxis1 + extAxis2;
    Vector3<Real> dir = point - box.Center;
    Radius = dir.Length();
    Center = box.Center;
}
//----------------------------------------------------------------------------
