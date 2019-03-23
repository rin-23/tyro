#pragma once

#include "RAEnginePrerequisites.h"
#include "TyroIGLMesh.h"
#include "bshapes.h"
#include "RAVisibleSet.h"
#include "iglmesh.h"

#define MESH_COLOR Eigen::Vector3d(255/255.0f, 223/255.0f, 196/255.0f)

namespace tyro
{  
struct MRenderData 
{
    IGLMeshSPtr mesh=nullptr;
    IGLMeshSPtr mesh2=nullptr;

    struct eye
    {
        IGLMeshTwoSPtr white;
        IGLMeshTwoSPtr brown;
        IGLMeshTwoSPtr black;
    };

    eye eye_left;
    eye eye_right;
    eye eye_left2;
    eye eye_right2;

    void CreateMesh(FaceModel& faceModel) 
    {   
        Eigen::MatrixXd V, N;
        Eigen::MatrixXi F;
        faceModel.getExpression(V, F, N);
        mesh = IGLMesh::Create(V, F, N, MESH_COLOR);
        mesh->Update(true);    

        auto eye_color_white = Eigen::Vector3d(0.8f, 0.8f, 0.8f);
        auto eye_color_brown = Eigen::Vector3d(92/255.0f, 64/255.0f, 51/255.0f);
        auto eye_color_black = Eigen::Vector3d(0.0f, 0.0f, 0.0f);
        
        eye_right.white = IGLMeshTwo::Create(faceModel.mRightEye.whiteV, faceModel.mRightEye.whiteF, faceModel.mRightEye.whiteN, eye_color_white);
        eye_right.white->Update(true);        
        eye_right.brown = IGLMeshTwo::Create(faceModel.mRightEye.brownV, faceModel.mRightEye.brownF, faceModel.mRightEye.brownN, eye_color_brown);
        eye_right.brown->Update(true);        
        eye_right.black = IGLMeshTwo::Create(faceModel.mRightEye.blackV, faceModel.mRightEye.blackF, faceModel.mRightEye.blackN, eye_color_black);
        eye_right.black->Update(true);    

        eye_left.white = IGLMeshTwo::Create(faceModel.mLeftEye.whiteV, faceModel.mLeftEye.whiteF, faceModel.mLeftEye.whiteN, eye_color_white);
        eye_left.white->Update(true);
        eye_left.brown = IGLMeshTwo::Create(faceModel.mLeftEye.brownV, faceModel.mLeftEye.brownF, faceModel.mLeftEye.brownN, eye_color_brown);
        eye_left.brown->Update(true);
        eye_left.black = IGLMeshTwo::Create(faceModel.mLeftEye.blackV, faceModel.mLeftEye.blackF, faceModel.mLeftEye.blackN, eye_color_black);
        eye_left.black->Update(true); 
    }

    void UpdateMesh(FaceModel& faceModel) 
    {
        Eigen::MatrixXd V, N;
        Eigen::MatrixXi F;
        faceModel.getExpression(V, F, N);
        mesh->UpdateData(V, F, N, MESH_COLOR);
        mesh->Update(true);
    }

    void CreateMesh2(FaceModel& faceModel) 
    {   
        Eigen::MatrixXd V, N;
        Eigen::MatrixXi F;
        faceModel.getExpression(V, F, N);
        mesh2 = IGLMesh::Create(V, F, N, MESH_COLOR);
        Wm5::Transform tr;
        tr.SetTranslate(Wm5::APoint(-1.5*mesh->WorldBoundBox.GetRadius(), 0, 0));
        mesh2->LocalTransform = tr * mesh2->LocalTransform;  
        mesh2->Update(true);

        auto eye_color_white = Eigen::Vector3d(0.8f, 0.8f, 0.8f);
        auto eye_color_brown = Eigen::Vector3d(92/255.0f, 64/255.0f, 51/255.0f);
        auto eye_color_black = Eigen::Vector3d(0.0f, 0.0f, 0.0f);
        
        eye_right2.white = IGLMeshTwo::Create(faceModel.mRightEye.whiteV, faceModel.mRightEye.whiteF, faceModel.mRightEye.whiteN, eye_color_white);
        eye_right2.white->LocalTransform = tr * eye_right2.white->LocalTransform;  
        eye_right2.white->Update(true);

        eye_right2.brown = IGLMeshTwo::Create(faceModel.mRightEye.brownV, faceModel.mRightEye.brownF, faceModel.mRightEye.brownN, eye_color_brown);
        eye_right2.brown->LocalTransform = tr * eye_right2.brown->LocalTransform;  
        eye_right2.brown->Update(true);
        
        eye_right2.black = IGLMeshTwo::Create(faceModel.mRightEye.blackV, faceModel.mRightEye.blackF, faceModel.mRightEye.blackN, eye_color_black);
        eye_right2.black->LocalTransform = tr * eye_right2.black->LocalTransform;  
        eye_right2.black->Update(true);   


        eye_left2.white = IGLMeshTwo::Create(faceModel.mLeftEye.whiteV, faceModel.mLeftEye.whiteF, faceModel.mLeftEye.whiteN, eye_color_white);
        eye_left2.white->LocalTransform = tr * eye_left2.white->LocalTransform;  
        eye_left2.white->Update(true);

        eye_left2.brown = IGLMeshTwo::Create(faceModel.mLeftEye.brownV, faceModel.mLeftEye.brownF, faceModel.mLeftEye.brownN, eye_color_brown);
        eye_left2.brown->LocalTransform = tr * eye_left2.brown->LocalTransform;  
        eye_left2.brown->Update(true);
        
        eye_left2.black = IGLMeshTwo::Create(faceModel.mLeftEye.blackV, faceModel.mLeftEye.blackF, faceModel.mLeftEye.blackN, eye_color_black);
        eye_left2.black->LocalTransform = tr * eye_left2.black->LocalTransform;  
        eye_left2.black->Update(true);   
        
    }

    void UpdateMesh2(FaceModel& faceModel) 
    {
        Eigen::MatrixXd V2, N2;
        Eigen::MatrixXi F2;
        faceModel.getExpression(V2, F2, N2);
        mesh2->UpdateData(V2, F2, N2, MESH_COLOR);
        mesh2->Update(true);
    }

    void AddToVisibleSet(VisibleSet& vis_set) 
    {
        vis_set.Insert(mesh.get());
        vis_set.Insert(eye_right.white.get());
        vis_set.Insert(eye_right.brown.get());
        vis_set.Insert(eye_right.black.get());
        vis_set.Insert(eye_left.white.get());
        vis_set.Insert(eye_left.brown.get());
        vis_set.Insert(eye_left.black.get());

        if (mesh2 !=nullptr) 
        {
            vis_set.Insert(mesh2.get());
            vis_set.Insert(eye_right2.white.get());
            vis_set.Insert(eye_right2.brown.get());
            vis_set.Insert(eye_right2.black.get());
            vis_set.Insert(eye_left2.white.get());
            vis_set.Insert(eye_left2.brown.get());
            vis_set.Insert(eye_left2.black.get());
        }
    }
};
}