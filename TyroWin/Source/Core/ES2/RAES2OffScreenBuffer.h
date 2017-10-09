//
//  RAES2OffScreenBuffer.h
//  PAM2
//
//  Created by Rinat Abdrashitov on 2014-07-07.
//  Copyright (c) 2014 Rinat Abdrashitov. All rights reserved.
//

#pragma once

//#include <OpenGLES/ES2/gl.h>
#include <GL/glew.h>

namespace RAEngine
{
    class ES2OffScreenBuffer
    {
    public:
        ES2OffScreenBuffer();
        ~ES2OffScreenBuffer();
        void CreateOffScreenBuffer(GLsizei width, GLsizei height);
        void Bind();
        void Unbind();
    private:
        GLuint frameBuffer;
        GLuint colorBuffer;
        GLuint depthBuffer;
    };
}


