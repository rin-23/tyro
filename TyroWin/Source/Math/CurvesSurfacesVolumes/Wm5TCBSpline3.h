    // Geometric Tools, LLC
// Copyright (c) 1998-2014
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
//
// File Version: 5.0.2 (2011/07/23)

#ifndef WM5TCBSPLINE3_H
#define WM5TCBSPLINE3_H

#include "Wm5MathematicsLIB.h"
#include "Wm5MultipleCurve3.h"

namespace Wm5
{

template <typename Real>
class WM5_MATHEMATICS_ITEM TCBSpline3 : public MultipleCurve3<Real>
{
public:
    TCBSpline3() {}
    // Construction and destruction.  TCBSpline3 accepts responsibility for
    // deleting the input arrays.
    TCBSpline3 (int numSegments, Real* times, Vector3<Real>* points,
        Real* tension, Real* continuity, Real* bias);
    
    // Catmul spline. Approximate the times based on distances between
    // points
    TCBSpline3 (int numSegments, Real* times, Vector3<Real>* points);

    virtual ~TCBSpline3 ();

    const Vector3<Real>* GetPoints () const;
    const Real* GetTensions () const;
    const Real* GetContinuities () const;
    const Real* GetBiases () const;
    
    virtual Vector3<Real> GetPosition (Real t) const;
    virtual Vector3<Real> GetFirstDerivative (Real t) const;
    virtual Vector3<Real> GetSecondDerivative (Real t) const;
    virtual Vector3<Real> GetThirdDerivative (Real t) const;
    
    void SetPoint(int i, const Vector3<Real>& ctrl);
    Vector3<Real> GetPoint (int i) const;
    int GetNumberOfPoints() const;

public: //Serialization
    template<class Archive>
    void save(Archive & archive, std::uint32_t const version) const
    {
        archive(cereal::virtual_base_class<MultipleCurve3<Real>>(this));
        archive(cereal::binary_data(mPoints, sizeof(Vector3<Real>)*(mNumSegments+1)),
                cereal::binary_data(mTension, sizeof(Real)*(mNumSegments+1)),
                cereal::binary_data(mContinuity, sizeof(Real)*(mNumSegments+1)),
                cereal::binary_data(mBias, sizeof(Real)*(mNumSegments+1)),
                cereal::binary_data(mA, sizeof(Vector3<Real>)*(mNumSegments)),
                cereal::binary_data(mB, sizeof(Vector3<Real>)*(mNumSegments)),
                cereal::binary_data(mC, sizeof(Vector3<Real>)*(mNumSegments)),
                cereal::binary_data(mD, sizeof(Vector3<Real>)*(mNumSegments))
                );
    }
    
    template<class Archive>
    void load(Archive & archive, std::uint32_t const version)
    {
        archive(cereal::virtual_base_class<MultipleCurve3<Real>>(this));
        InitSerialized();
        archive(cereal::binary_data(mPoints, sizeof(Vector3<Real>)*(mNumSegments+1)),
                cereal::binary_data(mTension, sizeof(Real)*(mNumSegments+1)),
                cereal::binary_data(mContinuity, sizeof(Real)*(mNumSegments+1)),
                cereal::binary_data(mBias, sizeof(Real)*(mNumSegments+1)),
                cereal::binary_data(mA, sizeof(Vector3<Real>)*(mNumSegments)),
                cereal::binary_data(mB, sizeof(Vector3<Real>)*(mNumSegments)),
                cereal::binary_data(mC, sizeof(Vector3<Real>)*(mNumSegments)),
                cereal::binary_data(mD, sizeof(Vector3<Real>)*(mNumSegments))
                );
    }
protected:
    using MultipleCurve3<Real>::mNumSegments;
    using MultipleCurve3<Real>::mTimes;
    using MultipleCurve3<Real>::GetKeyInfo;
    using MultipleCurve3<Real>::GetSpeedWithData;

    void ComputePoly (int i0, int i1, int i2, int i3);
    void InitSerialized();
    virtual Real GetSpeedKey (int key, Real t) const;
    virtual Real GetLengthKey (int key, Real t0, Real t1) const;

    Vector3<Real>* mPoints;
    Real* mTension;
    Real* mContinuity;
    Real* mBias;
    Vector3<Real>* mA;
    Vector3<Real>* mB;
    Vector3<Real>* mC;
    Vector3<Real>* mD;

    class WM5_MATHEMATICS_ITEM SplineKey
    {
    public:
        SplineKey (const TCBSpline3* spline, int key);

        const TCBSpline3* Spline;
        int Key;
    };
};

typedef TCBSpline3<float> TCBSpline3f;
typedef TCBSpline3<double> TCBSpline3d;

}

#endif
