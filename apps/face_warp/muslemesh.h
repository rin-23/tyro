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
#include "RAES2CoreVisualEffects.h"
#include "RACamera.h"
#include <Eigen/Core>
#include "GLStructures.h"
#include "RAVertexHardwareBuffer.h"
#include "RAVertexBufferAccessor.h"
#include "RAFileManager.h"
#include "RAES2VisualEffect.h"
#include "RAES2BufferTexture.h"

namespace tyro
{
    class MuscleMesh : public ES2TriMesh
    {
    public:
        MuscleMesh() {}
        
        virtual ~MuscleMesh() {}
        
        static MuscleMeshSPtr Create(const Eigen::MatrixXd& V, 
                                     const Eigen::MatrixXi& F, 
                                     const Eigen::MatrixXd& N, 
                                     const Eigen::VectorXd& MV, 
                                     const Eigen::MatrixXd& TV, 
                                     const Eigen::MatrixXi& TT,
                                     const Eigen::VectorXi& FtoT,
                                     const Eigen::MatrixXd& G);
        
        virtual void SetColor(const Eigen::Vector4f& color);
        
        virtual void UpdateUniformsWithCamera(const Camera* camera) override;

        ES2VisualEffectSPtr VisualEffect()
        {
            ES2ShaderProgram* shader = new ES2ShaderProgram();
            shader->LoadProgram(GetFilePath("Muscle", "vs"), GetFilePath("Muscle", "fs"));
            
            ES2VertexFormat* vertexFormat = new ES2VertexFormat(4);
            vertexFormat->SetAttribute(0, shader->GetAttributeLocation("aPosition"), 0, ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_POSITION);
            vertexFormat->SetAttribute(1, shader->GetAttributeLocation("aNormal"), vertexFormat->GetOffsetForNextAttrib(0), ES2VertexFormat::AT_FLOAT3, ES2VertexFormat::AU_NORMAL);
            vertexFormat->SetAttribute(2, shader->GetAttributeLocation("aDiffusion"), vertexFormat->GetOffsetForNextAttrib(1), ES2VertexFormat::AT_FLOAT1, ES2VertexFormat::AU_BLENDWEIGHT_1);
            vertexFormat->SetAttribute(3, shader->GetAttributeLocation("aTetId"), vertexFormat->GetOffsetForNextAttrib(2), ES2VertexFormat::AT_INT1, ES2VertexFormat::AU_JOINT_INDEX_1);
            
            ES2ShaderUniforms* uniforms = new ES2ShaderUniforms(10);
            uniforms->SetUniform(0, shader->GetUniformLocation("uMVPMatrix"), 1, "uMVPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
            uniforms->SetUniform(1, shader->GetUniformLocation("uNMatrix"), 1, "uNMatrix", ES2ShaderUniforms::UniformMatrix3fv);
            uniforms->SetUniform(2, shader->GetUniformLocation("uColor"), 1, "uColor", ES2ShaderUniforms::Uniform4fv);
            uniforms->SetUniform(3, shader->GetUniformLocation("uMVMatrix"), 1, "uMVMatrix", ES2ShaderUniforms::UniformMatrix4fv);
            uniforms->SetUniform(4, shader->GetUniformLocation("uVerSampler"), 1, "uVerSampler", ES2ShaderUniforms::Uniform1iv);
            uniforms->SetUniform(5, shader->GetUniformLocation("uNormSampler"), 1, "uNormSampler", ES2ShaderUniforms::Uniform1iv);
            uniforms->SetUniform(6, shader->GetUniformLocation("uViewport"), 1, "uViewport", ES2ShaderUniforms::Uniform4fv);
            uniforms->SetUniform(7, shader->GetUniformLocation("uPMatrix"), 1, "uPMatrix", ES2ShaderUniforms::UniformMatrix4fv);
            uniforms->SetUniform(8, shader->GetUniformLocation("uDiffusedValuesSampler"), 1, "uDiffusedValuesSampler", ES2ShaderUniforms::Uniform1iv);
            uniforms->SetUniform(9, shader->GetUniformLocation("uGradSampler"), 1, "uGradSampler", ES2ShaderUniforms::Uniform1iv);

            ES2VisualEffectSPtr effect(std::make_shared<ES2VisualEffect>(shader, vertexFormat, uniforms));

            return effect;
        }
    protected:
        Eigen::Vector4f mColor;
        
        void Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, 
                  const Eigen::VectorXd& MV, const Eigen::MatrixXd& TV, const Eigen::MatrixXi& TT,
                  const Eigen::VectorXi& FtoT, const Eigen::MatrixXd& G);
    };
}


namespace tyro
{
    void MuscleMesh::Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, 
                          const Eigen::VectorXd& MV, const Eigen::MatrixXd& TV, const Eigen::MatrixXi& TT,
                          const Eigen::VectorXi& FtoT, const Eigen::MatrixXd& G) 
    {
        // ES2TriMesh::Init();
        // SetVisualEffect(ES2CoreVisualEffects::GourandDirectional());
        assert(N.rows() == F.rows());
        ES2TriMesh::Init();
        SetVisualEffect(MuscleMesh::VisualEffect());
        SetColor(Eigen::Vector4f(1.0f, 0.0f, 0.0f, 1.0f));
        
        int numVertices = V.rows();
        int numTriangles = F.rows();
        int numIndices = 3*numTriangles;
        int numNormals = N.rows();

        struct MuscleVertex
        {
            GLfloat position[3];
            GLfloat normal[3];
            GLfloat diffusion;
            GLint tetidx;
        };

        int stride = sizeof(MuscleVertex);

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
                vba.Position<Eigen::Vector3f>(vIndex) = Eigen::Vector3f(V(vid,0), V(vid,1), V(vid,2));
                vba.Normal<Eigen::Vector3f>(vIndex) = Eigen::Vector3f(N(fid, 0), N(fid, 1), N(fid, 2));
                vba.BlendWeight1<float>(vIndex) = 0.0; //float(MV(vid));
                int tid = FtoT(fid);
                vba.JointIndex1<int>(vIndex) = tid;

                vIndex++;
            }                
        }
        vba.Unmap();        
        
        //compute bounding box
        LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
        vbuffer->Unmap();

	    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	    SetVertexArray(varray);        
        GetVisualEffect()->CullStateEnabled = true; 


        // generate vertex texture buffer
        assert(TT.cols()==4);
        int numEntries = 4*TT.rows();
        Eigen::Vector3f* vertexDataPtr = new Eigen::Vector3f[numEntries]; 
        for (int tid = 0; tid < TT.rows(); ++tid) 
        {   
            int tid_offset = 4*tid;
            for (int j = 0; j < 4; ++j) 
            {   
                int vid = TT(tid,j);
                vertexDataPtr[tid_offset + j] = Eigen::Vector3f(TV(vid,0), TV(vid,1), TV(vid,2));
            }                
        }

        // generate normal texture buffer
        Eigen::Vector3f* normalDataPtr = new Eigen::Vector3f[numEntries]; 
        assert(N.rows() ==  4*TT.rows());

        for (int tid = 0; tid < TT.rows(); ++tid) 
        {   
            int tid_offset = 4*tid;
            for (int j = 0; j < TT.cols(); ++j) 
            {   
                int nid = 4*tid + j;
                normalDataPtr[tid_offset + j] = Eigen::Vector3f(N(nid,0), N(nid,1), N(nid,2));
            }                
        }

        // generate diffused texture buffer
        Eigen::Vector4f* diffusedDataPtr = new Eigen::Vector4f[TT.rows()]; 
        for (int tid = 0; tid < TT.rows(); ++tid) 
        {   
            // for (int j = 0; j < TT.cols(); ++j) 
            // {   
            Eigen::Vector4f tidval = Eigen::Vector4f(MV(TT(tid,0)), MV(TT(tid,1)), MV(TT(tid,2)), MV(TT(tid,3)));
            diffusedDataPtr[tid] = tidval;
            // }                
        }

       
        Eigen::Vector3f* gradDataPtr = new Eigen::Vector3f[TT.rows()]; 
        for (int tid = 0; tid < TT.rows(); ++tid) 
        {   
            // for (int j = 0; j < TT.cols(); ++j) 
            // {   
            Eigen::Vector3f tidval = Eigen::Vector3f(G(tid,0), G(tid,1), G(tid,2));
            gradDataPtr[tid] = tidval;
            // }                
        }

        ES2BufferTextureSPtr vertexBufTex =   std::make_shared<ES2BufferTexture>(ES2BufferTexture::TY_RGB32F, numEntries, vertexDataPtr,   HardwareBuffer::Usage::BU_STATIC);  
        ES2BufferTextureSPtr normalBufTex =   std::make_shared<ES2BufferTexture>(ES2BufferTexture::TY_RGB32F, numEntries, normalDataPtr,   HardwareBuffer::Usage::BU_STATIC);  
        ES2BufferTextureSPtr diffusedBufTex = std::make_shared<ES2BufferTexture>(ES2BufferTexture::TY_RGBA32F, TT.rows(), diffusedDataPtr, HardwareBuffer::Usage::BU_STATIC);  
        ES2BufferTextureSPtr gradBufTex     = std::make_shared<ES2BufferTexture>(ES2BufferTexture::TY_RGB32F, TT.rows(), gradDataPtr, HardwareBuffer::Usage::BU_STATIC);  
        
        GetVisualEffect()->AddBufferTexture(vertexBufTex);
        GetVisualEffect()->AddBufferTexture(normalBufTex);
        GetVisualEffect()->AddBufferTexture(diffusedBufTex);
        GetVisualEffect()->AddBufferTexture(gradBufTex);

        delete[] vertexDataPtr;
        delete[] normalDataPtr;
        delete[] diffusedDataPtr;
        delete[] gradDataPtr;
    }
        
    MuscleMeshSPtr MuscleMesh::Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F,   
                                      const Eigen::MatrixXd& N, const Eigen::VectorXd& MV, 
                                      const Eigen::MatrixXd& TV, const Eigen::MatrixXi& TT,
                                      const Eigen::VectorXi& FtoT,  const Eigen::MatrixXd& G) 
    {
        MuscleMeshSPtr ptr = std::make_shared<MuscleMesh>();
        ptr->Init(V,F,N,MV,TV,TT,FtoT,G);
        return ptr;
    }

    void MuscleMesh::SetColor(const Eigen::Vector4f& color)
    {
        mColor = color;
    }

    void MuscleMesh::UpdateUniformsWithCamera(const tyro::Camera* camera)
    {
        Wm5::HMatrix viewMatrix = camera->GetViewMatrix();
        Wm5::HMatrix projectionMatrix = camera->GetProjectionMatrix();
        Wm5::HMatrix modelViewMatrix = viewMatrix * WorldTransform.Matrix();
        Wm5::HMatrix modelViewProjectionMatrix =  camera->GetProjectionMatrix() * modelViewMatrix;
        Wm5::Matrix3f normalMatrix = modelViewMatrix.GetNormalMatrix();

        Wm5::Vector4i vport_i = camera->GetViewport();
        Wm5::Vector4f vport_f(vport_i[0], vport_i[1], vport_i[2],vport_i[3]);

        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, modelViewProjectionMatrix.Transpose());
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, normalMatrix.Transpose());
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(2, mColor.data());
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(3, modelViewMatrix.Transpose());
        
        int vertexbuf = GetVisualEffect()->GetBufferTexture(0)->GetTextureID();
        GetVisualEffect()->GetUniforms()->UpdateIntUniform(4, &vertexbuf);
        
        int normtexbuf = GetVisualEffect()->GetBufferTexture(1)->GetTextureID();
        GetVisualEffect()->GetUniforms()->UpdateIntUniform(5, &normtexbuf);

        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(6, vport_f);
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(7, projectionMatrix.Transpose());

        int diffusedtexbuf = GetVisualEffect()->GetBufferTexture(2)->GetTextureID();
        GetVisualEffect()->GetUniforms()->UpdateIntUniform(8, &diffusedtexbuf);

        int gradtexbuf = GetVisualEffect()->GetBufferTexture(3)->GetTextureID();
        GetVisualEffect()->GetUniforms()->UpdateIntUniform(9, &gradtexbuf);

    }
}