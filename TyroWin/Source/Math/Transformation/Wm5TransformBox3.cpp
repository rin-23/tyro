//
//  Wm5Box3Transform.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-02.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "Wm5TransformBox3.h"
#include "Wm5MathematicsPCH.h"

namespace Wm5
{
    Box3f TransformBox (const Box3f& box, const Transform& transform)
    {
        Box3f newBox;

        //@todo refactor, trouble with implicit conversion
        newBox.Center = (transform * APoint(box.Center)).operator Wm5::Vector3<float> &();
        
        newBox.Axis[0] = (transform.GetRotate() * AVector(box.Axis[0])).operator Wm5::Vector3<float> &();
        newBox.Axis[1] = (transform.GetRotate() * AVector(box.Axis[1])).operator Wm5::Vector3<float> &();
        newBox.Axis[2] = (transform.GetRotate() * AVector(box.Axis[2])).operator Wm5::Vector3<float> &();
        newBox.Extent[0] = transform.GetScale()[0] * box.Extent[0];
        newBox.Extent[1] = transform.GetScale()[1] * box.Extent[1];
        newBox.Extent[2] = transform.GetScale()[2] * box.Extent[2];
        
        return newBox;
    }
}