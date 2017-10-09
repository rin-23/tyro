//
//  RAMath.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-09.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include "Wm5Matrix4.h"
#include "Wm5Matrix3.h"
#include "Wm5Matrix2.h"
#include "Wm5HMatrix.h"

namespace RAEngine
{
    class Math
    {
    public:

        static inline float InchToMM(float inch)
        {
            return inch * 2.54 * 10;
        }
        
        static inline float RadToDeg(float rad)
        {
            return rad * (180.0f / M_PI );
        };
        
        static inline float DegToRad(float degrees)
        {
            return degrees * (M_PI / 180.0f);
        };

        //Helper method to emulate GLSL
        static inline float fract(float value)
        {
            return value - floorf(value);
        };

        //Helper method to go from a float to packed char
        static unsigned char ConvertChar(float value)
        {
            //Scale and bias
            value = (value + 1.0f) * 0.5f;
            return (unsigned char)(value*255.0f);
        }

        //Pack 3 values into 1 float
        static float PackToFloat(unsigned char x, unsigned char y, unsigned char z)
        {
            unsigned int packedColor = (x << 16) | (y << 8) | z;
            float packedFloat = (float) ( ((double)packedColor) / ((double) (1 << 24)) );
            
            return packedFloat;
        }

        //UnPack 3 values from 1 float
        static void UnPackFloat(float src, float &r, float &g, float &b)
        {
            r = fract(src);
            g = fract(src * 256.0f);
            b = fract(src * 65536.0f);
            
            //Unpack to the -1..1 range
            r = (r * 2.0f) - 1.0f;
            g = (g * 2.0f) - 1.0f;
            b = (b * 2.0f) - 1.0f;
        }

        ////Test pack/unpack 3 values
        static void DoTest(float r, float g, float b)
        {
            float outR, outG, outB;
            
            printf("Testing %f %f %f\n",r, g, b);
            
            //Pack
            float result = PackToFloat(ConvertChar(r), ConvertChar(g), ConvertChar(b));
            
            //Unpack
            UnPackFloat(result, outR, outG, outB);
            
            printf("Result %f %f %f\n",outR, outG, outB);
            printf("Diff   %f %f %f\n\n",r-outR, g-outG, b-outB);
        }

        static int project(const Wm5::Vector3f& world, const Wm5::Vector4i& viewport, const Wm5::HMatrix& matrix, Wm5::Vector3f& win);

        static int unProject(const Wm5::Vector3f& win, const Wm5::Vector4i& viewport, const Wm5::HMatrix& matrix, Wm5::Vector3f& world);

        //creates 32 bit float from a vector of unsinged char
        static float UnsginedCharToFloat(const Wm5::Vector4uc& color)
        {
            unsigned char r = color[0];
            unsigned char g = color[1];
            unsigned char b = color[2];
            unsigned char a = color[3];
            
            if (r != 0 && g != 0 && b != 0)
            {
                Wm5::Vector4f colorf = Wm5::Vector4f(r, g, b, a);
                float depth = colorf.Dot(Wm5::Vector4f(1.0, 1/255.0, 1/65025.0, 1/16581375.0));
                depth = depth/255.0;
                return depth;
            }
            return -1;
        }
        
        //Project vector onto the plane
        inline static Wm5::Vector3f ProjectOnPlane (Wm5::Vector3f v1, Wm5::Vector3f n)
        {
            return v1 - ProjectOnVector(v1, n);
        }
        
        inline static Wm5::Vector3f ProjectOnVector (Wm5::Vector3f v1, Wm5::Vector3f v2)
        {
            return (v1.Dot(v2)/v2.SquaredLength()) * v2;
        }
        
        inline static float AngleBetweenVectors (const Wm5::Vector3f& v1, const Wm5::Vector3f& v2)
        {
            return acosf(v1.Dot(v2)/(v1.Length()* v2.Length()));
        }

        
//        static void getTranformationsFromTouch(const Wm5::Vector2f& touch1,
//                                               const Wm5::Vector2f& touch2,
//                                               Wm5::Matrix3f& A,
//                                               Wm5::Vector3f& t);
//
//        static void getTranformations(const Wm5::Vector2f& p1,
//                                      const Wm5::Vector2f& p2,
//                                      const Wm5::Vector2f& p3,
//                                      Wm5::Matrix3f& A,
//                                      Wm5::Vector3f& t);
    };
}
