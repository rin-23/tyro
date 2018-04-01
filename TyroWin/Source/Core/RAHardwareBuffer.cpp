//
//  RAHardwareBuffer.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-12.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAHardwareBuffer.h"

namespace tyro
{
//zero size and static usage
HardwareBuffer::HardwareBuffer()
:
mUsage(BU_STATIC),
mSizeInBytes(0)
{
}

HardwareBuffer::HardwareBuffer(size_t sizeInBytes, Usage usage)
:
mUsage(usage),
mSizeInBytes(sizeInBytes)
{
}

HardwareBuffer::~HardwareBuffer()
{}

void HardwareBuffer::CopyData(HardwareBuffer& srcBuffer, size_t srcOffset, size_t dstOffset, size_t length)
{
    const void *srcData = srcBuffer.Map(BL_READ);
    this->WriteData(dstOffset, length, srcData);
    srcBuffer.Unmap();
}

}