//
//  RAES2VertexArray.cpp
//  PAM2
//
//  Created by Rinat Abdrashitov on 2014-06-23.
//  Copyright (c) 2014 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2VertexArray.h"
//#include <OpenGLES/ES2/glext.h>
#include <GL/glew.h>
#include "RALogManager.h"
#include "RAES2VertexHardwareBuffer.h"
#include "RAES2VisualEffect.h"

namespace tyro
{
ES2VertexArray::ES2VertexArray(const ES2VisualEffectSPtr visualEffect, const ES2VertexHardwareBufferSPtr vertexBuffer)
{
    glGenVertexArrays(1, &mName);
	GL_CHECK_ERROR;
	glBindVertexArray(mName);
	GL_CHECK_ERROR;
	
	vertexBuffer->Bind();
	for (int i = 0; i < visualEffect->GetVertexFormat()->GetNumOfAttributes(); ++i)
	{
		int shaderLocation;
		GLint size;
		GLsizeiptr offset;
		GLenum dataType;
		GLboolean normalized;
		GLboolean enabled;
		visualEffect->GetVertexFormat()->GetGLAttribute(i, shaderLocation, size, offset, dataType, normalized, enabled);

		if (enabled)
		{
			vertexBuffer->PrepareToDraw(shaderLocation, size, offset, dataType, normalized);
			vertexBuffer->EnableAttribute(shaderLocation);
		}
		else
		{
			RA_LOG_ERROR_ASSERT("not implemented");
			vertexBuffer->DisableAttribute(shaderLocation);
		}
	}

	glBindVertexArray(0);
	GL_CHECK_ERROR;
}

ES2VertexArray::~ES2VertexArray() 
{
	// Delete buffer from current context
	if (0 != mName)
	{
		glDeleteVertexArrays(1, &mName); // Step 7
		GL_CHECK_ERROR;
	}
}


void ES2VertexArray::Bind() const
{
    glBindVertexArray(mName);
    GL_CHECK_ERROR;
}

void ES2VertexArray::Unbind()
{
    glBindVertexArray(0);
	GL_CHECK_ERROR;
}

GLuint ES2VertexArray::GetName() const
{
    return mName;
}

}