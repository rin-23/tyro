//
//  RAES2IndexHardwareBuffer.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2IndexHardwareBuffer.h"
#include "RALogManager.h"
#include <GL/glew.h>
//#include <OpenGLES/ES2/glext.h>

namespace tyro
{

ES2IndexHardwareBuffer::ES2IndexHardwareBuffer()
:
IndexHardwareBuffer()
{
}
 

ES2IndexHardwareBuffer::ES2IndexHardwareBuffer(GLsizei numIndexes, const GLvoid* dataPtr)
:
ES2IndexHardwareBuffer(numIndexes, dataPtr, HardwareBuffer::BU_STATIC)
{
}

ES2IndexHardwareBuffer::ES2IndexHardwareBuffer(GLsizei numIndexes, const GLvoid* dataPtr, HardwareBuffer::Usage usage)
:
ES2IndexHardwareBuffer(IndexType::IT_32BIT, numIndexes, dataPtr, usage)
{
}

ES2IndexHardwareBuffer::ES2IndexHardwareBuffer(IndexType indexType, GLsizei numIndexes, const GLvoid* dataPtr, HardwareBuffer::Usage usage)
:
IndexHardwareBuffer(indexType, numIndexes, usage)
{
    assert(numIndexes > 0);
    
    glGenBuffers(1, &mName);
    GL_CHECK_ERROR;
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mName);
    GL_CHECK_ERROR;
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,  // Initialize buffer contents
                 mSizeInBytes,  // Number of bytes to copy
                 dataPtr,
                 GetGLUsage(mUsage));
    GL_CHECK_ERROR;
    
    assert(0 != mName);
}

void ES2IndexHardwareBuffer::_SerializeInit()
{
    assert(mNumIndexes > 0);
    
    glGenBuffers(1, &mName);
    GL_CHECK_ERROR;
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mName);
    GL_CHECK_ERROR;
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,  // Initialize buffer contents
                 mSizeInBytes,  // Number of bytes to copy
                 NULL,
                 GetGLUsage(mUsage));
    GL_CHECK_ERROR;
    
    assert(0 != mName);
}

ES2IndexHardwareBuffer::~ES2IndexHardwareBuffer()
{
    // Delete buffer from current context
    if (0 != mName)
    {
        glDeleteBuffers(1, &mName); // Step 7
        GL_CHECK_ERROR;
    }
}

GLuint ES2IndexHardwareBuffer::GetName() const
{
    return mName;
}

void ES2IndexHardwareBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mName);
    GL_CHECK_ERROR;
}

void ES2IndexHardwareBuffer::Unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    GL_CHECK_ERROR;
}

void* ES2IndexHardwareBuffer::Map(LockOptions options)
{
    Bind();
    GLvoid* temp;
    
    if (options == BL_READ)
    {
        temp = (void*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, mSizeInBytes, GL_MAP_READ_BIT);
        GL_CHECK_ERROR;
    }
    else if (options == BL_WRITE)
    {
        temp = (void*) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        GL_CHECK_ERROR;
    }
    else if (options == BL_READ_WRITE)
    {
        temp = (void*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, mSizeInBytes, GL_MAP_READ_BIT|GL_MAP_WRITE_BIT);
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

void* ES2IndexHardwareBuffer::MapRead() const
{
    Bind();
    GLvoid* temp;
    
    temp = (void*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, mSizeInBytes, GL_MAP_READ_BIT);
    GL_CHECK_ERROR;

    return temp;
}

void* ES2IndexHardwareBuffer::MapWrite()
{
    return Map(BL_WRITE);
}

void ES2IndexHardwareBuffer::Unmap() const
{
    Bind();
    bool success = glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    GL_CHECK_ERROR;
    assert(success);
}

void ES2IndexHardwareBuffer::WriteData(size_t offset, size_t length, const void* pSource)
{
    assert(offset + length <= mSizeInBytes);
    Bind();
    if (offset == 0 && length == mSizeInBytes)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mSizeInBytes, pSource, GetGLUsage(mUsage));
    }
    else
    {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, length, pSource);
        GL_CHECK_ERROR;
    }
}

GLenum ES2IndexHardwareBuffer::GetGLUsage(HardwareBuffer::Usage usage) const
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
}