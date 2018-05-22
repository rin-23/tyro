//
//  DataStructures.h
//  Pelvic-iOS
//
//  Created by Rinat Abdrashitov on 12-10-22.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef Pelvic_iOS_GLStructures_h
#define Pelvic_iOS_GLStructures_h

#include "Wm5Vector2.h"
#include "Wm5Vector3.h"
#include "Wm5Vector4.h"
//#include <OpenGLES/gltypes.h>
#include <GL/glew.h>

//WM5 specific
struct VertexRGBA
{
    VertexRGBA() {};
    
    VertexRGBA(const Wm5::Vector3f& p, const Wm5::Vector4uc& c)
    {
        position = p;
        color = c;
    }
    
    Wm5::Vector3f position;
    Wm5::Vector4uc color;
};

struct VertexNorm
{
    VertexNorm() {}
    VertexNorm(const Wm5::Vector3f& p, const Wm5::Vector3f& n)
    {
        position = p;
        normal = n;
    }
    
    Wm5::Vector3f position;
    Wm5::Vector3f normal;
};

struct VertexNormAO
{
    VertexNormAO() {}
    VertexNormAO(const Wm5::Vector3f& p, const Wm5::Vector3f& n, double t_ao)
    {
        position = p;
        normal = n;
        ao = t_ao;
    }
    
    Wm5::Vector3f position;
    Wm5::Vector3f normal;
    double ao;
};

struct VertexGeneral
{
    Wm5::Vector3f position;
    Wm5::Vector3f normal;
    Wm5::Vector3f color;
};

struct WireframeGeneral
{
    Wm5::Vector3f position;
    Wm5::Vector3f color;
};

struct VertexTexture
{
    VertexTexture() {}
    VertexTexture(const Wm5::Vector3f& p, const Wm5::Vector2f& t)
    {
        position = p;
        texture = t;
    }
    
    Wm5::Vector3f position;
    Wm5::Vector2f texture;
};

struct TrianglePosNorm
{
    TrianglePosNorm(const VertexNorm* verticies)
    {
        v[0] = verticies[0];
        v[1] = verticies[1];
        v[2] = verticies[2];
    }

    Wm5::Vector3f GetCentroid() const
    {
        return (v[0].position + v[1].position +v[2].position)/3.0f;
    }
    
    VertexNorm v[3]; //verticies
};

//struct Wm5VertexNormRGBA
//{
//    Wm5::Vector3f position;
//    Wm5::Vector3f normal;
//    Wm5::Vector4uc color;
//} ;


//struct Wm5Triangle
//{
//    Wm5VertexNormRGBA v[3]; //verticies
//    Wm5::Vector3f GetCentroid() const {
//        return (v[0].position + v[1].position +v[2].position)/3.0f;
//    }
//};




#endif
