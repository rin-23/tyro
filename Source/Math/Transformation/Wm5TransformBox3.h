//
//  Wm5Box3Transform.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-02.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#ifndef WM5TransformBOX3_H
#define WM5TransformBOX3_H

#include "Wm5MathematicsLIB.h"
#include "Wm5Box3.h"
#include "Wm5Transform.h"

namespace Wm5
{
    // Transform box
    Box3f TransformBox (const Box3f& box, const Transform& transform);
}

#endif /* defined(WM5TransformBOX3_H) */
