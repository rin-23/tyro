#include "TyroIGLMesh.h"
#include "GLStructures.h"
#include "RAVertexHardwareBuffer.h"
#include "RAES2CoreVisualEffects.h"
#include "RAVertexBufferAccessor.h"
#include "Wm5Float3.h"
#include <igl/edges.h>

namespace tyro 
{
    void IGLMesh::Init(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd N)
    {
        ES2DefaultTriMesh::Init();
        SetColor(Wm5::Vector4f(0.4f, 0.4f, 0.4f, 1.0f));
        
        this->V = V;
        this->F = F;
        this->N = N;

        int numVertices = V.rows();
        int numTriangles = F.rows();
        int numIndices = 3*numTriangles;
        int numNormals = N.rows();
        int stride = sizeof(VertexNorm);

        if (numNormals != numVertices)
            RA_LOG_ERROR_ASSERT("Number of verticies and normals doesnt match");
        
        //vertex buffer data
        auto vbuffer = std::make_shared<tyro::ES2VertexHardwareBuffer>(stride, numVertices, nullptr, HardwareBuffer::BU_STATIC);
        VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), vbuffer.get());
        vba.MapWrite();
        for (int i = 0; i < numVertices; ++i) 
        {   
            //RA_LOG_INFO("V: %f %f %f", V(i, 0), V(i, 1), V(i, 2));
            vba.Position<Wm5::Float3>(i) = Wm5::Float3(V(i,0), V(i,1), V(i,2));
            //RA_LOG_INFO("N: %f %f %f", N(i, 0), N(i, 1), N(i, 2));
            vba.Normal<Wm5::Float3>(i) = Wm5::Float3(N(i, 0), N(i, 1), N(i, 2));
        }                
        vba.Unmap();

        //index buffer data
        auto ibuffer = std::make_shared<ES2IndexHardwareBuffer>(numIndices, nullptr, HardwareBuffer::BU_STATIC);
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

    IGLMeshSPtr IGLMesh::Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd N)
    {
        IGLMeshSPtr sptr = std::make_shared<IGLMesh>();
        sptr->Init(V,F,N);
        return sptr;
    }
    
    IGLMeshWireframeSPtr IGLMeshWireframe::Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F)
    {
        IGLMeshWireframeSPtr sptr = std::make_shared<IGLMeshWireframe>();
        sptr->Init(V,F);
        return sptr;
    }
        
    void IGLMeshWireframe::Init(Eigen::MatrixXd& V, Eigen::MatrixXi& F)
    {
        ES2DefaultPolyline::Init(false);
        SetColor(Wm5::Vector4f(0.05,0.05,0.05,1));
    
        //Calculate unique edges
        Eigen::MatrixXi E;
        igl::edges(F, E);
        
        int numVertices = V.rows();
        int numTriangles = F.rows();
        int numEdges = E.rows();

        int numPoints = numEdges * 2;
        int numIndices = 3*numTriangles;
        int stride = sizeof(Wm5::Vector3f);
    
        //vertex buffer data
        auto vbuffer = std::make_shared<tyro::ES2VertexHardwareBuffer>(stride, numPoints, nullptr, HardwareBuffer::BU_STATIC);
        VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), vbuffer.get());
        int vIndex = 0;
        
        vba.MapWrite();        
        for (int i = 0; i < numEdges; ++i) 
        {   
            //RA_LOG_INFO("V: %f %f %f", V(i, 0), V(i, 1), V(i, 2));
            int e1 = E(i,0);
            int e2 = E(i,1);
            
            vba.Position<Wm5::Float3>(vIndex++) = Wm5::Float3(V(e1,0), V(e1,1), V(e1,2));
            vba.Position<Wm5::Float3>(vIndex++) = Wm5::Float3(V(e2,0), V(e2,1), V(e2,2));
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
    
}