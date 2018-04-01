//
//  RAIndexHardwareBuffer.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAIndexHardwareBuffer.h"

namespace tyro
{

IndexHardwareBuffer::IndexHardwareBuffer()
:
HardwareBuffer()
{
}

IndexHardwareBuffer::IndexHardwareBuffer(IndexType idxType, size_t numIndexes, HardwareBuffer::Usage usage)
:
HardwareBuffer(),
mIndexType(idxType),
mNumIndexes(numIndexes)
{
    switch (mIndexType)
    {
//        case IT_16BIT:
//            mIndexSize = sizeof(unsigned short);
//            break;
        case IT_32BIT:
            mIndexSize = sizeof(unsigned int);
            break;
    }
    
    //HardwareBuffer members
    mUsage = usage;
    mSizeInBytes = mNumIndexes * mIndexSize;
}

IndexHardwareBuffer::~IndexHardwareBuffer()
{}
}