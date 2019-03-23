#include "iglmesh.h"
#include "GLStructures.h"
#include "RAVertexHardwareBuffer.h"
#include "RAES2CoreVisualEffects.h"
#include "RAVertexBufferAccessor.h"
#include "Wm5Float3.h"
#include <igl/edges.h>
#include "RACamera.h"

namespace tyro 
{    
    void IGLMeshTwo::UpdateData(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C) 
    {
        int numVertices = V.rows();
        int numTriangles = F.rows();
        int numIndices = 3*numTriangles;
        int numNormals = N.rows();
        int stride = sizeof(VertexGeneral);

        if (numNormals != numVertices)
            RA_LOG_ERROR_ASSERT("Number of verticies and normals doesnt match");
        
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
                vba.Normal<Wm5::Float3>(vIndex) = Wm5::Float3(N(vid, 0), N(vid, 1), N(vid, 2));
                vba.Color<Wm5::Float3>(vIndex) = Wm5::Float3(C(fid, 0), C(fid, 1), C(fid, 2)); 
                vIndex++;
            }                
        }
        vba.Unmap();
    }

    void IGLMeshTwo::Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C)
    {
        ES2TriMesh::Init();

        //SetVisualEffect(ES2CoreVisualEffects::PBR());
        SetVisualEffect(ES2CoreVisualEffects::GourandDirectionalWithVColor());

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

        this->UpdateData(V,F,N,C);
        
        //compute bounding box
        LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
        vbuffer->Unmap();

	    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	    SetVertexArray(varray);        
        GetVisualEffect()->CullStateEnabled = false;
    }
  
    IGLMeshTwoSPtr IGLMeshTwo::Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::Vector3d& color) 
    {
        Eigen::MatrixXd C;
        C.resize(F.rows(), 3);
        for (int fid =0 ; fid <  F.rows(); ++fid) 
        {
            C.row(fid) = color;
        }
        
        IGLMeshTwoSPtr sptr = std::make_shared<IGLMeshTwo>();
        sptr->Init(V,F,N,C);
        return sptr;
    }
    
  
    void IGLMeshTwo::UpdateUniformsWithCamera(const tyro::Camera* camera)
    {   
  
        Wm5::HMatrix viewMatrix = camera->GetViewMatrix();
        Wm5::HMatrix modelViewMatrix = viewMatrix * WorldTransform.Matrix();
        Wm5::HMatrix modelViewProjectionMatrix =  camera->GetProjectionMatrix() * modelViewMatrix;
        Wm5::Matrix3f normalMatrix = modelViewMatrix.GetNormalMatrix();

        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, modelViewProjectionMatrix.Transpose());
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, normalMatrix.Transpose());
        //GetVisualEffect()->GetUniforms()->UpdateFloatUniform(2, mColor);
    }

    
}