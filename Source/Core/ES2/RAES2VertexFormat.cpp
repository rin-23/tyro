//
//  RAES2VertexFormat.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-14.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2VertexFormat.h"
#include "RALogManager.h"
#include <cstring>
//#include <OpenGLES/ES2/gl.h>
//#include <OpenGLES/ES2/glext.h>
#include <GL/glew.h>

namespace tyro
{

int ES2VertexFormat::msComponentSize[AT_QUANTITY] =
{
    4,  // AT_FLOAT1
    4,  // AT_FLOAT2
    4,  // AT_FLOAT3
    4,  // AT_FLOAT4
    2,  // AT_HALF1
    2,  // AT_HALF2
    2,  // AT_HALF3
    2,  // AT_HALF4
    1,  // AT_UBYTE4
    2,  // AT_SHORT1
    2,  // AT_SHORT2
    2,  // AT_SHORT4
    4   // AT_INT1
};

int ES2VertexFormat::msNumComponents[AT_QUANTITY] =
{
    1,  // AT_FLOAT1
    2,  // AT_FLOAT2
    3,  // AT_FLOAT3
    4,  // AT_FLOAT4
    1,  // AT_HALF1
    2,  // AT_HALF2
    3,  // AT_HALF3
    4,  // AT_HALF4
    4,  // AT_UBYTE4
    1,  // AT_SHORT1
    2,  // AT_SHORT2
    4,   // AT_SHORT4
    1   // AT_INT4
};

int ES2VertexFormat::msTypeSize[AT_QUANTITY] =
{
    4,  // AT_FLOAT1
    8,  // AT_FLOAT2
    12, // AT_FLOAT3
    16, // AT_FLOAT4
    2,  // AT_HALF1
    4,  // AT_HALF2
    6,  // AT_HALF3
    8,  // AT_HALF4
    4,  // AT_UBYTE4
    2,  // AT_SHORT1
    4,  // AT_SHORT2
    8,   // AT_SHORT4
    4   // AT_INT1
};

GLenum ES2VertexFormat::msGLType[AT_QUANTITY] =
{
    GL_FLOAT,  // AT_FLOAT1
    GL_FLOAT,  // AT_FLOAT2
    GL_FLOAT, // AT_FLOAT3
    GL_FLOAT, // AT_FLOAT4
    GL_HALF_FLOAT,  // AT_HALF1
    GL_HALF_FLOAT,  // AT_HALF2
    GL_HALF_FLOAT,  // AT_HALF3
    GL_HALF_FLOAT,  // AT_HALF4
    GL_UNSIGNED_BYTE,  // AT_UBYTE4
    GL_SHORT,  // AT_SHORT1
    GL_SHORT,  // AT_SHORT2
    GL_SHORT,   // AT_SHORT4
    GL_INT   // AT_INT1
};

ES2VertexFormat::ES2VertexFormat(int numAttributes)
{
    mNumAttributes = numAttributes;
    mAttributes = new Attribute[numAttributes];
}

ES2VertexFormat::~ES2VertexFormat()
{
    delete [] mAttributes;
}

void ES2VertexFormat::SetAttribute(int index,
                                   int shaderLocation,
                                   GLsizeiptr offset,
                                   AttributeType type,
                                   AttributeUsage usage)
{
    if (index >= mNumAttributes)
    {
        assert(false);
        RA_LOG_ERROR("Attribute index %i is out of  max bound %i", index, mNumAttributes);
        return;
    }
    
    Attribute attribute;
    attribute.shaderLocation = shaderLocation;
    attribute.size = GetNumComponents(type);
    attribute.offset = offset;
    attribute.dataType = GetGLType(type);
    attribute.normalized = GL_FALSE;
    attribute.enabled = 1;
    attribute.usage = usage;
    attribute.atType = type;
    
    mAttributes[index] = attribute;
}

int ES2VertexFormat::GetNumOfAttributes() const
{
    return mNumAttributes;
}

int ES2VertexFormat::GetIndex(AttributeUsage usage) const
{
    for (int index = 0; index < mNumAttributes; index++)
    {
        const Attribute& attribute = mAttributes[index];
        if (attribute.usage == usage) {
            return index;
        }
    }
    
    return -1;
}

GLsizeiptr ES2VertexFormat::GetOffset(int index) const
{
    if (index >= mNumAttributes)
    {
        RA_LOG_ERROR("Attribute index %i is out of  max bound %i", index, mNumAttributes);
        assert(false);
        return -1;
    }
    
    const Attribute& attribute = mAttributes[index];
    return attribute.offset;
}

GLint ES2VertexFormat::GetSize(int index) const
{
    if (index >= mNumAttributes)
    {
        RA_LOG_ERROR("Attribute index %i is out of  max bound %i", index, mNumAttributes);
        assert(false);
        return -1;
    }
    
    const Attribute& attribute = mAttributes[index];
    return attribute.size;
}

GLsizeiptr ES2VertexFormat::GetOffsetForNextAttrib(int index) const
{
    
    if (index >= mNumAttributes)
    {
        RA_LOG_ERROR("Attribute index %i is out of  max bound %i", index, mNumAttributes);
        assert(false);
        return -1;
    }
    
    const Attribute& attribute = mAttributes[index];
    return GetOffset(index) + GetTypeSize(attribute.atType);
}

int ES2VertexFormat::GetShaderLocation(int index) const
{
    if (index >= mNumAttributes)
    {
        RA_LOG_ERROR("Attribute index %i is out of  max bound %i", index, mNumAttributes);
        assert(false);
        return -1;
    }
    
    const Attribute& attribute = mAttributes[index];
    return attribute.shaderLocation;
}

void ES2VertexFormat::GetGLAttribute(int index, int& shaderLocation, GLint& size, GLsizeiptr& offset, GLenum& dataType, GLboolean& normalized, GLboolean& enabled) const
{
    if (index >= mNumAttributes)
    {
        RA_LOG_ERROR("Attribute index %i is out of  max bound %i", index, mNumAttributes);
        assert(false);
        return;
    }
    
    const Attribute& attribute = mAttributes[index];
    shaderLocation = attribute.shaderLocation;
    size = attribute.size;
    offset = attribute.offset;
    dataType = attribute.dataType;
    normalized = attribute.normalized; 
    enabled = attribute.enabled;
}
}