//
//  RAVisual.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2Renderable.h"
#include "Wm5TransformBox3.h"
#include "RAVisibleSet.h"

namespace RAEngine
{
    void ES2Renderable::Init(ES2PrimitiveType type)
    {
        Spatial::Init();
        mPrimitiveType = type;
    }
    
//    ES2RenderableSPtr ES2Renderable::Create(ES2PrimitiveType type)
//    {
//        ES2RenderableSPtr sptr = std::make_shared<ES2Renderable>();
//        sptr->Init(type);
//        return sptr;
//    }

//
//    void ES2Renderable::GenerateVertexArray()
//    {
//        if (mVertexBuffer && mVisualEffect)
//        {
//            if (mVertexArray == NULL)
//            {
//                mVertexArray = new ES2VertexArray();
//            }
//            mVertexArray->Bind();
//
//            mVertexBuffer->Bind();
//            for (int i = 0; i < mVisualEffect->GetVertexFormat()->GetNumOfAttributes(); ++i)
//            {
//                int shaderLocation;
//                GLint size;
//                GLsizeiptr offset;
//                GLenum dataType;
//                GLboolean normalized;
//                GLboolean enabled;
//                mVisualEffect->GetVertexFormat()->GetGLAttribute(i, shaderLocation, size, offset, dataType, normalized, enabled);
//                
//                if (enabled)
//                {
//                    mVertexBuffer->PrepareToDraw(shaderLocation, size, offset, dataType, normalized);
//                    mVertexBuffer->EnableAttribute(shaderLocation);
//                }
//                else
//                {
//                    mVertexBuffer->DisableAttribute(shaderLocation);
//                }
//            }
//            
//            mVertexArray->Unbind();
//        }
//        else
//        {
//            assert(false);
//        }
//    }

    void ES2Renderable::UpdateBounds()
    {
        WorldBoundBox = AxisAlignedBBox::TransformAffine(WorldTransform, LocalBoundBox);
    }
    
    void ES2Renderable::GetVisibleSet (RAEngine::VisibleSet* visibleSet)
    {
        if (this->Visible)
        {
            Spatial::GetVisibleSet(visibleSet);
            visibleSet->Insert(this);
        }
    }
    
    void ES2Renderable::GetHitProxies(RAEngine::VisibleSet* visibleSet)
    {}
}
