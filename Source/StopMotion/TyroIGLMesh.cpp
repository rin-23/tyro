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
    //void UpdateData(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C, const Eigen::VectorXd& AO){}
    
    void IGLMesh::UpdateData(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C, const Eigen::VectorXd& Error, float max_error, Eigen::Vector3f& EmaxColor)
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
        Wm5::Vector3f minColor(0.8, 0.8, 0.8);
        //Wm5::Vector3f maxColor(147,112,219);
        Wm5::Vector3f maxColor = Wm5::Vector3f(EmaxColor(0),EmaxColor(1),EmaxColor(2));
        //maxColor *= 1/255.0;
        
        for (int fid = 0; fid < numTriangles; ++fid) 
        {   
            //RA_LOG_INFO("V: %f %f %f", V(i, 0), V(i, 1), V(i, 2));
            //RA_LOG_INFO("N: %f %f %f", N(i, 0), N(i, 1), N(i, 2));
            for (int j = 0; j < 3; ++j) 
            {   
                int vid = F(fid,j);
                vba.Position<Wm5::Float3>(vIndex) = Wm5::Float3(V(vid,0), V(vid,1), V(vid,2));
                vba.Normal<Wm5::Float3>(vIndex) = Wm5::Float3(N(vid, 0), N(vid, 1), N(vid, 2));
                float err = Error(vid);
                float weight = err/max_error;
                //std::cout << weight << " ";
                Wm5::Vector3f finalColor = (1-weight)*minColor + weight*maxColor;
                vba.Color<Wm5::Float3>(vIndex) = Wm5::Float3(finalColor[0], finalColor[1], finalColor[2]); ;//Wm5::Float3(r, 0, 0);
                vIndex++;
            }                
        }
        vba.Unmap();
    }

    void IGLMesh::UpdateData(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::Vector3d& color)
    {
        Eigen::MatrixXd C;
        C.resize(F.rows(), 3);
        for (int fid =0 ; fid <  F.rows(); ++fid) 
        {
            C.row(fid) = color;
        }
        
        UpdateData(V, F, N, C);
    }

    void IGLMesh::UpdateData(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C) 
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

    void IGLMesh::Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C)
    {
        ES2TriMesh::Init();

        SetVisualEffect(ES2CoreVisualEffects::PBR());
        //SetVisualEffect(ES2CoreVisualEffects::PBR());
        isPBR = true;

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
        GetVisualEffect()->GetCullState()->Enabled = false;
    
    }

    void IGLMesh::Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C, const Eigen::VectorXd& AO) 
    {
        ES2TriMesh::Init();

        SetVisualEffect(ES2CoreVisualEffects::PBR2());
        isPBR = true;


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
                
                //put AO instead
                vba.Color<Wm5::Float3>(vIndex) = Wm5::Float3(AO(vid), 0, 0);
                
                //vba.Color<Wm5::Float3>(vIndex) = Wm5::Float3(C(fid, 0), C(fid, 1), C(fid, 2));
                vIndex++;
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
        //GetVisualEffect()->GetPolygonOffset()->Enabled = true;
        //GetVisualEffect()->GetPolygonOffset()->Offset = -5.0;
    }


    void IGLMesh::InitColor(const Eigen::MatrixXd& V, 
                            const Eigen::MatrixXi& F, 
                            const Eigen::MatrixXd& N, 
                            const Eigen::MatrixXd& C, 
                            const Eigen::VectorXd& Error,
                            float max_error, 
                            Eigen::Vector3f& maxColor) 
    {
        ES2TriMesh::Init();

        SetVisualEffect(ES2CoreVisualEffects::GourandDirectionalWithVColor());
        //SetVisualEffect(ES2CoreVisualEffects::PBR());
        isPBR = false;
 
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
        
        this->UpdateData(V, F, N, C, Error, max_error, maxColor);

        //compute bounding box
        LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
        vbuffer->Unmap();

	    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	    SetVertexArray(varray);        
        GetVisualEffect()->GetCullState()->Enabled = false;
    }


    IGLMeshSPtr IGLMesh::Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C)
    {
        IGLMeshSPtr sptr = std::make_shared<IGLMesh>();
        sptr->Init(V,F,N,C);
        return sptr;
    }

    IGLMeshSPtr IGLMesh::Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C, const Eigen::VectorXd& AO)
    {
        IGLMeshSPtr sptr = std::make_shared<IGLMesh>();
        sptr->Init(V,F,N,C,AO);
        return sptr;
    }
    
    IGLMeshSPtr IGLMesh::Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::Vector3d& color) 
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
    
    IGLMeshSPtr IGLMesh::CreateColor(const Eigen::MatrixXd& V, const Eigen::MatrixXi& F, const Eigen::MatrixXd& N, const Eigen::MatrixXd& C, const Eigen::VectorXd& error, float max_error, Eigen::Vector3f& maxColor)
    {
        IGLMeshSPtr sptr = std::make_shared<IGLMesh>();
        sptr->InitColor(V,F,N,C,error, max_error, maxColor);
        return sptr;
    }


    void IGLMesh::UpdateUniformsWithCamera(const tyro::Camera* camera)
    {   
        if (!isPBR)
        {
        
            Wm5::HMatrix viewMatrix = camera->GetViewMatrix();
            Wm5::HMatrix modelViewMatrix = viewMatrix * WorldTransform.Matrix();
            Wm5::HMatrix modelViewProjectionMatrix =  camera->GetProjectionMatrix() * modelViewMatrix;
            Wm5::Matrix3f normalMatrix = modelViewMatrix.GetNormalMatrix();

            GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, modelViewProjectionMatrix.Transpose());
            GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, normalMatrix.Transpose());
            //GetVisualEffect()->GetUniforms()->UpdateFloatUniform(2, mColor);
        }
        else 
        {
        
            Wm5::HMatrix viewMatrix = camera->GetViewMatrix();
            Wm5::HMatrix modelMatrix = WorldTransform.Matrix();
            Wm5::HMatrix projectionMatrix = camera->GetProjectionMatrix();

            Wm5::HMatrix modelViewMatrix = viewMatrix * modelMatrix;
            Wm5::HMatrix modelViewProjectionMatrix =  camera->GetProjectionMatrix() * modelViewMatrix;
            Wm5::Matrix3f normalMatrix = modelViewMatrix.GetNormalMatrix();
            //Wm5::HMatrix modelViewMatrix = viewMatrix * modelMatrix;
            //Wm5::HMatrix modelViewProjectionMatrix =  camera->GetProjectionMatrix() * modelViewMatrix;
        // Wm5::Matrix3f normalMatrix = modelViewMatrix.GetNormalMatrix();
        
            //uniforms->SetUniform(0, shader->GetUniformLocation("projection"), 1, "projection", ES2ShaderUniforms::UniformMatrix4fv);
            GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, projectionMatrix.Transpose());
            
            //uniforms->SetUniform(1, shader->GetUniformLocation("view"), 1, "view", ES2ShaderUniforms::UniformMatrix4fv);
            GetVisualEffect()->GetUniforms()->UpdateFloatUniform(1, viewMatrix.Transpose());

            //uniforms->SetUniform(2, shader->GetUniformLocation("model"), 1, "model", ES2ShaderUniforms::UniformMatrix4fv);
            GetVisualEffect()->GetUniforms()->UpdateFloatUniform(2, modelMatrix.Transpose());

            //uniforms->SetUniform(3, shader->GetUniformLocation("albedo"), 1, "albedo", ES2ShaderUniforms::Uniform3fv);
            //GetVisualEffect()->GetUniforms()->UpdateFloatUniform(3, Wm5::Vector3f(255/255.0, 148/255.0, 135/255.0));
            //GetVisualEffect()->GetUniforms()->UpdateFloatUniform(3, Wm5::Vector3f(200/255.0, 10/255.0, 10/255.0));
            
            //uniforms->SetUniform(4, shader->GetUniformLocation("metallic"), 1, "metallic", ES2ShaderUniforms::Uniform1fv);
            float metallic[1] = {0.1f};
            GetVisualEffect()->GetUniforms()->UpdateFloatUniform(3, metallic);

            //uniforms->SetUniform(5, shader->GetUniformLocation("roughness"), 1, "roughness", ES2ShaderUniforms::Uniform1fv);
            float roughness[1] = {0.7f};
            GetVisualEffect()->GetUniforms()->UpdateFloatUniform(4, roughness);

            //uniforms->SetUniform(6, shader->GetUniformLocation("ao"), 1, "ao", ES2ShaderUniforms::Uniform1fv);
            float ao[1] = {0.5f};
            GetVisualEffect()->GetUniforms()->UpdateFloatUniform(5, ao);

            
            //uniforms->SetUniform(7, shader->GetUniformLocation("lightPositions"), 1, "lightPositions", ES2ShaderUniforms::Uniform3fv);
            Wm5::Vector3f lightPositions[] = {
            
            Wm5::Vector3f(10.0f,  10.0f, 20.0f),
            Wm5::Vector3f(-10.0f,  10.0f, 20.0f),
            Wm5::Vector3f(-10.0f,  10.0f, 20.0f),
            Wm5::Vector3f(-10.0f,  10.0f, 20.0f),
            
            //BUNNY LIGHTS
            //Wm5::Vector3f(10.0f,  10.0f, 10.0f),
           // Wm5::Vector3f(-10.0f,  10.0f, 10.0f),
           // Wm5::Vector3f(-10.0f,  10.0f, 10.0f),
           // Wm5::Vector3f(-10.0f,  10.0f, 10.0f),
            
            //Wm5::Vector3f( 10.0f,  10.0f, 10.0f),
            //Wm5::Vector3f(-10.0f, -10.0f, 10.0f),
            //Wm5::Vector3f( 10.0f, -10.0f, 10.0f),
            };
            int s = 7;
            for (int i =0;i<4;++i)
                GetVisualEffect()->GetUniforms()->UpdateFloatUniform(6+i, lightPositions[i]);

    //        uniforms->SetUniform(8, shader->GetUniformLocation("lightColors"), 1, "lightColors", ES2ShaderUniforms::Uniform3fv);
            GetVisualEffect()->GetUniforms()->UpdateFloatUniform(10, Wm5::Vector3f(210.0f, 210.0f, 210.0f));

            Wm5::Vector3f pos = camera->GetPosition();
            //uniforms->SetUniform(9, shader->GetUniformLocation("camPos"), 1, "camPos", ES2ShaderUniforms::Uniform3fv);
            GetVisualEffect()->GetUniforms()->UpdateFloatUniform(11, pos);

            //GetVisualEffect()->GetUniforms()->UpdateFloatUniform(10, normalMatrix.Transpose());
        
        }
    }

    void IGLMeshWireframe::UpdateData(const Eigen::MatrixXd& V, const Eigen::MatrixXi& uE, const Eigen::MatrixXd& uC) 
    {
        //int numVertices = V.rows();
        int numEdges = uE.rows();
        int numPoints = numEdges * 2;
        int stride = sizeof(WireframeGeneral);
    
        //vertex buffer data        
        VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), GetVertexBuffer().get());
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
    }

        
    void IGLMeshWireframe::Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& uE, const Eigen::MatrixXd& uC)
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
        SetVertexBuffer(vbuffer);
        this->UpdateData(V, uE, uC);
        
        //compute bounding box
        LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
        vbuffer->Unmap();
       
	    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	    SetVertexArray(varray);

        GetVisualEffect()->GetCullState()->Enabled = false;    
        GetVisualEffect()->GetPolygonOffset()->Enabled = true;
        GetVisualEffect()->GetPolygonOffset()->Factor = 0;
        GetVisualEffect()->GetPolygonOffset()->Units = -5;
        GetVisualEffect()->GetPolygonOffset()->IsSolid = false;
        
    }

     void IGLMeshWireframe::Init(const Eigen::MatrixXd& V, const Eigen::MatrixXi& uE, const Eigen::MatrixXd& uC, const std::vector<int>& eid_list)
    {   
        ES2Polyline::Init(false);
        SetVisualEffect(ES2CoreVisualEffects::WireframeColor());

        //Calculate unique edges
        
        //int numVertices = V.rows();
        int numEdges = eid_list.size();
        int numPoints = numEdges * 2;
        int stride = sizeof(WireframeGeneral);
    
        //vertex buffer data
        auto vbuffer = std::make_shared<tyro::ES2VertexHardwareBuffer>(stride, numPoints, nullptr, HardwareBuffer::BU_DYNAMIC);
        SetVertexBuffer(vbuffer);
        //this->UpdateData(V, uE, uC);
        
        //vertex buffer data        
        VertexBufferAccessor vba(GetVisualEffect()->GetVertexFormat(), GetVertexBuffer().get());
        int vIndex = 0;
        vba.MapWrite();        
        for (int j = 0; j < numEdges; ++j) 
        {   
            //RA_LOG_INFO("V: %f %f %f", V(i, 0), V(i, 1), V(i, 2));
            int i = eid_list[j];
            int v1 = uE(i,0);
            int v2 = uE(i,1);
            
            vba.Position<Wm5::Float3>(vIndex) = Wm5::Float3(V(v1,0), V(v1,1), V(v1,2));
            vba.Color<Wm5::Float3>(vIndex++) = Wm5::Float3(0, 0.8, 0);
            vba.Position<Wm5::Float3>(vIndex) = Wm5::Float3(V(v2,0), V(v2,1), V(v2,2));
            vba.Color<Wm5::Float3>(vIndex++) = Wm5::Float3(0, 0.8, 0);
        }                
        vba.Unmap();

        //compute bounding box
        LocalBoundBox.ComputeExtremes(vbuffer->GetNumOfVerticies(), vbuffer->GetVertexSize(), vbuffer->MapRead());
        vbuffer->Unmap();
       
	    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), vbuffer);
	    SetVertexArray(varray);

        GetVisualEffect()->GetCullState()->Enabled = false;    
        GetVisualEffect()->GetPolygonOffset()->Enabled = true;
        GetVisualEffect()->GetPolygonOffset()->Factor = 0;
        GetVisualEffect()->GetPolygonOffset()->Units = -5;
        GetVisualEffect()->GetPolygonOffset()->IsSolid = false;
        
    }

    IGLMeshWireframeSPtr IGLMeshWireframe::Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& uE, const Eigen::MatrixXd& uC)
    {
        IGLMeshWireframeSPtr sptr = std::make_shared<IGLMeshWireframe>();
        sptr->Init(V,uE,uC);
        return sptr;
    }
    
    IGLMeshWireframeSPtr IGLMeshWireframe::Create(const Eigen::MatrixXd& V, const Eigen::MatrixXi& uE, 
                                                  const Eigen::MatrixXd& uC, const std::vector<int>& eid_list) 
    {
        IGLMeshWireframeSPtr sptr = std::make_shared<IGLMeshWireframe>();
        sptr->Init(V, uE, uC, eid_list);
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