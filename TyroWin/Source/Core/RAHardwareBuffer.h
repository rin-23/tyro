//
//  RAHardwareBuffer.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-12.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include <stdio.h>

namespace RAEngine
{
    class HardwareBuffer
    {
    public:
        // Usage flags for vertex buffers, index buffers, and textures.
        enum Usage
        {
            BU_STATIC,
            BU_DYNAMIC
        };
        
        // Locking modes for accessing video memory for a buffer or texture.
        enum LockOptions
        {
            BL_READ_WRITE,
            BL_READ,
            BL_WRITE,
//            BL_DISCARD,
            BL_QUANTITY
        };
        
        //zero size and static usage
        HardwareBuffer();
        
        HardwareBuffer(const HardwareBuffer&) = delete;
        
        HardwareBuffer& operator=(const HardwareBuffer&) = delete;
        
        HardwareBuffer(size_t sizeInBytes, Usage usage);

        virtual ~HardwareBuffer();

        inline size_t GetSizeInBytes() const;
        
        inline Usage GetUsage() const;

        virtual void* Map(LockOptions options) = 0;

        virtual void* MapRead() const = 0;
        
        virtual void* MapWrite() = 0;
        
        virtual void Unmap() const = 0;
        
        virtual void WriteData(size_t offset, size_t length, const void* pSource) = 0;
        
        virtual void CopyData(HardwareBuffer& srcBuffer, size_t srcOffset, size_t dstOffset, size_t length);

    protected:

        size_t mSizeInBytes;
        Usage mUsage;
    };
    
    inline size_t HardwareBuffer::GetSizeInBytes() const
    {
        return mSizeInBytes;
    }
    
    inline HardwareBuffer::Usage HardwareBuffer::GetUsage() const
    {
        return mUsage;
    }
}