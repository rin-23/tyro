//
//  RAIndexHardwareBuffer.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAHardwareBuffer.h"

namespace tyro
{
    class IndexHardwareBuffer : public HardwareBuffer
    {
    public:
        enum IndexType
        {
//            IT_16BIT, // no support yet
            IT_32BIT
        };
        
        IndexHardwareBuffer();
        
        IndexHardwareBuffer(IndexType idxType, size_t numIndexes, HardwareBuffer::Usage usage);
        
        virtual ~IndexHardwareBuffer();
        
        inline size_t GetNumIndexes() const;
        
        inline size_t GetIndexSize() const;
        
        inline IndexType GetIndexType() const;
        
    protected:
        size_t mNumIndexes;
        size_t mIndexSize;
        IndexType mIndexType;
    };
    
    inline size_t IndexHardwareBuffer::GetNumIndexes() const
    {
        return mNumIndexes;
    }
    
    inline size_t IndexHardwareBuffer::GetIndexSize() const
    {
        return mIndexSize;
    }
    
    inline IndexHardwareBuffer::IndexType IndexHardwareBuffer::GetIndexType() const
    {
        return mIndexType;
    }
}
