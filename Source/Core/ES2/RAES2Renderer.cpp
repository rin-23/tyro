//
//  RAES2Renderer.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-08.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "RAES2Renderer.h"
#include "RALogManager.h"
#include "RACamera.h"
//#include "RAiOSBundle.h"
//#include "RAFileManager.h"
//#include "Wm5Time.h"
//#include <iterator>
//#include <list>
//#include "RAOverlayManager.h"
//#include "RAHitProxy.h"
//#include "OssaScene.h"
#include "RAVisibleSet.h"
//#include "RATextureBuffer.h"
#include "RAES2Context.h"
#include "RAES2Renderable.h"


namespace tyro 
{
	ES2Renderer::ES2Renderer()
	{}

	ES2Renderer::ES2Renderer(ES2Context* glContext)
	{		
		mContext = glContext;
		mClearColor = Wm5::Vector4f(200 / 255.0f, 200 / 255.0f, 200 / 255.0f, 1.0f);
		glContext->getFramebufferSize(&mViewWidth, &mViewHeight);
		glViewport(0, 0, mViewWidth, mViewHeight);
		glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);

		glLineWidth(3.0);
		GL_CHECK_ERROR;

		RA_LOG_VERBOSE("Finished creating framebuffers");
	}

	ES2Renderer::~ES2Renderer()
	{
		// DeleteFramebuffer();
	}

	void ES2Renderer::ClearScreen() 
	{
		mContext->setCurrent();
		GL_CHECK_ERROR;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GL_CHECK_ERROR;
	}

	void ES2Renderer::RenderVisibleSet(const VisibleSet* visualSet, const Camera* camera)
	{
		assert(visualSet);
		assert(camera);

		//    if (visualSet->GetNumVisible() == 0)
		//    {
		//        RA_LOG_WARN("Visual set is empty");
		//        return;
		//    }

		//glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
		mContext->setCurrent();
		GL_CHECK_ERROR;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GL_CHECK_ERROR;
		//        glDepthMask(GL_TRUE);
		for (int i = 0; i < visualSet->GetNumVisible(); ++i)
		{
			ES2Renderable* renderable = (ES2Renderable*)visualSet->GetVisible(i);
			assert(renderable);
			//        RA_LOG_INFO("Object name %s", renderable->GetName().data());
			if (!renderable->GetVisualEffect()->GetAlphaState()->Enabled)
			{
				renderable->UpdateUniformsWithCamera(camera);
				RenderPrimitive(renderable);
			}
			else
			{
				/*
				AlphaSceneNode* alphaNode = new AlphaSceneNode();
				alphaNode->renderable = renderable;
				Wm5::Vector3f cameraPos = camera->GetViewMatrix() * Wm5::APoint(renderable->WorldBoundBox.GetCenter());
				alphaNode->mScreenZ = cameraPos.Z();
				mAlphaSceneNodes.push_back(alphaNode);
				*/
			}
		}

		/*
		//Alpha Pass
		mAlphaSceneNodes.sort();
		//        glDepthMask(GL_FALSE);
		while (!mAlphaSceneNodes.empty())
		{
			AlphaSceneNodes::reverse_iterator i = mAlphaSceneNodes.rbegin();
			ES2Renderable* renderable = (*i)->renderable;
			renderable->UpdateUniformsWithCamera(camera);
			RenderPrimitive(renderable);
			mAlphaSceneNodes.pop_back();
		}

		//Draw UI elements
		VisibleSet uiSet;
		OverlayManager::GetSingleton()->GetVisibleSet(&uiSet);
		for (int i = 0; i < uiSet.GetNumVisible(); ++i)
		{
			ES2Renderable* uirenderable = (ES2Renderable*)uiSet.GetVisible(i);
			uirenderable->UpdateUniformsWithCamera(camera);
			RenderPrimitive(uirenderable);
		}
		*/
		//#warning I dont know why, but without this line blending is not working properly
			/***NEED TO FIX THIS****/
			//TODO
			//glDepthMask(GL_TRUE);
		/*******/

		//glBindRenderbuffer(GL_RENDERBUFFER, mColorRenderbuffer);
		//GL_CHECK_ERROR;

		//[mContext presentRenderbuffer : GL_RENDERBUFFER];
		//mContext->swapBuffers();
	}

	void ES2Renderer::RenderPrimitive(const ES2Renderable* renderable) const
	{
		assert(renderable);

		ES2VisualEffectSPtr effect = renderable->GetVisualEffect();

		SetShader(effect->GetShader());
		UpdateUniforms(effect->GetUniforms());
		if (renderable->GetVertexArray()) 
		{
			renderable->GetVertexArray()->Bind();
		}
		else 
		{
			PrepareVertexBuffers(effect->GetVertexFormat(), renderable->GetVertexBuffer());
		}
		
		SetTexture(effect->GetTexture2D());
		SetAlphaState(effect->GetAlphaState());
		SetCullState(effect->GetCullState());
		SetDepthState(effect->GetDepthState());
		DrawPrimitive(renderable);
		//DisableAttributes(effect->GetVertexFormat(), renderable->GetVertexBuffer());
	}

	void ES2Renderer::SetClearColor(const Wm5::Vector4f& clearColor) 
	{
		mClearColor = clearColor;
		glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
	}

	void ES2Renderer::SetShader(const ES2ShaderProgram* shaderProgram) const
	{
		shaderProgram->UseProgram();
	}

	void ES2Renderer::SetAlphaState(const ES2AlphaState* alphaState) const
	{
		if (alphaState->Enabled)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			GL_CHECK_ERROR;
		}
		else
		{
			glDisable(GL_BLEND);
			GL_CHECK_ERROR;
		}
	}

	void ES2Renderer::SetCullState(const ES2CullState* cullState) const
	{
		if (cullState->Enabled)
		{
			glEnable(GL_CULL_FACE);
			GL_CHECK_ERROR;
		}
		else
		{
			glDisable(GL_CULL_FACE);
			GL_CHECK_ERROR;
		}
	}

	void ES2Renderer::SetDepthState(const ES2DepthState* depthState) const
	{
		if (depthState->Enabled)
		{
			glEnable(GL_DEPTH_TEST);
			GL_CHECK_ERROR;
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
			GL_CHECK_ERROR;
		}

		if (depthState->DepthMaskEnabled)
		{
			glDepthMask(GL_TRUE);
			GL_CHECK_ERROR;
		}
		else
		{
			glDepthMask(GL_FALSE);
			GL_CHECK_ERROR;
		}
	}

	void ES2Renderer::SetTexture(const ES2Texture2DSPtr texture) const
	{
		if (texture)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture->GetTextureID());
			GL_CHECK_ERROR;
		}
	}

	void ES2Renderer::UpdateUniforms(const ES2ShaderUniforms* uniforms) const
	{
		for (int i = 0; i < uniforms->GetNumOfUniforms(); ++i)
		{
			const ES2ShaderUniforms::Uniform* uni = uniforms->GetUniform(i);

			switch (uni->type)
			{
			case ES2ShaderUniforms::UniformMatrix4fv:
			{
				glUniformMatrix4fv(uni->shaderLocation, uni->count, 0, uni->floatValue);
				GL_CHECK_ERROR;
				break;
			}
			case ES2ShaderUniforms::UniformMatrix3fv:
			{
				glUniformMatrix3fv(uni->shaderLocation, uni->count, 0, uni->floatValue);
				GL_CHECK_ERROR;
				break;
			}
			case ES2ShaderUniforms::UniformMatrix2fv:
			{
				glUniformMatrix2fv(uni->shaderLocation, uni->count, 0, uni->floatValue);
				GL_CHECK_ERROR;
				break;
			}
			case ES2ShaderUniforms::Uniform4fv:
			{
				glUniform4fv(uni->shaderLocation, uni->count, uni->floatValue);
				GL_CHECK_ERROR;
				break;
			}
			case ES2ShaderUniforms::Uniform3fv:
			{
				glUniform3fv(uni->shaderLocation, uni->count, uni->floatValue);
				GL_CHECK_ERROR;
				break;
			}
			case ES2ShaderUniforms::Uniform2fv:
			{
				glUniform2fv(uni->shaderLocation, uni->count, uni->floatValue);
				GL_CHECK_ERROR;
				break;
			}
			case ES2ShaderUniforms::Uniform1fv:
			{
				glUniform1fv(uni->shaderLocation, uni->count, uni->floatValue);
				GL_CHECK_ERROR;
				break;
			}
			case ES2ShaderUniforms::Uniform1iv:
			{
				glUniform1iv(uni->shaderLocation, uni->count, uni->intValue);
				GL_CHECK_ERROR;
				break;
			}
			default:
				break;
			}
		}
	}

	void ES2Renderer::PrepareVertexBuffers(const ES2VertexFormat* vertexFormat, const ES2VertexHardwareBufferSPtr vertexBufffer) const
	{
		vertexBufffer->Bind();
		for (int i = 0; i < vertexFormat->GetNumOfAttributes(); ++i)
		{
			int shaderLocation;
			GLint size;
			GLsizeiptr offset;
			GLenum dataType;
			GLboolean normalized;
			GLboolean enabled;
			vertexFormat->GetGLAttribute(i, shaderLocation, size, offset, dataType, normalized, enabled);

			if (enabled)
			{	
				vertexBufffer->EnableAttribute(shaderLocation);
				vertexBufffer->PrepareToDraw(shaderLocation, size, offset, dataType, normalized);
			}
			else
			{
				vertexBufffer->DisableAttribute(shaderLocation);
			}
		}
	}

	void ES2Renderer::DrawPrimitive(const ES2Renderable* renderable) const
	{
		ES2Renderable::ES2PrimitiveType primitiveType = renderable->GetPrimitiveType();
		GLenum glPrimType;

		switch (primitiveType)
		{
		case ES2Renderable::PT_NONE:
		{
			assert(false);
			return;
		}
		case ES2Renderable::PT_TRIANGLES:
		{
			glPrimType = GL_TRIANGLES;
			break;
		}
		case ES2Renderable::PT_LINES:
		{
			glPrimType = GL_LINES;
			break;
		}
		case ES2Renderable::PT_LINE_STRIP:
		{
			glPrimType = GL_LINE_STRIP;
			break;
		}
		case ES2Renderable::PT_POINTS:
		{
			glPrimType = GL_POINTS;
			break;
		}
		default:
			break;
		}

		if (renderable->GetIndexBuffer())
		{
			ES2IndexHardwareBufferSPtr indexBuffer = renderable->GetIndexBuffer();
			indexBuffer->Bind();

			//        if (indexBuffer->GetIndexType() == IndexHardwareBuffer::IT_16BIT)
			//        {
			//            glDrawElements(glPrimType, indexBuffer->GetNumIndexes(), GL_UNSIGNED_SHORT, 0);
			//        }
			//        else
			if (indexBuffer->GetIndexType() == IndexHardwareBuffer::IT_32BIT)
			{
				glDrawElements(glPrimType, indexBuffer->GetNumIndexes(), GL_UNSIGNED_INT, 0);
			}
			GL_CHECK_ERROR;
		}
		else
		{
			//vertex buffer should be bound by this time
			ES2VertexHardwareBufferSPtr vertexBuffer = renderable->GetVertexBuffer();
			glDrawArrays(glPrimType, 0, vertexBuffer->GetNumOfVerticies());
			GL_CHECK_ERROR;
		}
	}

	void ES2Renderer::DisableAttributes(const ES2VertexFormat* vertexFormat, const ES2VertexHardwareBufferSPtr vertexBufffer) const
	{
		for (int i = 0; i < vertexFormat->GetNumOfAttributes(); ++i)
		{
			vertexBufffer->DisableAttribute(vertexFormat->GetShaderLocation(i));
		}
	}

}