//
//  RAArcBallCamera.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-20.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RACamera.h"
#include "Wm5Vector2.h"
#include "RAEnginePrerequisites.h"
#include <Eigen/Core>
//#include <cereal/cereal.hpp>

namespace tyro
{
    class iOSCamera : public Camera
    {
    public:
        iOSCamera(const Wm5::APoint& worldCenter,
                  float radius,
                  float aspect,
                  float scale,
                  const Wm5::Vector4i& viewport,
                  bool isOrtho);
        
		virtual ~iOSCamera();

        void SetWorldCenter(const Wm5::APoint& worldCenter);
        void SetRadius(float radius);
        float GetRadius() const;
        void Reset();

        void HandleOneFingerPanGesture(int state, Wm5::Vector2i glTouch);
        void HandleTwoFingerPanGesture(int state, Wm5::Vector2i glTouch);
        //void HandleRotationGesture(UIRotationGestureRecognizer* sender);
        void HandlePinchGesture(int state, Wm5::Vector2i glTouch, double offset);
        void Translate(const Wm5::Vector2i& offset);
        //Eigen::Matrix4f GetEigenViewMatrix();
        //Eigen::Matrix4f GetEigenProjMatrix();
        //Eigen::Vector4f GetEigenViewport();
        //int WorldToScreen(UIGestureRecognizer* sender, float depth, Wm5::Vector3f& win);
        //int CameraToScreen(UIGestureRecognizer* sender, float depth, Wm5::Vector3f& win);
        //Wm5::Vector3f ScreenGestureToWorld(UIGestureRecognizer* sender, float depth) const;
        //Wm5::Vector3f ScreenGestureToCamera(UIGestureRecognizer* sender, float depth) const;
        
        //static GestureState GestureStateForRecognizer(UIGestureRecognizer* sender);
        //static Wm5::Vector2i TouchPointForGesture(UIGestureRecognizer* sender);

    protected:
        virtual void UpdateProjectionMatrix() override;
        virtual void UpdateViewMatrix() override;
        
        //static CGPoint ScaleTouchPoint(CGPoint touchPoint, UIView* view);
        
        Wm5::APoint mInitialPosition;
        Wm5::APoint mWorldCenter;
        float mInitialScale;
        float mRadius;

        //Rotation
        Wm5::Vector2i mLastLoc; //x,y axes rotaiton
        float mLastRot; //z axis rotation
        
        //Scale
        float mCurScale;
        float mScale;
        Wm5::APoint mCentroid;
        Wm5::Vector2f mInitialTouch;
        
        //Translation
        Wm5::Vector3f mStartPoint;
        Wm5::HMatrix mTranslationMatrix;
        Wm5::HMatrix mAccumulatedTranslation;
    };

    /*
    template<class Archive>
    void save(Archive & archive) const
    {
        archive(mInitialPosition);
        archive(mWorldCenter);

        archive(mInitialScale);
        archive(mRadius);
        
        archive(mLastLoc);
        archive(mLastRot);
        
        archive(mCurScale);
        archive(mScale);

        archive(mCentroid);
        archive(mInitialTouch);

        archive(mStartPoint);
        archive(mTranslationMatrix);
        archive(mAccumulatedTranslation);
    }
    
    template<class Archive>
    void load(Archive & archive)
    {
        archive(mInitialPosition);
        archive(mWorldCenter);

        archive(mInitialScale);
        archive(mRadius);
        
        archive(mLastLoc);
        archive(mLastRot);
        
        archive(mCurScale);
        archive(mScale);

        archive(mCentroid);
        archive(mInitialTouch);

        archive(mStartPoint);
        archive(mTranslationMatrix);
        archive(mAccumulatedTranslation);
    }
    */
}

