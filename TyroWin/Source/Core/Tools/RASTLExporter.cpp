//
//  RASTLExporter.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-11.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RASTLExporter.h"
#include "Wm5Float3.h"
#include "Wm5APoint.h"
#include "Wm5AVector.h"
#include "Wm5Transform.h"
#include "Wm5HMatrix.h"
#include "RASkeleton.h"
#include <memory>
#include "RAES2StandardMesh.h"
#include "RAVertexBufferAccessor.h"
//#include "OssaES2BoneTriMesh.h"

namespace Ossa
{

//int STLExporter::ExportToBinarySTL(const tyro::ES2TriMesh& mesh, const std::string& path)
//{
//    return ExportToBinarySTL(mesh, path, Transform::IDENTITY);
//}
//
int STLExporter::ExportToBinarySTL(tyro::ES2TriMesh* mesh, const std::string& path)
{
    FILE* pFile;
    pFile = fopen(path.data(), "wb");
    if (!pFile)
    {
        RA_LOG_ERROR_ASSERT("Failed to open %s", path.data());
        return 0;
    }
    
    uint8_t header[80];
    if(fwrite(header, sizeof(uint8_t), 80, pFile) != 80)
    {
        fclose(pFile);
        RA_LOG_ERROR_ASSERT("Failed to write header");
        return 0;
    }
    
    uint32_t nTriangles = mesh->GetNumTriangles();
    if (fwrite(&nTriangles, sizeof(uint32_t), 1, pFile) != 1)
    {
        fclose(pFile);
        RA_LOG_ERROR_ASSERT("Failed to write number of triangles");
        return 0;
    }
    
    tyro::VertexBufferAccessor vba(mesh->GetVisualEffect()->GetVertexFormat(), mesh->GetVertexBuffer().get());
    vba.MapRead();
    unsigned int* indicies = (unsigned int*) mesh->GetIndexBuffer()->MapRead();

    for (int tri_index = 0; tri_index < mesh->GetNumTriangles(); tri_index++)
    {
        Wm5::Vector3f positions[3];
        for (int i = 0; i < 3; ++i)
        {
            int index = indicies[3*tri_index + i];
            
            Wm5::APoint pos = vba.Position<Wm5::Float3>(index);
            Wm5::APoint t_pos = mesh->WorldTransform * pos;
            positions[i] = t_pos.operator Wm5::Vector3<float> &();
        }
        
        Wm5::Vector3f normal = (positions[2] - positions[1]).Cross(positions[3] - positions[1]);
        normal.Normalize();
        
        uint16_t attribute;
        if(fwrite(normal, sizeof(float), 3, pFile) != 3 ||
           fwrite(positions, sizeof(float), 9, pFile) != 9 ||
           fwrite(&attribute, sizeof(uint16_t), 1, pFile) != 1)
        {
            fclose(pFile);
            mesh->GetIndexBuffer()->Unmap();
            vba.Unmap();
            RA_LOG_ERROR_ASSERT("Failed to write a face %i", tri_index);
            return 0;
        }
    }
    
    mesh->GetIndexBuffer()->Unmap();
    vba.Unmap();
    
    fclose(pFile);
    return 1;
}


/*
int STLExporter::ExportToBinarySTL(Ossa::ES2Plate* mesh, const std::string& path)
{
    FILE* pFile;
    pFile = fopen(path.data(), "wb");
    if (!pFile)
    {
        RA_LOG_ERROR_ASSERT("Failed to open %s", path.data());
        return 0;
    }
    
    uint8_t header[80];
    if(fwrite(header, sizeof(uint8_t), 80, pFile) != 80)
    {
        fclose(pFile);
        RA_LOG_ERROR_ASSERT("Failed to write header");
        return 0;
    }
    
    uint32_t nTriangles = mesh->GetNumTriangles();
    if (fwrite(&nTriangles, sizeof(uint32_t), 1, pFile) != 1)
    {
        fclose(pFile);
        RA_LOG_ERROR_ASSERT("Failed to write number of triangles");
        return 0;
    }
    
    tyro::VertexBufferAccessor vba(mesh->GetVisualEffect()->GetVertexFormat(), mesh->GetVertexBuffer().get());
    vba.MapRead();
    unsigned int* indicies = (unsigned int*) mesh->GetIndexBuffer()->MapRead();
    
    //Calculate sking matrix pallete
    Wm5::HMatrix* matrix_palete = new Wm5::HMatrix[mesh->GetSkeleton()->m_joint_count];
    for (int i = 0; i < mesh->GetSkeleton()->m_joint_count; ++i)
    {
        if (mesh->GetCurrentPose())
        {
            matrix_palete[i] = (mesh->GetCurrentPose()->m_global_poses[i] * mesh->GetSkeleton()->m_joints[i].m_inv_bind_pose);
        }
        else
        {
            matrix_palete[i] = Wm5::HMatrix::IDENTITY;
        }
    }
    
    for (int tri_index = 0; tri_index < nTriangles; tri_index++)
    {
        Wm5::Vector3f positions[3];
        for (int i = 0; i < 3; ++i)
        {
            int index = indicies[3*tri_index + i];

            int joint1 = (int)(vba.JointIndex1<float>(index) + 0.5);
            int joint2 = (int)(vba.JointIndex2<float>(index) + 0.5);
            
            float weight1 = vba.BlendWeight1<float>(index);
            float weight2 = vba.BlendWeight2<float>(index);
            
            Wm5::APoint pos = vba.Position<Wm5::Float3>(index);
            Wm5::APoint t_pos = mesh->WorldTransform * (weight1 * (matrix_palete[joint1] * pos) + weight2 *  (matrix_palete[joint2] * pos));
            positions[i] = t_pos.operator Wm5::Vector3<float> &();
        }
        
        Wm5::Vector3f normal = (positions[2] - positions[1]).Cross(positions[3] - positions[1]);
        normal.Normalize();
        
        uint16_t attribute;
        if(fwrite(normal, sizeof(float), 3, pFile) != 3 ||
           fwrite(positions, sizeof(float), 9, pFile) != 9 ||
           fwrite(&attribute, sizeof(uint16_t), 1, pFile) != 1)
        {
            fclose(pFile);
            mesh->GetIndexBuffer()->Unmap();
            vba.Unmap();
            RA_LOG_ERROR_ASSERT("Failed to write a face %i", tri_index);
            return 0;
        }
    }
    
    mesh->GetIndexBuffer()->Unmap();
    vba.Unmap();
    
    fclose(pFile);
    return 1;
}

int STLExporter::ExportToBinarySTL(Ossa::ScrewNode* node, const std::string& path)
{
    FILE* pFile;
    pFile = fopen(path.data(), "wb");
    if (!pFile)
    {
        RA_LOG_ERROR_ASSERT("Failed to open %s", path.data());
        return 0;
    }
    
    uint8_t header[80];
    if(fwrite(header, sizeof(uint8_t), 80, pFile) != 80)
    {
        fclose(pFile);
        RA_LOG_ERROR_ASSERT("Failed to write header");
        return 0;
    }
    
    ScrewNode* s_node = node;
    assert(s_node);
    
    tyro::ES2CylinderSPtr meshes[2] = {s_node->GetHeadShaftNode()->GetHead(), s_node->GetHeadShaftNode()->GetShaft()};

    uint32_t nTriangles = meshes[0]->GetNumTriangles() + meshes[1]->GetNumTriangles();
    if (fwrite(&nTriangles, sizeof(uint32_t), 1, pFile) != 1)
    {
        fclose(pFile);
        RA_LOG_ERROR_ASSERT("Failed to write number of triangles");
        return 0;
    }
    
    for (int m_index = 0; m_index < 2; ++m_index)
    {
        tyro::ES2CylinderSPtr head = meshes[m_index];
        tyro::VertexBufferAccessor vba(head->GetVisualEffect()->GetVertexFormat(), head->GetVertexBuffer().get());
        vba.MapRead();
        unsigned int* indicies = (unsigned int*) head->GetIndexBuffer()->MapRead();
        
        for (int tri_index = 0; tri_index < head->GetNumTriangles(); tri_index++)
        {
            Wm5::Vector3f positions[3];
            for (int i = 0; i < 3; ++i)
            {
                int index = indicies[3*tri_index + i];
                
                Wm5::APoint pos = vba.Position<Wm5::Float3>(index);
                Wm5::APoint t_pos = head->WorldTransform * pos;
                positions[i] = t_pos.operator Wm5::Vector3<float> &();
            }
            
            Wm5::Vector3f normal = (positions[2] - positions[1]).Cross(positions[3] - positions[1]);
            normal.Normalize();
            
            uint16_t attribute;
            if(fwrite(normal, sizeof(float), 3, pFile) != 3 ||
               fwrite(positions, sizeof(float), 9, pFile) != 9 ||
               fwrite(&attribute, sizeof(uint16_t), 1, pFile) != 1)
            {
                fclose(pFile);
                head->GetIndexBuffer()->Unmap();
                vba.Unmap();
                RA_LOG_ERROR_ASSERT("Failed to write a face %i", tri_index);
                return 0;
            }
        }
    
        head->GetIndexBuffer()->Unmap();
        vba.Unmap();
    }
    
    fclose(pFile);
    return 1;

}
*/

//int STLExporter::ExportToASCIISTL(const tyro::ES2TriMesh& plate, const std::string& path)
//{
//    return 0;
//}
}