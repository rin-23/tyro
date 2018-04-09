#include "TyroIGLMesh.h"
#include "GLStructures.h"
#include "RAVertexHardwareBuffer.h"
#include "RAES2CoreVisualEffects.h"
#include "RAVertexBufferAccessor.h"
#include "Wm5Float3.h"

namespace tyro 
{

    void ES2IGLMesh::Init(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd N)
    {
        ES2DefaultTriMesh::Init();
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
            vba.Position<Wm5::Float3>(i) = Wm5::Float3(V(i,0), V(i,1), V(i,2));
            vba.Normal<Wm5::Float3>(i) = Wm5::Float3(N(i,0), N(i,1), N(i,2));
        }                
        vba.Unmap();

        //index buffer data
        auto ibuffer = std::make_shared<ES2IndexHardwareBuffer>(numIndices, nullptr, HardwareBuffer::BU_STATIC);
        uint32_t* indices = static_cast<uint32_t*>(ibuffer->MapWrite());
        int iIndex = 0;
        for (int i = 0; i < numIndices; ++i) 
        {
            indices[iIndex++] = F(i,0);
            indices[iIndex++] = F(i,1);
            indices[iIndex++] = F(i,2);
        }
        ibuffer->Unmap();

        //compute bounding box
        LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
        vbuffer->Unmap();

        SetVertexBuffer(vbuffer);
	    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	    SetVertexArray(varray);
        SetIndexBuffer(ibuffer);
    }

    ES2IGLMeshSPtr ES2IGLMesh::Create(Eigen::MatrixXd& V, Eigen::MatrixXi& F, Eigen::MatrixXd N)
    {
        ES2IGLMeshSPtr sptr = std::make_shared<ES2IGLMesh>();
        sptr->Init(V,F,N);
        return sptr;
    }

    

}