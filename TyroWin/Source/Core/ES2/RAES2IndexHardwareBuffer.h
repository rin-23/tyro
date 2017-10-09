//
//  RAES2IndexHardwareBuffer.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include <memory>
#include <GL/glew.h>
#include "RAIndexHardwareBuffer.h"
#include <cereal/types/base_class.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>

namespace RAEngine
{
    class ES2IndexHardwareBuffer : public IndexHardwareBuffer
    {
    public:
        ES2IndexHardwareBuffer();
        
        //32 bite indicies, static
        ES2IndexHardwareBuffer(GLsizei numIndexes, const GLvoid* dataPtr);
        
        //32 bit indicies
        ES2IndexHardwareBuffer(GLsizei numIndexes, const GLvoid* dataPtr, HardwareBuffer::Usage usage);

        ES2IndexHardwareBuffer(IndexType indexType, GLsizei numIndexes, const GLvoid* dataPtr, HardwareBuffer::Usage usage);

        virtual ~ES2IndexHardwareBuffer();
        
        GLuint GetName() const;
        
        void Bind() const;
        
        void Unbind() const;
        
        virtual void* Map(LockOptions options) override;
        
        virtual void* MapRead() const override;
        
        virtual void* MapWrite() override;
        
        virtual void Unmap() const override;
        
        virtual void WriteData(size_t offset, size_t length, const void* pSource) override;
        
        //Serialization
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(mSizeInBytes, mUsage, mNumIndexes, mIndexSize, mIndexType);
            char* data = (char*)MapRead();
            archive(cereal::binary_data(data, GetSizeInBytes()));
            Unmap();
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(mSizeInBytes, mUsage, mNumIndexes, mIndexSize, mIndexType);
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
    
    using ES2IndexHardwareBufferSPtr = std::shared_ptr<ES2IndexHardwareBuffer>;
}

