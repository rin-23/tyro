//
//  RAES2VertexArray.h
//  PAM2
//
//  Created by Rinat Abdrashitov on 2014-06-23.
//  Copyright (c) 2014 Rinat Abdrashitov. All rights reserved.
//

#pragma once

//#include <OpenGLES/gltypes.h>
#include <GL/glew.h>
#include "RAEnginePrerequisites.h"

namespace RAEngine
{
    class ES2VertexArray
    {
    public:
        ES2VertexArray(const ES2VisualEffectSPtr visualEffect, const ES2VertexHardwareBufferSPtr vertexBuffer);
		
		~ES2VertexArray();
        
		GLuint GetName() const;
        
        void Bind() const;
        
        static void Unbind();
    private:
        GLuint mName;
    };
}

