//
//  RAES2VertexHardwareBuffer.h
//  PAM2
//
//  Created by Rinat Abdrashitov on 2014-06-11.
//  Copyright (c) 2014 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include <iostream>
#include <memory>
//#include <OpenGLES/gltypes.h>
#include <GL/glew.h>
#include "RAVertexHardwareBuffer.h"
#include <cereal/types/base_class.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>

namespace tyro
{
    class ES2VertexHardwareBuffer : public VertexHardwareBuffer
    {
    public:
        
        ES2VertexHardwareBuffer();
        
        //static usage
        ES2VertexHardwareBuffer(GLsizei vertexSize, GLsizei numOfVerticies, const GLvoid* dataPtr);

        ES2VertexHardwareBuffer(GLsizei vertexSize, GLsizei numOfVerticies, const GLvoid* dataPtr, HardwareBuffer::Usage usage);

        virtual ~ES2VertexHardwareBuffer();
        
        GLuint GetName() const;
        
        void EnableAttribute(GLuint index) const;
        
        void DisableAttribute(GLuint index) const;
        
        void Bind() const;
        
        void Unbind() const;

        virtual void* Map(LockOptions options) override;
        
        virtual void* MapRead() const override;
        
        virtual void* MapWrite() override;

        virtual void Unmap() const override;
        
        virtual void WriteData(size_t offset, size_t length, const void* pSource) override;
        
        void PrepareToDraw(GLuint index, GLint count, GLsizeiptr offset, GLenum type, GLboolean normalized) const;
        
        //Serialization
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(mSizeInBytes, mUsage, mNumOfVerticies, mVertexSize);
            char* data = (char*)MapRead();
            archive(cereal::binary_data(data, GetSizeInBytes()));
            Unmap();
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(mSizeInBytes, mUsage, mNumOfVerticies, mVertexSize);
            _SerializeInit();
            char* data = (char*)MapWrite();
            archive(cereal::binary_data(data, GetSizeInBytes()));
            Unmap();
        }
        
    private:
        GLuint mName;
        
        GLenum GetGLUsage(HardwareBuffer::Usage usage) const;
        
        void _SerializeInit();
    };
    
    using ES2VertexHardwareBufferSPtr = std::shared_ptr<ES2VertexHardwareBuffer>;
}
