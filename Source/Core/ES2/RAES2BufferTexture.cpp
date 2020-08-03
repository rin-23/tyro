#include "RAES2BufferTexture.h"
#include "RALogManager.h"
namespace tyro 
{
        ES2BufferTexture::ES2BufferTexture() 
        :        
        mTex(0),
        mBuf(0)
        {}
       
        ES2BufferTexture::ES2BufferTexture(Format format, 
                                           int numElements,  
                                           const GLvoid* dataPtr, 
                                           HardwareBuffer::Usage usage) 
        {
            glGenBuffers(1, &mBuf);
            GL_CHECK_ERROR;
            glBindBuffer(GL_TEXTURE_BUFFER, mBuf);
            GL_CHECK_ERROR;
            int bytesize = GetGLByteSizes(format, numElements);
            glBufferData(GL_TEXTURE_BUFFER, bytesize, dataPtr, GetGLUsage(usage));
            GL_CHECK_ERROR;

            glGenTextures(1, &mTex);
            GL_CHECK_ERROR;
            glBindTexture(GL_TEXTURE_BUFFER, mTex);
            GL_CHECK_ERROR;
            glTexBuffer(GL_TEXTURE_BUFFER, GetGLFormat(format), mBuf);
            GL_CHECK_ERROR;
        }

        GLenum ES2BufferTexture::GetGLUsage(HardwareBuffer::Usage usage) const
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

        GLenum ES2BufferTexture::GetGLFormat(Format usage) const 
        {
            if (usage == TY_RGBA32F) 
                return  GL_RGBA32F;
            else if (usage == TY_RGB32F) 
                return GL_RGB32F;
            else 
            {
                RA_LOG_ERROR_ASSERT("unsupported format");
                return 0;
            }            
        }

        int ES2BufferTexture::GetGLByteSizes(Format format, int numElements) const 
        {   
            int numComponents=0;
            int typeSize;
            if (format == TY_RGBA32F) 
            {
                numComponents = 4;
                typeSize = sizeof(GLfloat);
            }
            else if (format == TY_RGB32F)  
            {
                numComponents = 3;
                typeSize = sizeof(GLfloat);
            }

            int totalsize = numComponents * typeSize * numElements;
            return totalsize;
        }
}