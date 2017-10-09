//
//  RAES2STLMesh.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-17.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2STLMesh.h"
#include "GLStructures.h"
#include "RAFileManager.h"
#include "RACamera.h"
#include "RASTLLoader.h"
#include "RAES2VertexHardwareBuffer.h"

using namespace std;
using namespace Wm5;

namespace RAEngine
{
    ES2STLMesh::ES2STLMesh()
    {}
    
    ES2STLMesh::~ES2STLMesh()
    {}
    
    int ES2STLMesh::createWithSTL(const std::string& filePath)
    {
        //Load vertex buffer
        STLLoader stlLoader(filePath);
        int numTriangles = stlLoader.GetNumberOfTriangles();
        
        assert(numTriangles);
        if (numTriangles == 0)
        {
            RA_LOG_ERROR("Number of triangles is zero for stl %s", filePath.c_str());
            return 0;
        }
        /* @TODO needs refactoring
		ES2VertexHardwareBufferSPtr vertexBuffer(make_shared<ES2VertexHardwareBuffer>(sizeof(VertexNorm), 3*numTriangles, (const GLvoid*)NULL, GL_STATIC_DRAW, GL_ARRAY_BUFFER));
        void* mappedData = vertexBuffer->Map();
        STLLoader::BoundingBox bbox;
        
        if (!stlLoader.LoadSTLFile(mappedData, bbox))
        {
            RA_LOG_ERROR("Could not load stl %s", filePath.c_str());
            vertexBuffer->Unmap();
            return 0;
        }
        
        vertexBuffer->Unmap();
        SetVertexBuffer(vertexBuffer);
        LocalBoundBox = AxisAlignedBBox(bbox.min, bbox.max);

        //Setup shader
        ES2ShaderProgram* shader = new ES2ShaderProgram();
        shader->LoadProgram(GetFilePath("GourandDirectional", "vsh"), GetFilePath("GourandDirectional", "fsh"));
        
        //accessing attributes
        ES2VertexFormat* vertexFormat = new ES2VertexFormat(2);
        vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), "aPosition", 3, 0, GL_FLOAT, GL_FALSE, ES2VertexFormat::AU_POSITION);
        vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aNormal"), "aNormal", 3, 3*sizeof(GLfloat), GL_FLOAT, GL_FALSE, ES2VertexFormat::AU_NORMAL);
        
        //accesing uniforms
        ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(3);
        uniforms->SetUniform(0, shader->GetUniformLocation("uModelViewProjectionMatrix"), 1, "uModelViewProjectionMatrix", ES2ShaderUniforms::UniformMatrix4fv);
        uniforms->SetUniform(1, shader->GetUniformLocation("uNormalMatrix"), 1, "uNormalMatrix", ES2ShaderUniforms::UniformMatrix3fv);
        uniforms->SetUniform(2, shader->GetUniformLocation("uColor"), 1, "uColor", ES2ShaderUniforms::Uniform3fv);
        
        ES2VisualEffectSharedPtr effect(make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms, new ES2AlphaState(), new ES2CullState(), new ES2DepthState()));
        SetVisualEffect(effect);
		*/
        return 1;
    }
    
    void ES2STLMesh::UpdateUniformsWithCamera(const RAEngine::Camera* camera)
    {
        HMatrix modelViewMatrix = camera->GetViewMatrix() * WorldTransform.Matrix();
        HMatrix modelViewProjectionMatrix =  camera->GetProjectionMatrix() * modelViewMatrix;
        Matrix3f normalMatrix = modelViewMatrix.GetNormalMatrix();

        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, modelViewProjectionMatrix.Transpose());
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, normalMatrix.Transpose());
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(2, Vector3f(1.0f, 0.0f, 0.0f));
    }
}