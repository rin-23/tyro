//
//  RAES2Texture.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-30.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2Texture2D.h"
//#include <OpenGLES/ES2/gl.h>
//#include <OpenGLES/ES2/glext.h>
#include <GL/glew.h>
#include "RALogManager.h"

namespace RAEngine
{

ES2Texture2D::ES2Texture2D(TextureFormat format, int width, int height)
:
Texture(format, TextureType::TT_2D)
{
    mWidth = width;
    mHeight = height;
}

ES2Texture2D::~ES2Texture2D()
{
    glDeleteTextures(1, &mTextureID);
    GL_CHECK_ERROR;
}

void ES2Texture2D::LoadData(const void* data)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    GL_CHECK_ERROR;
    glActiveTexture(GL_TEXTURE0);
    GL_CHECK_ERROR;
    glGenTextures(1, &mTextureID);
    GL_CHECK_ERROR;
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    GL_CHECK_ERROR;

    GLenum glFormat = GetGLFormat(mFormat);
    GLenum glType = GetGLDataType(mFormat);
    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, mWidth, mHeight, 0, glFormat, glType, data);
    GL_CHECK_ERROR;

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GL_CHECK_ERROR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL_CHECK_ERROR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERROR;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL_CHECK_ERROR;
    
    glBindTexture(GL_TEXTURE_2D, 0);
    GL_CHECK_ERROR;
}

void ES2Texture2D::LoadSubData(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, const void* data)
{
    glActiveTexture(GL_TEXTURE0);
    GL_CHECK_ERROR;
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    GL_CHECK_ERROR;

    GLenum glFormat = GetGLFormat(mFormat);
    GLenum glType = GetGLDataType(mFormat);
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, glFormat, glType, data);
    GL_CHECK_ERROR;
}

GLuint ES2Texture2D::GetTextureID() const
{
    return mTextureID;
}

GLenum ES2Texture2D::GetGLFormat(TextureFormat format)
{
    switch (format)
    {
        case Texture::TF_A8:
            return GL_ALPHA;
        case Texture::TF_L8:
            return GL_LUMINANCE;
        case Texture::TF_A8L8:
            return GL_LUMINANCE_ALPHA;
        case Texture::TF_R8G8B8:
            return GL_RGB;
        case Texture::TF_A8R8G8B8:
            return GL_RGBA;
        default:
            assert(false);
            return 0;
    }
}

GLenum ES2Texture2D::GetGLDataType(TextureFormat format)
{
    switch (format)
    {
        case Texture::TF_A8:
            return GL_UNSIGNED_BYTE;
        case Texture::TF_L8:
            return GL_UNSIGNED_BYTE;
        case Texture::TF_A8L8:
            return GL_UNSIGNED_BYTE;
        case Texture::TF_R8G8B8:
            return GL_UNSIGNED_BYTE;
        case Texture::TF_A8R8G8B8:
            return GL_UNSIGNED_BYTE;
        default:
            assert(false);
            return 0;
    }
}
}