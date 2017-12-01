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

namespace RAEngine
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

        //void HandleOneFingerPanGesture(UIPanGestureRecognizer* sender);
        //void HandleTwoFingerPanGesture(UIPanGestureRecognizer* sender);
        //void HandleRotationGesture(UIRotationGestureRecognizer* sender);
        //void HandlePinchGesture(UIPinchGestureRecognizer* sender);
        
//        int WorldToScreen(UIGestureRecognizer* sender, float depth, Wm5::Vector3f& win);
//        int CameraToScreen(UIGestureRecognizer* sender, float depth, Wm5::Vector3f& win);
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
}

