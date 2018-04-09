//
//  RAVertexHardwareBuffer.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-12.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAVertexHardwareBuffer.h"

namespace tyro
{
    
VertexHardwareBuffer::VertexHardwareBuffer()
:
HardwareBuffer(),
mNumOfVerticies(0),
mVertexSize(0)
{
}

VertexHardwareBuffer::VertexHardwareBuffer(size_t vertexSize, size_t numOfVerticies, HardwareBuffer::Usage usage)
:
HardwareBuffer(vertexSize * numOfVerticies, usage),
mVertexSize(vertexSize),
mNumOfVerticies(numOfVerticies)
{}

VertexHardwareBuffer::~VertexHardwareBuffer()
{}
}