//
//  RAES2Renderer.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-08.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once


#include "RAEnginePrerequisites.h"



namespace tyro
{  
//	class ES2Context;
    
	class ES2Renderer
    {
    public:
		ES2Renderer();
		ES2Renderer(ES2Context* glContext);

        ~ES2Renderer();
        
        void RenderVisibleSet(const VisibleSet* visibleSet, const Camera* camera);
        
        TextureBuffer* RenderVisibleSetToTexture(const VisibleSet* visualSet, const Camera* camera);

		void SetClearColor(float r, float g, float b, float a);

		void ClearScreen();
    
	private:
 
        ES2Context* mContext;

		float mClearColor[4];
		int mViewWidth;
		int mViewHeight;
	
		void RenderPrimitive(const ES2Renderable* renderable) const;
		
		void SetShader(const ES2ShaderProgram* shaderProgram) const;

		void SetTexture(const ES2Texture2DSPtr texture) const;

		void SetBufferTextures(const ES2VisualEffect* ve) const;

		void SetAlphaState(const ES2VisualEffectSPtr ve) const;

		void SetCullState(const ES2VisualEffectSPtr ve) const;

		void SetDepthState(const ES2VisualEffectSPtr ve) const;

		void SetPolygonOffset(const ES2VisualEffectSPtr ve) const;

		void UpdateUniforms(const ES2ShaderUniforms* uniforms) const;

		void PrepareVertexBuffers(const ES2VertexFormat* vertexFormat, const ES2VertexHardwareBufferSPtr vertexBufffer) const;

		void DrawPrimitive(const ES2Renderable* renderable) const;

		void DisableAttributes(const ES2VertexFormat* vertexFormat, const ES2VertexHardwareBufferSPtr vertexBufffer) const;
	
	};
}
