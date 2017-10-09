//
//  RAVertexBufferAccessor.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAVertexBufferAccessor.h"
#include "RAES2Renderable.h"
#include "RAES2VertexFormat.h"
#include "RAVertexHardwareBuffer.h"

namespace RAEngine
{

VertexBufferAccessor::VertexBufferAccessor(ES2VertexFormat* format, VertexHardwareBuffer* buffer)
:
mVFormat(format),
mVBuffer(buffer),
mStride(0),
mData(nullptr),
mPosition(nullptr),
mNormal(nullptr),
mPackedNormal(nullptr),
mTangent(nullptr),
mBinormal(nullptr),
mTCoord(nullptr),
mColor(nullptr),
mJointIndex1(nullptr),
mJointIndex2(nullptr),
mBlendWeight1(nullptr),
mBlendWeight2(nullptr)
{}


VertexBufferAccessor::VertexBufferAccessor(ES2Renderable* renderable)
:
VertexBufferAccessor(renderable->GetVisualEffect()->GetVertexFormat(), renderable->GetVertexBuffer().get())
{}

VertexBufferAccessor::~VertexBufferAccessor()
{}

int VertexBufferAccessor::GetNumElements() const
{
    return mVBuffer->GetNumOfVerticies();
}

void VertexBufferAccessor::Map(HardwareBuffer::LockOptions options)
{
    mStride = mVBuffer->GetVertexSize();
    mData = static_cast<char*>(mVBuffer->Map(options));
    
    int index = mVFormat->GetIndex(ES2VertexFormat::AU_POSITION);
    if (index >= 0)
    {
        mPosition = mData + mVFormat->GetOffset(index);
    }
    
    index = mVFormat->GetIndex(ES2VertexFormat::AU_NORMAL);
    if (index >= 0)
    {
        mNormal = mData + mVFormat->GetOffset(index);
    }
    
    index = mVFormat->GetIndex(ES2VertexFormat::AU_PACKED_NORMAL);
    if (index >= 0)
    {
        mPackedNormal = mData + mVFormat->GetOffset(index);
    }
    
    index = mVFormat->GetIndex(ES2VertexFormat::AU_TANGENT);
    if (index >= 0)
    {
        mTangent = mData + mVFormat->GetOffset(index);
    }
    
    index = mVFormat->GetIndex(ES2VertexFormat::AU_BINORMAL);
    if (index >= 0)
    {
        mBinormal = mData + mVFormat->GetOffset(index);
    }
    
    index = mVFormat->GetIndex(ES2VertexFormat::AU_TEXCOORD);
    if (index >= 0)
    {
        mTCoord = mData + mVFormat->GetOffset(index);
    }
    
    index = mVFormat->GetIndex(ES2VertexFormat::AU_COLOR);
    if (index >= 0)
    {
        mColor = mData + mVFormat->GetOffset(index);
    }
    
    index = mVFormat->GetIndex(ES2VertexFormat::AU_JOINT_INDEX_1);
    if (index >= 0)
    {
        mJointIndex1 = mData + mVFormat->GetOffset(index);
    }
    
    index = mVFormat->GetIndex(ES2VertexFormat::AU_JOINT_INDEX_2);
    if (index >= 0)
    {
        mJointIndex2 = mData + mVFormat->GetOffset(index);
    }
    
    index = mVFormat->GetIndex(ES2VertexFormat::AU_BLENDWEIGHT_1);
    if (index >= 0)
    {
        mBlendWeight1 = mData + mVFormat->GetOffset(index);
    }
    
    index = mVFormat->GetIndex(ES2VertexFormat::AU_BLENDWEIGHT_2);
    if (index >= 0)
    {
        mBlendWeight2 = mData + mVFormat->GetOffset(index);
    }
}

void VertexBufferAccessor::MapRead()
{
    Map(HardwareBuffer::BL_READ);
}

void VertexBufferAccessor::MapWrite()
{
    Map(HardwareBuffer::BL_WRITE);
}

void VertexBufferAccessor::Unmap()
{
    mVBuffer->Unmap();
}
}