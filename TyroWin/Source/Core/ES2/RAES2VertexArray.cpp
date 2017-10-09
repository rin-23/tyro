//
//  RAES2VertexArray.cpp
//  PAM2
//
//  Created by Rinat Abdrashitov on 2014-06-23.
//  Copyright (c) 2014 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2VertexArray.h"
//#include <OpenGLES/ES2/glext.h>
#include <GL/glew.h>
#include "RALogManager.h"

namespace RAEngine
{
ES2VertexArray::ES2VertexArray()
{
    glGenVertexArrays(1, &mName);
    GL_CHECK_ERROR;
}

void ES2VertexArray::Bind() const
{
    glBindVertexArray(mName);
    GL_CHECK_ERROR;
}

void ES2VertexArray::Unbind()
{
    glBindVertexArray(0);
}

GLuint ES2VertexArray::GetName() const
{
    return mName;
}
}