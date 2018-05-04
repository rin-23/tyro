#include "TyroIGLMesh.h"
#include "GLStructures.h"
#include "RAVertexHardwareBuffer.h"
#include "RAES2CoreVisualEffects.h"
#include "RAVertexBufferAccessor.h"
#include "Wm5Float3.h"
#include <igl/edges.h>
#include "RACamera.h"

namespace tyro 
{   

     void IGLMesh::Init(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& N, Eigen::MatrixXd& C)
    {
        ES2TriMesh::Init();

        ES2TriMesh::Init();
        SetVisualEffect(ES2CoreVisualEffects::GourandDirectionalWithVColor());
        
        this->V = V;
        this->F = F;
        this->N = N;
        this->C = C;

        int numVertices = V.rows();
        int numTriangles = F.rows();
        int numIndices = 3*numTriangles;
        int numNormals = N.rows();
        int stride = sizeof(VertexGeneral);

        if (numNormals != numVertices)
            RA_LOG_ERROR_ASSERT("Number of verticies and normals doesnt match");
        
        //vertex buffer data
        auto vbuffer = std::make_shared<tyro::ES2VertexHardwareBuffer>(stride, numIndices, nullptr, HardwareBuffer::BU_DYNAMIC);
        VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), vbuffer.get());
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
                vba.Color<Wm5::Float3>(vIndex++) = Wm5::Float3(C(fid, 0), C(fid, 1), C(fid, 2));
            }                
        }
        vba.Unmap();

        
        //compute bounding box
        LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
        vbuffer->Unmap();

        SetVertexBuffer(vbuffer);
	    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	    SetVertexArray(varray);        
        GetVisualEffect()->GetCullState()->Enabled = false;
        GetVisualEffect()->GetPolygonOffset()->Enabled = true;
        GetVisualEffect()->GetPolygonOffset()->Offset = 1.0;
    }

    /*
    void IGLMesh::Init(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& N, Eigen::MatrixXd& C)
    {
        ES2TriMesh::Init();

        ES2TriMesh::Init();
        SetVisualEffect(ES2CoreVisualEffects::GourandDirectionalWithVColor());
        
        this->V = V;
        this->F = F;
        this->N = N;
        this->C = C;

        int numVertices = V.rows();
        int numTriangles = F.rows();
        int numIndices = 3*numTriangles;
        int numNormals = N.rows();
        int stride = sizeof(VertexGeneral);

        if (numNormals != numVertices)
            RA_LOG_ERROR_ASSERT("Number of verticies and normals doesnt match");
        
        //vertex buffer data
        auto vbuffer = std::make_shared<tyro::ES2VertexHardwareBuffer>(stride, numVertices, nullptr, HardwareBuffer::BU_DYNAMIC);
        VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), vbuffer.get());
        vba.MapWrite();
        for (int i = 0; i < numVertices; ++i) 
        {   
            //RA_LOG_INFO("V: %f %f %f", V(i, 0), V(i, 1), V(i, 2));
            //RA_LOG_INFO("N: %f %f %f", N(i, 0), N(i, 1), N(i, 2));
            vba.Position<Wm5::Float3>(i) = Wm5::Float3(V(i,0), V(i,1), V(i,2));
            vba.Normal<Wm5::Float3>(i) = Wm5::Float3(N(i, 0), N(i, 1), N(i, 2));
            vba.Color<Wm5::Float3>(i) = Wm5::Float3(C(i, 0), C(i, 1), C(i, 2));
        }                
        vba.Unmap();

        //index buffer data
        auto ibuffer = std::make_shared<ES2IndexHardwareBuffer>(numIndices, nullptr, HardwareBuffer::BU_DYNAMIC);
        uint32_t* indices = static_cast<uint32_t*>(ibuffer->MapWrite());
        int iIndex = 0;
        for (int i = 0; i < numTriangles; ++i) 
        {         
            //RA_LOG_INFO("F: %i %i %i", F(i, 0), F(i, 1), F(i, 2));  
            indices[iIndex++] = F(i, 0);
            indices[iIndex++] = F(i, 1);
            indices[iIndex++] = F(i, 2);
        }
        ibuffer->Unmap();

        //compute bounding box
        LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
        vbuffer->Unmap();

        SetVertexBuffer(vbuffer);
	    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	    SetVertexArray(varray);
        SetIndexBuffer(ibuffer);

        GetVisualEffect()->GetCullState()->Enabled = false;
    }
    */

    IGLMeshSPtr IGLMesh::Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& N, Eigen::MatrixXd& C)
    {
        IGLMeshSPtr sptr = std::make_shared<IGLMesh>();
        sptr->Init(V,F,N,C);
        return sptr;
    }
    
    IGLMeshSPtr IGLMesh::Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& N, Eigen::Vector3d& color) 
    {
        Eigen::MatrixXd C;
        C.resize(F.rows(), 3);
        for (int fid =0 ; fid <  F.rows(); ++fid) 
        {
            C.row(fid) = color;
        }
        
        IGLMeshSPtr sptr = std::make_shared<IGLMesh>();
        sptr->Init(V,F,N,C);
        return sptr;
    }
    
    void IGLMesh::UpdateUniformsWithCamera(const tyro::Camera* camera)
    {
        Wm5::HMatrix viewMatrix = camera->GetViewMatrix();
        Wm5::HMatrix modelViewMatrix = viewMatrix * WorldTransform.Matrix();
        Wm5::HMatrix modelViewProjectionMatrix =  camera->GetProjectionMatrix() * modelViewMatrix;
        Wm5::Matrix3f normalMatrix = modelViewMatrix.GetNormalMatrix();

        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, modelViewProjectionMatrix.Transpose());
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, normalMatrix.Transpose());
        //GetVisualEffect()->GetUniforms()->UpdateFloatUniform(2, mColor);
    }
        
    void IGLMeshWireframe::Init(Eigen::MatrixXd& V, Eigen::MatrixXi& uE, Eigen::MatrixXd& uC)
    {   
        ES2Polyline::Init(false);
        SetVisualEffect(ES2CoreVisualEffects::WireframeColor());

        //Calculate unique edges
        
        //int numVertices = V.rows();
        int numEdges = uE.rows();
        int numPoints = numEdges * 2;
        int stride = sizeof(WireframeGeneral);
    
        //vertex buffer data
        auto vbuffer = std::make_shared<tyro::ES2VertexHardwareBuffer>(stride, numPoints, nullptr, HardwareBuffer::BU_DYNAMIC);
        VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), vbuffer.get());
        int vIndex = 0;
        
        vba.MapWrite();        
        for (int i = 0; i < numEdges; ++i) 
        {   
            //RA_LOG_INFO("V: %f %f %f", V(i, 0), V(i, 1), V(i, 2));
            int v1 = uE(i,0);
            int v2 = uE(i,1);
            
            vba.Position<Wm5::Float3>(vIndex) = Wm5::Float3(V(v1,0), V(v1,1), V(v1,2));
            vba.Color<Wm5::Float3>(vIndex++) = Wm5::Float3(uC(i,0), uC(i,1), uC(i,2));
            vba.Position<Wm5::Float3>(vIndex) = Wm5::Float3(V(v2,0), V(v2,1), V(v2,2));
            vba.Color<Wm5::Float3>(vIndex++) = Wm5::Float3(uC(i,0), uC(i,1), uC(i,2));
        }                
        vba.Unmap();
        
        //compute bounding box
        LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
        vbuffer->Unmap();

        SetVertexBuffer(vbuffer);
	    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	    SetVertexArray(varray);

        GetVisualEffect()->GetCullState()->Enabled = false;    
    }

    IGLMeshWireframeSPtr IGLMeshWireframe::Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd& C)
    {
        IGLMeshWireframeSPtr sptr = std::make_shared<IGLMeshWireframe>();
        sptr->Init(V,F,C);
        return sptr;
    }
    
    /*
    IGLMeshWireframeSPtr IGLMeshWireframe::Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::Vector3d& color)
    {
        Eigen::MatrixXd C;
        C.resize(V.rows(), V.cols());
        for (int i =0 ; i <  V.rows(); ++i) 
        {
            C.row(i) = color;
        }

        IGLMeshWireframeSPtr sptr = std::make_shared<IGLMeshWireframe>();
        sptr->Init(V,F,C);
        return sptr;
    }
    */
 
    void IGLMeshWireframe::UpdateUniformsWithCamera(const Camera* camera)
    {
        Wm5::HMatrix MVPMatrix = camera->GetProjectionMatrix() * camera->GetViewMatrix() * WorldTransform.Matrix();
        GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, MVPMatrix.Transpose());
        //GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, mColor);
    }
    
}