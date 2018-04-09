//
//  RAMath.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-09.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAMath.h"
#include "Wm5APoint.h"

using namespace Wm5;

namespace tyro
{
        int Math::project(const Wm5::Vector3f& world, const Wm5::Vector4i& viewport, const Wm5::HMatrix& matrix, Wm5::Vector3f& win)
        {
            APoint fV4 = matrix * APoint(world);
            
            if (fV4[3] == 0.0) return 0;
            fV4[0] /= fV4[3];
            fV4[1] /= fV4[3];
            fV4[2] /= fV4[3];
            /* Map x, y and z to range 0-1 */
            fV4[0] = fV4[0] * 0.5f + 0.5f;
            fV4[1] = fV4[1] * 0.5f + 0.5f;
            fV4[2] = fV4[2] * 0.5f + 0.5f;
            
            /* Map x,y to viewport */
            fV4[0] = fV4[0] * viewport[2] + viewport[0];
            fV4[1] = fV4[1] * viewport[3] + viewport[1];
            
            win[0] = fV4[0];
            win[1] = fV4[1];
            win[2] = fV4[2];
            return 1;
        }

        int Math::unProject(const Wm5::Vector3f& win, const Wm5::Vector4i& viewport, const Wm5::HMatrix& matrix, Wm5::Vector3f& world)
        {
            APoint inVec;
            inVec[0]=(win[0] - viewport[0])/viewport[2] * 2.0f - 1.0f;
            inVec[1]=(win[1] - viewport[1])/viewport[3] * 2.0f - 1.0f;
            inVec[2]=2.0f * win[2] - 1.0f;
            inVec[3]=1.0f;
            
            APoint outVec = matrix.Inverse() * inVec;
            
            if(outVec[3] == 0.0)
                return 0;
            
            outVec[3]=1.0f/outVec[3];
            world[0]=outVec[0]*outVec[3];
            world[1]=outVec[1]*outVec[3];
            world[2]=outVec[2]*outVec[3];
            return 1;

        }

//        void RAMath::getTranformationsFromTouch(const Wm5::Vector2f& touch1,
//                                               const Wm5::Vector2f& touch2,
//                                               Wm5::Matrix3f& A,
//                                               Wm5::Vector3f& t)
//        {
//            
//        }
//
//        void RAMath::getTranformations(const Wm5::Vector2f& p1,
//                                      const Wm5::Vector2f& p2,
//                                      const Wm5::Vector2f& p3,
//                                      Wm5::Matrix2f& A,
//                                      Wm5::Vector2f& t)
//        {
//            
//        }
    
}
