//
//  RASkeleton.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-07-30.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once 

#include "Wm5HMatrix.h"
#include "RAEnginePrerequisites.h"

namespace RAEngine
{
    struct Joint
    {
        Wm5::HMatrix m_inv_bind_pose;
        std::string m_name;
        uint8 m_parent_joint; //parent or UCHAR_MAX if root
    };
    
    struct Skeleton
    {
        Skeleton()
        :
        m_joints(nullptr),
        m_joint_count(0)
        {}
        
        Skeleton(Joint* joints, int joint_count)
        :
        m_joints(joints),
        m_joint_count(joint_count)
        {}
        
        ~Skeleton()
        {
            delete [] m_joints;
        }
        
        Joint* m_joints; //array of joints
        int m_joint_count; // number of joints

    };
    
    struct SkeletonPose
    {   
        ~SkeletonPose()
        {
            m_skeleton = nullptr;
            delete [] m_local_poses;
            delete [] m_global_poses;
        }
        
        Skeleton* m_skeleton; //weak pntr
        Wm5::HMatrix* m_local_poses;
        Wm5::HMatrix* m_global_poses;
    };
}

