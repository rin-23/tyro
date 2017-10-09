//
//  RAVertexBufferAccessor.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAEnginePrerequisites.h"
#include "RAHardwareBuffer.h"

namespace RAEngine
{
    class ES2Renderable;
    
    class VertexBufferAccessor
    {
    public:
        VertexBufferAccessor(ES2VertexFormat* format, VertexHardwareBuffer* buffer);

        VertexBufferAccessor(ES2Renderable* renderable);

        ~VertexBufferAccessor();
        
        int GetNumElements() const;
        
        // Called by the constructors.
        void Map(HardwareBuffer::LockOptions options);
        void MapRead();
        void MapWrite();
        void Unmap();
        
        // Generic accessors to the vertex buffer data.  You must know the type T
        // for the particular attributes.  The Get*Channels() functions are valid
        // only for FLOAT{1,2,3,4} data and return the number of floats for the
        // attributes.
        template <typename T>
        inline T& Position (int i);
        inline bool HasPosition () const;
        inline float* PositionTuple (int i);
        
        template <typename T>
        inline T& Normal (int i);
        inline bool HasNormal () const;
        inline float* NormalTuple (int i);
        
        inline float PackedNormal (int i);
        inline bool HasPackedNormal () const;
        
        template <typename T>
        inline T& Tangent (int i);
        inline bool HasTangent () const;
        inline float* TangentTuple (int i);
        
        template <typename T>
        inline T& Binormal (int i);
        inline bool HasBinormal () const;
        inline int GetBinormalChannels () const;
        inline float* BinormalTuple (int i);
        
        template <typename T>
        inline T& TCoord (int i);
        inline bool HasTCoord () const;
        inline float* TCoordTuple (int i);
        
        template <typename T>
        inline T& Color (int i);
        inline bool HasColor () const;
        inline float* ColorTuple (int i);
        
        template <typename T>
        inline T& JointIndex1 (int i);
        inline bool HasJointIndex1 () const;
        
        template <typename T>
        inline T& JointIndex2 (int i);
        inline bool HasJointIndex2 () const;
        
        template <typename T>
        inline T& BlendWeight1 (int i);
        inline bool HasBlendWeight1 () const;

        template <typename T>
        inline T& BlendWeight2 (int i);
        inline bool HasBlendWeight2 () const;

        
    private:

        ES2VertexFormat* mVFormat;
        VertexHardwareBuffer* mVBuffer;
        int mStride;
        char* mData;
        int mNumElements;
        
        char* mPosition;
        char* mNormal;
        char* mPackedNormal;
        char* mTangent;
        char* mBinormal;
        char* mTCoord;
        char* mColor;
        char* mJointIndex1;
        char* mJointIndex2;
        char* mBlendWeight1;
        char* mBlendWeight2;
    };
    
    //----------------------------------------------------------------------------
    template <typename T>
    inline T& VertexBufferAccessor::Position (int i)
    {
        return *(T*)(mPosition + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasPosition () const
    {
        return mPosition != nullptr;
    }
    //----------------------------------------------------------------------------
    inline float* VertexBufferAccessor::PositionTuple (int i)
    {
        return (float*)(mPosition + i*mStride);
    }
    //----------------------------------------------------------------------------
    template <typename T>
    inline T& VertexBufferAccessor::Normal (int i)
    {
        return *(T*)(mNormal + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasNormal () const
    {
        return mNormal != nullptr;
    }
    //----------------------------------------------------------------------------
    inline float* VertexBufferAccessor::NormalTuple (int i)
    {
        return (float*)(mNormal + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline float VertexBufferAccessor::PackedNormal (int i)
    {
        return *(float*)(mPackedNormal + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasPackedNormal () const
    {
        return mPackedNormal != nullptr;
    }
    //----------------------------------------------------------------------------
    template <typename T>
    inline T& VertexBufferAccessor::Tangent (int i)
    {
        return *(T*)(mTangent + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasTangent () const
    {
        return mTangent != nullptr;
    }
    //----------------------------------------------------------------------------
    inline float* VertexBufferAccessor::TangentTuple (int i)
    {
        return (float*)(mTangent + i*mStride);
    }
    //----------------------------------------------------------------------------
    template <typename T>
    inline T& VertexBufferAccessor::Binormal (int i)
    {
        return *(T*)(mBinormal + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasBinormal () const
    {
        return mBinormal != nullptr;
    }
    //----------------------------------------------------------------------------
    inline float* VertexBufferAccessor::BinormalTuple (int i)
    {
        return (float*)(mBinormal + i*mStride);
    }
    //----------------------------------------------------------------------------
    template <typename T>
    inline T& VertexBufferAccessor::TCoord (int i)
    {
        return *(T*)(mTCoord + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasTCoord () const
    {
        return mTCoord != nullptr;
    }
    //----------------------------------------------------------------------------
    inline float* VertexBufferAccessor::TCoordTuple (int i)
    {
        return (float*)(mTCoord + i*mStride);
    }
    //----------------------------------------------------------------------------
    template <typename T>
    inline T& VertexBufferAccessor::Color (int i)
    {
        return *(T*)(mColor + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasColor () const
    {
        return mColor != nullptr;
    }
    //----------------------------------------------------------------------------
    inline float* VertexBufferAccessor::ColorTuple (int i)
    {
        return (float*)(mColor + i*mStride);
    }
    //----------------------------------------------------------------------------
    template <typename T>
    inline T& VertexBufferAccessor::JointIndex1 (int i)
    {
        return *(T*)(mJointIndex1 + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasJointIndex1 () const
    {
        return mJointIndex1 != nullptr;
    }
    //----------------------------------------------------------------------------
    template <typename T>
    inline T& VertexBufferAccessor::JointIndex2 (int i)
    {
        return *(T*)(mJointIndex2 + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasJointIndex2 () const
    {
        return mJointIndex2 != nullptr;
    }
    //----------------------------------------------------------------------------
    template <typename T>
    inline T& VertexBufferAccessor::BlendWeight1 (int i)
    {
        return *(T*)(mBlendWeight1 + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasBlendWeight1 () const
    {
        return mBlendWeight1 != nullptr;
    }
    //----------------------------------------------------------------------------
    template <typename T>
    inline T& VertexBufferAccessor::BlendWeight2 (int i)
    {
        return *(T*)(mBlendWeight2 + i*mStride);
    }
    //----------------------------------------------------------------------------
    inline bool VertexBufferAccessor::HasBlendWeight2 () const
    {
        return mBlendWeight2 != nullptr;
    }
    //----------------------------------------------------------------------------
}

