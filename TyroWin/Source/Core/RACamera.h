//
//  RACamera.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-09.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once 

#include "Wm5HMatrix.h"
#include "Wm5HQuaternion.h"
#include "Wm5Vector4.h"

namespace RAEngine
{
    class Camera
    {
    public:
        Camera();
        Camera(const Wm5::APoint& position, const Wm5::Vector4i& viewport, float aspect, bool isOrtho);
        Camera(const Wm5::APoint& position, const Wm5::APoint& target, const Wm5::Vector4i& viewport, float aspect, bool isOrtho);
        virtual ~Camera();
        
        //Projection matrix
        void SetOrthoProjection(float left, float right, float bottom, float top, float nearZ, float farZ);
        void SetPerpProjection(float left, float right, float bottom, float top, float near, float far);
        void SetPerpProjection(float fovy, float aspect, float near, float far);

        void SetViewport(const Wm5::Vector4i& viewport);
        void SetAspect(float aspect);
        void SetPosition(const Wm5::APoint& pos);
        void SetOrientation(const Wm5::HQuaternion& orientation);
        void SetDirection(Wm5::AVector direction);
        void LookAt(const Wm5::APoint& target);
        void Rotate(const Wm5::HQuaternion& quat);

        inline const Wm5::Vector4i& GetViewport() const;
        inline float GetAspect() const;
        inline const Wm5::HMatrix& GetViewMatrix() const;
        inline const Wm5::HMatrix& GetProjectionMatrix() const;
        inline Wm5::HMatrix GetViewProjectionMatrix() const;
        inline const Wm5::APoint& GetPosition() const;
        inline const Wm5::HQuaternion& GetOrientation() const;
        Wm5::AVector GetUpVector() const;
        Wm5::AVector GetRightVector() const;
        Wm5::AVector GetDirectionVector() const;

//        void SetGyroscopeRotationEnabled(bool enable);
        
        Wm5::Vector3f WorldToScreen(const Wm5::Vector3f& world) const;
        Wm5::Vector3f CameraToScreen(const Wm5::Vector3f& world) const;
        Wm5::Vector3f ScreenToWorld(const Wm5::Vector3f& win) const;
        Wm5::Vector3f ScreenToCamera(const Wm5::Vector3f& win) const;
        
        Wm5::Vector3f ConvertCameraToWorldOrientation(const Wm5::Vector3f vec) const;
        

    protected:
        virtual void UpdateViewMatrix();
        virtual void UpdateProjectionMatrix();
        
        Wm5::HMatrix mProjectionMatrix;
        Wm5::HMatrix mViewMatrix;
        Wm5::APoint mPosition;
        Wm5::HQuaternion mOrientation;
        Wm5::Vector4i mViewport;
        float mAspect;
        bool mIsOrtho;
//        bool mGyroscopeRotationEnabled;

    };
    
    /****INLINE FUCNTIONS****/
    
    const Wm5::Vector4i& Camera::GetViewport() const
    {
        return mViewport;
    }
    
    float Camera::GetAspect() const
    {
        return mAspect;
    }
    
    const Wm5::HMatrix& Camera::GetViewMatrix() const
    {
        return mViewMatrix;
    }
    
    const Wm5::HMatrix& Camera::GetProjectionMatrix() const
    {
        return mProjectionMatrix;
    }
    
    Wm5::HMatrix Camera::GetViewProjectionMatrix() const
    {
        return mProjectionMatrix * mViewMatrix;
    }

    const Wm5::APoint& Camera::GetPosition() const
    {
        return mPosition;
    }
    
    const Wm5::HQuaternion& Camera::GetOrientation() const
    {
        return mOrientation;
    }
    
}
