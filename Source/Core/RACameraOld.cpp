//
//  RACamera.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-09.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RACamera.h"
#include "Wm5Transform.h"
#include "Wm5Quaternion.h"
#include "RAMath.h"

using Wm5::Transform;
using Wm5::HQuaternion;
using Wm5::Vector3f;
using Wm5::Vector4i;
using Wm5::HMatrix;
using Wm5::AVector;
using Wm5::APoint;

namespace tyro
{
    Camera::Camera()
    {
        mViewMatrix.MakeIdentity();
        mPosition = Vector3f(0,0,0);
        mOrientation = HQuaternion::IDENTITY;
//        mGyroscopeRotationEnabled = false;
        UpdateViewMatrix();
        
        mProjectionMatrix.MakeIdentity();
        mViewport = Vector4i(0,0,0,0);
        mIsOrtho = true;
        mAspect = 1.0f;
        UpdateProjectionMatrix();
    }
    
    Camera::Camera(const Wm5::APoint& position, const Wm5::Vector4i& viewport, float aspect, bool isOrtho)
    {
        mViewMatrix.MakeIdentity();
        mPosition = position;
        mOrientation = HQuaternion::IDENTITY;
//        mGyroscopeRotationEnabled = false;
        UpdateViewMatrix();
        
        mProjectionMatrix.MakeIdentity();
        mViewport = viewport;
        mIsOrtho = isOrtho;
        mAspect = aspect;
        UpdateProjectionMatrix();
    }
    
    Camera::Camera(const Wm5::APoint& position, const Wm5::APoint& target, const Wm5::Vector4i& viewport, float aspect, bool isOrtho)
    {
        mViewMatrix.MakeIdentity();
        mPosition = position;
        mOrientation = HQuaternion::IDENTITY;
        LookAt(target);
//        mGyroscopeRotationEnabled = false;
        UpdateViewMatrix();
        
        mProjectionMatrix.MakeIdentity();
        mViewport = viewport;
        mIsOrtho = isOrtho;
        mAspect = aspect;
        UpdateProjectionMatrix();
    }

    Camera::~Camera()
    {
    }
    
    //Projection matrix
    void Camera::SetOrthoProjection(float left, float right, float bottom, float top, float near, float far)
    {
        float tx = -(right + left)/(right - left);
        float ty = -(top + bottom)/(top - bottom);
        float tz = -(far + near)/(far-near);
        float d1 = 2.0f/(right-left);
        float d2 = 2.0f/(top-bottom);
        float d3 = -2.0f/(far-near);
        mProjectionMatrix = HMatrix(d1,  0,  0, tx,
                                     0, d2,  0, ty,
                                     0,  0, d3, tz,
                                     0,  0,  0,  1);
    }
    
    void Camera::SetPerpProjection(float left, float right, float bottom, float top, float near, float far)
    {
        float d1 = 2*near/(right - left);
        float d2 = 2*near/(top - bottom);
        float A = (right + left)/(right - left);
        float B = (top + bottom)/(top - bottom);
        float C = -1*(far + near)/(far - near);
        float D = -1*(far * near)/(far - near);
        mProjectionMatrix = HMatrix(d1, 0, A, 0,
                                    0, d2, B, 0,
                                    0,  0, C, D,
                                    0, 0, -1, 0);
    }
    
    void Camera::SetPerpProjection(float fovyRadians, float aspect, float near, float far)
    {
        float cotan = 1.0f / tanf(fovyRadians / 2.0f);
        
        mProjectionMatrix = HMatrix(cotan / aspect, 0.0f, 0.0f, 0.0f,
                                    0.0f, cotan, 0.0f, 0.0f,
                                    0.0f, 0.0f, (far + near) / (near - far), -1.0f,
                                    0.0f, 0.0f, (2.0f * far * near) / (near - far), 0.0f);
        mProjectionMatrix = mProjectionMatrix.Transpose();
        
//        float fH = tanf(fovy / 360 * M_PI) * near;
//        float fW = fH * aspect;
//        Camera::SetPerpProjection(-fW, fW, -fH, fH, near, far);
    }
    
    void Camera::SetPosition(const Wm5::APoint& pos)
    {
        mPosition = pos;
        UpdateViewMatrix();
    }
    
    void Camera::SetViewport(const Wm5::Vector4i& viewport)
    {
        mViewport = viewport;
    }
    
    void Camera::SetAspect(float aspect)
    {
        mAspect = aspect;
        UpdateProjectionMatrix();
    }
    
    void Camera::SetOrientation(const Wm5::HQuaternion& orientation)
    {
        mOrientation = orientation;
        UpdateViewMatrix();
    }
    
    void Camera::SetDirection(Wm5::AVector direction)
    {
        if (direction == AVector::ZERO) {
            return;
        }
        direction.Normalize();
        
        AVector curDir = mOrientation.Rotate(AVector::UNIT_NEGATIVE_Z);
        curDir.Normalize();

        HQuaternion rotQuat;
        rotQuat.Align(curDir, direction);
        mOrientation =  rotQuat * mOrientation;
        
        UpdateViewMatrix();
    }
    
    void Camera::LookAt(const Wm5::APoint& target)
    {
        SetDirection(target - mPosition);
    }

    void Camera::Rotate(const Wm5::HQuaternion& quat)
    {
        mOrientation = quat * mOrientation;
        UpdateViewMatrix();
    }
  
    void Camera::UpdateViewMatrix()
    {
        AVector mRVector = GetRightVector();
        AVector mUVector = GetUpVector();
        AVector mDVector = -1*GetDirectionVector();
        AVector::Orthonormalize(mDVector, mUVector, mRVector);
        
        mViewMatrix[0][0] = mRVector[0];
        mViewMatrix[0][1] = mRVector[1];
        mViewMatrix[0][2] = mRVector[2];
        mViewMatrix[0][3] = -mPosition.Dot(mRVector);
        mViewMatrix[1][0] = mUVector[0];
        mViewMatrix[1][1] = mUVector[1];
        mViewMatrix[1][2] = mUVector[2];
        mViewMatrix[1][3] = -mPosition.Dot(mUVector);
        mViewMatrix[2][0] = mDVector[0];
        mViewMatrix[2][1] = mDVector[1];
        mViewMatrix[2][2] = mDVector[2];
        mViewMatrix[2][3] = -mPosition.Dot(mDVector);
        mViewMatrix[3][0] = 0.0f;
        mViewMatrix[3][1] = 0.0f;
        mViewMatrix[3][2] = 0.0f;
        mViewMatrix[3][3] = 1.0f;
    }
    
    void Camera::UpdateProjectionMatrix()
    {
    }
    
    Wm5::AVector Camera::GetUpVector() const
    {
        return mOrientation.Rotate(AVector::UNIT_Y).GetNormalized();
    }
    
    Wm5::AVector Camera::GetRightVector() const
    {
        return mOrientation.Rotate(AVector::UNIT_X).GetNormalized();
    }
    
    Wm5::AVector Camera::GetDirectionVector() const
    {
        return mOrientation.Rotate(AVector::UNIT_NEGATIVE_Z).GetNormalized();
    }
    
//    void Camera::SetGyroscopeRotationEnabled(bool enable)
//    {
//        mGyroscopeRotationEnabled = enable;
//        //TODO uncomment
////        if (enable) {
////            RAGyroscopeManager::getInstance().enableGyro();
////        } else {
////            RAGyroscopeManager::getInstance().disableGyro();
////        }
//    }
    
    Wm5::Vector3f Camera::WorldToScreen(const Wm5::Vector3f& world) const
    {
        Wm5::Vector3f win;
        int result = Math::project(world, GetViewport(), GetViewProjectionMatrix(), win);
        assert(result);
        return win;
    }
    
    Wm5::Vector3f Camera::CameraToScreen(const Wm5::Vector3f& world) const
    {
        Wm5::Vector3f win;
        int result =  Math::project(world, GetViewport(), GetProjectionMatrix(), win);
        assert(result);
        return win;
    }
    
    Wm5::Vector3f Camera::ScreenToWorld(const Wm5::Vector3f& win) const
    {
        Wm5::Vector3f world;
        int result = Math::unProject(win, GetViewport(), GetViewProjectionMatrix(), world);
        assert(result);
        return world;
    }
    
    Wm5::Vector3f Camera::ScreenToCamera(const Wm5::Vector3f& win) const
    {
        Wm5::Vector3f world;
        int result = Math::unProject(win, GetViewport(), GetProjectionMatrix(), world);
        assert(result);
        return world;
    }
    
    Wm5::Vector3f Camera::ConvertCameraToWorldOrientation(const Wm5::Vector3f vec) const
    {
        return GetViewMatrix().Inverse() * AVector(vec);
    }
}
