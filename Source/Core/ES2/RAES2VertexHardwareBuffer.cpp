//
//  RAES2VertexHardwareBuffer.cpp
//  PAM2
//
//  Created by Rinat Abdrashitov on 2014-06-11.
//  Copyright (c) 2014 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2VertexHardwareBuffer.h"
#include "RALogManager.h"
//#include <OpenGLES/ES2/gl.h>
//#include <OpenGLES/ES2/glext.h>
#include <GL/glew.h>

namespace tyro
{

ES2VertexHardwareBuffer::ES2VertexHardwareBuffer()
:
VertexHardwareBuffer()
{
}

ES2VertexHardwareBuffer::ES2VertexHardwareBuffer(GLsizei vertexSize, GLsizei numOfVerticies, const GLvoid* dataPtr)
:
ES2VertexHardwareBuffer(vertexSize, numOfVerticies, dataPtr, HardwareBuffer::BU_STATIC)
{
}

ES2VertexHardwareBuffer::ES2VertexHardwareBuffer(GLsizei vertexSize, GLsizei numOfVerticies, const GLvoid* dataPtr, HardwareBuffer::Usage usage)
:
VertexHardwareBuffer(vertexSize, numOfVerticies, usage)
{
    assert(vertexSize > 0);
    assert(numOfVerticies > 0);
    
    mVertexSize = vertexSize;
    mNumOfVerticies = numOfVerticies;
    
    glGenBuffers(1, &mName);
    GL_CHECK_ERROR;
    glBindBuffer(GL_ARRAY_BUFFER, mName);
    GL_CHECK_ERROR;
    
    glBufferData(GL_ARRAY_BUFFER,  // Initialize buffer contents
                 mSizeInBytes,  // Number of bytes to copy
                 dataPtr,          // Address of bytes to copy
                 GetGLUsage(mUsage));           // Hint: cache in GPU memory
    GL_CHECK_ERROR;
    
    assert(0 != mName);
}

void ES2VertexHardwareBuffer::_SerializeInit()
{
    assert(mVertexSize > 0);
    assert(mNumOfVerticies > 0);
    
    glGenBuffers(1, &mName);
    GL_CHECK_ERROR;
    glBindBuffer(GL_ARRAY_BUFFER, mName);
    GL_CHECK_ERROR;
    
    glBufferData(GL_ARRAY_BUFFER,  // Initialize buffer contents
                 mSizeInBytes,  // Number of bytes to copy
                 NULL,          // Address of bytes to copy
                 GetGLUsage(mUsage));           // Hint: cache in GPU memory
    GL_CHECK_ERROR;
    
    assert(0 != mName);
}

ES2VertexHardwareBuffer::~ES2VertexHardwareBuffer()
{
    // Delete buffer from current context
    if (0 != mName)
    {
        glDeleteBuffers(1, &mName); // Step 7
        GL_CHECK_ERROR;
    }
}

GLuint ES2VertexHardwareBuffer::GetName() const
{
    return mName;
}

void ES2VertexHardwareBuffer::EnableAttribute(GLuint index) const
{
    glEnableVertexAttribArray(index);
    GL_CHECK_ERROR;
}

void ES2VertexHardwareBuffer::DisableAttribute(GLuint index) const
{
    glDisableVertexAttribArray(index);
    GL_CHECK_ERROR;
}

void ES2VertexHardwareBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, mName);
    GL_CHECK_ERROR;
}

void ES2VertexHardwareBuffer::Unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GL_CHECK_ERROR;
}

void* ES2VertexHardwareBuffer::Map(LockOptions options)
{
    Bind();
    GLvoid* temp;
    
    if (options == BL_READ)
    {
        temp = (void*)glMapBufferRange(GL_ARRAY_BUFFER, 0, mSizeInBytes, GL_MAP_READ_BIT);
        GL_CHECK_ERROR;
    }
    else if (options == BL_WRITE)
    {
        temp = (void*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        GL_CHECK_ERROR;
    }
    else if (options == BL_READ_WRITE)
    {
        temp = (void*)glMapBufferRange(GL_ARRAY_BUFFER, 0, mSizeInBytes, GL_MAP_READ_BIT|GL_MAP_WRITE_BIT);
        GL_CHECK_ERROR;
    }
    else
    {
        RA_LOG_ERROR_ASSERT("Lock option is not supported");
        Unbind();
        temp = nullptr;
    }

    //dont unbind because Unmap will be caleed anyways
    return temp;
}

void* ES2VertexHardwareBuffer::MapRead() const
{
    Bind();
    GLvoid* temp;
    
    temp = (void*)glMapBufferRange(GL_ARRAY_BUFFER, 0, mSizeInBytes, GL_MAP_READ_BIT);
    GL_CHECK_ERROR;
    
    return temp;
}

void* ES2VertexHardwareBuffer::MapWrite()
{
    return Map(BL_WRITE);
}

void ES2VertexHardwareBuffer::Unmap() const
{
    Bind();
    bool success = glUnmapBuffer(GL_ARRAY_BUFFER);
    GL_CHECK_ERROR;
    assert(success);
}

void ES2VertexHardwareBuffer::WriteData(size_t offset, size_t length, const void* pSource)
{
    assert(offset + length <= mSizeInBytes);
    Bind();
    if (offset == 0 && length == mSizeInBytes)
    {
        glBufferData(GL_ARRAY_BUFFER, mSizeInBytes, pSource, GetGLUsage(mUsage));
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, offset, length, pSource);
        GL_CHECK_ERROR;
    }
}

void ES2VertexHardwareBuffer::PrepareToDraw(GLuint index,
                                    GLint count,
                                    GLsizeiptr offset,
                                    GLenum type,
                                    GLboolean normalized) const
{
    assert((0 < count) && (count <= 4));
    assert(offset < mVertexSize);
    assert(0 != mName);
    
    glVertexAttribPointer(index,               // Identifies the attribute to use
                          count,               // number of coordinates for attribute
                          type,                // data is floating point
                          normalized,          // no fixed point scaling
                          mVertexSize,			// total num bytes stored per vertex
                          (char*)NULL + offset);      // offset from start of each vertex to
    
	GL_CHECK_ERROR;
}

GLenum ES2VertexHardwareBuffer::GetGLUsage(HardwareBuffer::Usage usage) const
{
    switch(usage)
    {
        case HardwareBuffer::BU_STATIC:
            return GL_STATIC_DRAW;
        case HardwareBuffer::BU_DYNAMIC:
            return GL_DYNAMIC_DRAW;
        default:
            return GL_STATIC_DRAW;
    };
}



//void ES2VertexHardwareBuffer::DrawPreparedArrays(GLenum mode,
//                                           GLint first,
//                                           GLsizei count)
//{
//    glDrawArrays(mode, first, count);
//    GL_CHECK_ERROR;
//}

//void ES2VertexHardwareBuffer::DrawPreparedArraysIndicies(GLenum mode,
//                                                   GLenum dataType,
//                                                   GLsizei numIndcies)
//{
//    glDrawElements(mode, numIndcies, dataType, 0);
//    GL_CHECK_ERROR;
//}


}