//
//  RABoundBox.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-29.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAEnginePrerequisites.h"
#include "Wm5AxisAlignedBox3.h"

namespace tyro
{
    class AxisAlignedBBox : public Wm5::AxisAlignedBox3<float>
    {
    public:
        AxisAlignedBBox();

        AxisAlignedBBox(const Wm5::Vector3f& min, const Wm5::Vector3f& max);
        
        AxisAlignedBBox(const float min[3], const float max[3]);
        
        // Compute the axis-aligned bounding box of the points.zz
        void ComputeExtremes (int numVectors, size_t stride, const void* vectors);

        void ComputeExtremesd (int numVectors, size_t stride, const void* vectors);

        static AxisAlignedBBox TransformAffine (const Wm5::Transform& transform, const AxisAlignedBBox& bound);
        
        void Merge(const AxisAlignedBBox& box);

        virtual ~AxisAlignedBBox();
    };
}