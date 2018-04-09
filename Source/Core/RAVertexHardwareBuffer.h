//
//  RAVertexHardwareBuffer.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-12.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAHardwareBuffer.h"

namespace tyro
{
    class VertexHardwareBuffer : public HardwareBuffer
    {
    public:
        VertexHardwareBuffer();
        
        VertexHardwareBuffer(size_t vertexSize, size_t numOfVerticies, HardwareBuffer::Usage usage);
        
        virtual ~VertexHardwareBuffer();
        
        inline size_t GetNumOfVerticies() const;
        
        inline size_t GetVertexSize() const;

    protected:
        size_t mNumOfVerticies;
        
        size_t mVertexSize;
    };
    
    inline size_t VertexHardwareBuffer::GetNumOfVerticies() const
    {
        return mNumOfVerticies;
    }
    
    inline size_t VertexHardwareBuffer::GetVertexSize() const
    {
        return mVertexSize;
    }
}
