//
//  RAVisual.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RASpatial.h"
#include "RAES2VisualEffect.h"
#include "RAES2VertexHardwareBuffer.h"
#include "RAES2IndexHardwareBuffer.h"
#include "RAES2VertexArray.h"
#include "RAEnginePrerequisites.h"

namespace RAEngine
{
    class ES2Renderable : public Spatial
    {
    public:
        enum ES2PrimitiveType
        {
            PT_NONE,  // default constructor
            PT_POINTS,
            PT_LINES,
            PT_LINE_STRIP,
            PT_TRIANGLES,
            //            PT_TRIANGLE_STRIP,
            //            PT_TRIANGLE_FAN,
            PT_MAX_QUANTITY
        };

        ES2Renderable() {}
     
        virtual ~ES2Renderable() {}

//        static ES2RenderableSPtr Create(ES2PrimitiveType type);
        
        inline ES2PrimitiveType GetPrimitiveType() const;
        
		inline void SetVertexArray(ES2VertexArraySPtr vertexArray);
		inline const ES2VertexArraySPtr GetVertexArray() const;
        inline ES2VertexArraySPtr GetVertexArray();
        
        inline void SetVisualEffect(ES2VisualEffectSPtr visualEffect);
        inline const ES2VisualEffectSPtr GetVisualEffect() const;
        inline ES2VisualEffectSPtr GetVisualEffect();
        
        inline void SetVertexBuffer(ES2VertexHardwareBufferSPtr vertexBuffer);
        inline const ES2VertexHardwareBufferSPtr GetVertexBuffer() const;
        inline ES2VertexHardwareBufferSPtr GetVertexBuffer();
        
        inline void SetIndexBuffer(ES2IndexHardwareBufferSPtr indexBuffer);
        inline const ES2IndexHardwareBufferSPtr GetIndexBuffer() const;
        inline ES2IndexHardwareBufferSPtr GetIndexBuffer();
        
        virtual void UpdateUniformsWithCamera(const Camera* camera) = 0;

        virtual void GetVisibleSet(RAEngine::VisibleSet* visibleSet) override;
        
        virtual void GetHitProxies(RAEngine::VisibleSet* visibleSet) override;
        
    public: //Serialization
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<Spatial>(this));
            archive(mVertexBuffer);
            archive(mIndexBuffer);
            archive(LocalBoundBox);
            archive(mPrimitiveType);
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<Spatial>(this));
            archive(mVertexBuffer);
            archive(mIndexBuffer);
            archive(LocalBoundBox);
            archive(mPrimitiveType);
        }
    protected:
        
        virtual void UpdateBounds () override;
        
        void Init(ES2PrimitiveType type);
        
    public:
    
        AxisAlignedBBox LocalBoundBox;
    
    private:
        ES2PrimitiveType mPrimitiveType;
		ES2VertexArraySPtr mVertexArray;
		ES2VisualEffectSPtr mVisualEffect;
        ES2VertexHardwareBufferSPtr mVertexBuffer;
        ES2IndexHardwareBufferSPtr mIndexBuffer;
    };
    
    /****INLINE FUNCTIONS****/
    
//    inline const ES2VertexArray* ES2Renderable::GetVertexArray() const
//    {
//        return mVertexArray;
//    }
//    
//    inline ES2VertexArray* ES2Renderable::GetVertexArray()
//    {
//        return mVertexArray;
//    }
    
    inline void ES2Renderable::SetVisualEffect(ES2VisualEffectSPtr visualEffect)
    {
        mVisualEffect = visualEffect;
    }
    
    inline const ES2VisualEffectSPtr ES2Renderable::GetVisualEffect() const
    {
        return mVisualEffect;
    }
    
    inline ES2VisualEffectSPtr ES2Renderable::GetVisualEffect()
    {
        return mVisualEffect;
    }

    inline ES2Renderable::ES2PrimitiveType ES2Renderable::GetPrimitiveType() const
    {
        return mPrimitiveType;
    }
    
    inline void ES2Renderable::SetVertexBuffer(ES2VertexHardwareBufferSPtr vertexBuffer)
    {
        mVertexBuffer = vertexBuffer;
    }
    
    inline const ES2VertexHardwareBufferSPtr ES2Renderable::GetVertexBuffer() const
    {
        return mVertexBuffer;
    }
    
    inline ES2VertexHardwareBufferSPtr ES2Renderable::GetVertexBuffer()
    {
        return mVertexBuffer;
    }
    
    inline void ES2Renderable::SetIndexBuffer(ES2IndexHardwareBufferSPtr indexBuffer)
    {
        mIndexBuffer = indexBuffer;
    }
    
    inline const ES2IndexHardwareBufferSPtr ES2Renderable::GetIndexBuffer() const
    {
        return mIndexBuffer;
    }
    
    inline ES2IndexHardwareBufferSPtr ES2Renderable::GetIndexBuffer()
    {
        return mIndexBuffer;
    }
	
	inline void ES2Renderable::SetVertexArray(ES2VertexArraySPtr vertexArray)
	{
		mVertexArray = vertexArray;
	}

	inline const ES2VertexArraySPtr ES2Renderable::GetVertexArray() const
	{
		return mVertexArray;
	}

	inline ES2VertexArraySPtr ES2Renderable::GetVertexArray()
	{
		return mVertexArray;
	}
}

CEREAL_REGISTER_TYPE(RAEngine::ES2Renderable);

