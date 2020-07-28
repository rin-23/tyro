//
//  RAMuscleMesh.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-09.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAEnginePrerequisites.h"
#include "RAES2TriMesh.h"
#include "Wm5Vector4.h"
#include "RAES2CoreVisualEffects.h"
#include "RACamera.h"
#include <Eigen/Core>
#include "GLStructures.h"
#include "RAVertexHardwareBuffer.h"
#include "RAVertexBufferAccessor.h"
#include "Wm5Float3.h"
#include "RAFileManager.h"
#include "RAES2VisualEffect.h"

namespace tyro
{
    class MuscleMesh : public ES2TriMesh
    {
    public:
        MuscleMesh() {}
        
        virtual ~MuscleMesh() {}
        
        static MuscleMeshSPtr Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::VectorXd& MV);
        
        virtual void SetColor(Wm5::Vector4f color);
        
        virtual void UpdateUniformsWithCamera(const Camera* camera) override;

        ES2VisualEffectSPtr VisualEffect()
        {
            ES2ShaderProgram* shader = new ES2ShaderProgram();
            shader->LoadProgram(GetFilePath("Muscle", "vsh"), GetFilePath("Muscle", "fsh"));
            
            ES2VertexFormat* vertexFormat = new ES2VertexFormat(3);
            vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
            vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aNormal"), vertexFormat->GetOffsetForNextAttrib(0), ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_NORMAL);
            vertexFormat->SetAttribute(2, shader->GetAttributeLocation("aDiffusion"), vertexFormat->GetOffsetForNextAttrib(1), ES2VertexFormat::AT_FLOAT1, ES2VertexFormat::AU_BLENDWEIGHT_1);
            
            ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(3);
            uniforms->SetUniform(0, shader->GetUniformLocation("uMVPMatrix"), 1, "uMVPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
            uniforms->SetUniform(1, shader->GetUniformLocation("uNMatrix"), 1, "uNMatrix", ES2ShaderUniforms::UniformMatrix3fv);
            uniforms->SetUniform(2, shader->GetUniformLocation("uColor"), 1, "uColor", ES2ShaderUniforms::Uniform4fv);
            
            ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms));

            return effect;
        }
    protected:
        Wm5::Vector4f mColor;
        
        void Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::VectorXd& MV);
        
        void InitSerialized();
    };
}


namespace tyro
{
   

    void MuscleMesh::Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::VectorXd& MV) 
    {
        // ES2TriMesh::Init();
        // SetVisualEffect(ES2CoreVisualEffects::GourandDirectional());
        assert(N.rows() == F.rows());
        ES2TriMesh::Init();
        SetVisualEffect(MuscleMesh::VisualEffect());
        SetColor(Wm5::Vector4f(1.0f, 0.0f, 0.0f, 1.0f));
        
        int numVertices = V.rows();
        int numTriangles = F.rows();
        int numIndices = 3*numTriangles;
        int numNormals = N.rows();
        int stride = sizeof(VertexGeneral);

        if (numNormals != numVertices)
            RA_LOG_ERROR_ASSERT("Number of verticies and normals doesnt match");
        
        //vertex buffer data
        auto vbuffer = std::make_shared<tyro::ES2VertexHardwareBuffer>(stride, numIndices, nullptr, HardwareBuffer::BU_DYNAMIC);
        SetVertexBuffer(vbuffer);

        //vertex buffer data
        VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), GetVertexBuffer().get());
        vba.MapWrite();
        int vIndex = 0;
        
        for (int fid = 0; fid < numTriangles; ++fid) 
        {   
            //RA_LOG_INFO("V: %f %f %f", V(i, 0), V(i, 1), V(i, 2));
            //RA_LOG_INFO("N: %f %f %f", N(i, 0), N(i, 1), N(i, 2));
            for (int j = 0; j < 3; ++j) 
            {   
                int vid = F(fid,j);
                vba.Position<Wm5::Float3>(vIndex) = Wm5::Float3(V(vid,0), V(vid,1), V(vid,2));
                vba.Normal<Wm5::Float3>(vIndex) = Wm5::Float3(N(fid, 0), N(fid, 1), N(fid, 2));
                vba.BlendWeight1<float>(vIndex) = float(MV(vid));
                // vba.Color<Wm5::Float3>(vIndex) = Wm5::Float3(0.0f, 1.0f, 0.0f); ;//Wm5::Float3(r, 0, 0);
                vIndex++;
            }                
        }
        vba.Unmap();        
        
        //compute bounding box
        LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
        vbuffer->Unmap();

	    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	    SetVertexArray(varray);        
        // GetVisualEffect()->CullStateEnabled = false;
    }
        
    void MuscleMesh::InitSerialized()
    {
        RA_LOG_ERROR_ASSERT("Why are you calling this?");

        SetVisualEffect(ES2CoreVisualEffects::GourandDirectional());
    }
        
    MuscleMeshSPtr MuscleMesh::Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::VectorXd& MV) 
    {
        MuscleMeshSPtr ptr = std::make_shared<MuscleMesh>();
        ptr->Init(V,F,N,MV);
        return ptr;
    }

    void MuscleMesh::SetColor(Wm5::Vector4f color)
    {
        mColor = color;
    }

    void MuscleMesh::UpdateUniformsWithCamera(const tyro::Camera* camera)
    {
        Wm5::HMatrix viewMatrix = camera->GetViewMatrix();
        Wm5::HMatrix modelViewMatrix = viewMatrix * WorldTransform.Matrix();
        Wm5::HMatrix modelViewProjectionMatrix =  camera->GetProjectionMatrix() * modelViewMatrix;
        Wm5::Matrix3f normalMatrix = modelViewMatrix.GetNormalMatrix();

        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, modelViewProjectionMatrix.Transpose());
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, normalMatrix.Transpose());
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(2, mColor);
    }

}