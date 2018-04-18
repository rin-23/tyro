//
//  RAArcBallCamera.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-20.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAiOSCamera.h"
#include "RAMath.h"
#include "RALogManager.h"
#include "Wm5HQuaternion.h"

using namespace Wm5;

namespace tyro
{
    //Orbiting camera araound worldCenter at radius
    iOSCamera::iOSCamera(const Wm5::APoint& worldCenter,
                         float radius,
                         float aspect,
                         float scale,
                         const Wm5::Vector4i& viewport,
                         bool isOrtho)
    :
    tyro::Camera(APoint(0,0,0), worldCenter, viewport, aspect, isOrtho)
     {
        mInitialPosition = worldCenter;
        mInitialPosition.Z() += radius;
        SetPosition(mInitialPosition);
        LookAt(worldCenter);

        mWorldCenter = worldCenter;
        mRadius = radius;
        mScale = scale;
        mInitialScale = scale;
        mTranslationMatrix.MakeIdentity();
        mAccumulatedTranslation.MakeIdentity();
        
        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }
    
    iOSCamera::~iOSCamera() {}
    
    void iOSCamera::SetWorldCenter(const Wm5::APoint& worldCenter)
    {
        mWorldCenter = worldCenter;
        LookAt(mWorldCenter);
    }
    
    void iOSCamera::SetRadius(float radius)
    {
        mRadius = radius;
        UpdateProjectionMatrix();
    }
    
    float iOSCamera::GetRadius() const
    {
        return mRadius;
    }

#pragma mark - Gesture handlers
	
    void iOSCamera::HandleOneFingerPanGesture(int state, Vector2i glTouch)
    {
        //GestureState state = GestureStateForRecognizer(sender);
        //Vector2i glTouch = TouchPointForGesture(sender);

        //if (state == GestureState::Began)
        if (state == 0)
        {
            mLastLoc = glTouch;
        }
        //else if (state == GestureState::Changed || state == GestureState::Ended)
        else if (state == 1 || state == 2)
        {
            Vector2i diff = glTouch - mLastLoc;
            float rotX =  Math::DegToRad(-1*diff[1]/3.0f); //because positive angle is clockwise
            float rotY =  Math::DegToRad(-1*diff[0]/3.0f);

            AVector xAxis = GetOrientation().Rotate(AVector::UNIT_X);
            AVector yAxis = GetOrientation().Rotate(AVector::UNIT_Y);

            HQuaternion xAxisRot(xAxis, rotX);
            HQuaternion yAxisRot(yAxis, rotY);
            AVector pivotToOrigin = APoint::ORIGIN - mWorldCenter;
            HMatrix toOrigin(pivotToOrigin);
            HMatrix fromOrigin(-pivotToOrigin);

            Vector3f positiveX(1, 0, 0);
            Vector3f transVectorNormalized(diff.X(), diff.Y(), 0);
            transVectorNormalized.Normalize();
            float angle = acosf(positiveX.Dot(transVectorNormalized));
            
            if (angle < Math::DegToRad(45.0) || angle > Math::DegToRad(135.0) )
            {
                HMatrix rot;
                yAxisRot.ToRotationMatrix(rot);
                APoint newPosition = fromOrigin * rot * toOrigin * GetPosition();
                Camera::SetPosition(newPosition);
                Camera::LookAt(mWorldCenter);
            }
            else
            {
                HMatrix rot;
                xAxisRot.ToRotationMatrix(rot);
                APoint newPosition = fromOrigin * rot * toOrigin * GetPosition();
                Camera::SetPosition(newPosition);
                Camera::LookAt(mWorldCenter);
            }
            
            mLastLoc = glTouch;
        } 
    }
    
    void iOSCamera::HandleTwoFingerPanGesture(int state, Wm5::Vector2i translation)
    {
        //if (sender.numberOfTouches != 2) {
        //    mAccumulatedTranslation = mTranslationMatrix;
        //    return;
        //}
        
        //GestureState state = GestureStateForRecognizer(sender);
        //Vector2i translation = TouchPointForGesture(sender);
        
        Vector3f screen(translation.X(), translation.Y(), 0.0f);
        Vector3f world = Camera::ScreenToCamera(screen);
        world.Z() = 0;
        
        if (state == 0)
        {
            mStartPoint = world;
        }
        else if (state == 1 || state == 2)
        {
            AVector diff(world - mStartPoint);
            HMatrix temp(diff);
            mTranslationMatrix = temp * mAccumulatedTranslation;
            UpdateViewMatrix();
            
            if (state == 2)
            {
                mAccumulatedTranslation = mTranslationMatrix;
            }
        }
    }

    /*
    void iOSCamera::HandleRotationGesture(UIRotationGestureRecognizer* sender)
    {
        if (sender.numberOfTouches != 2) {
            return;
        }
        
        GestureState state = GestureStateForRecognizer(sender);
        float rotation = sender.rotation;
        
        if (state == GestureState::Began)
        {
            mLastRot = rotation;
        }
        else if (state == GestureState::Changed || state == GestureState::Ended)
        {
            float rotZ = (rotation - mLastRot);
            AVector zAxis = GetOrientation().Rotate(AVector::UNIT_NEGATIVE_Z);
            HQuaternion zAxisRot(zAxis, -1*rotZ);
            Camera::Rotate(zAxisRot);
            mLastRot = rotation;
        }
    }
    */

    void iOSCamera::HandlePinchGesture(int state, Wm5::Vector2i glTouch, double offset)
    {
        //if (sender.numberOfTouches != 2) {
        //    mAccumulatedTranslation = mTranslationMatrix;
        //    return;
        //}
        
        //GestureState state = GestureStateForRecognizer(sender);
        //CGPoint touchPoint1 = ScaleTouchPoint([sender locationOfTouch:0 inView:sender.view],
        //                                      sender.view);
        //CGPoint touchPoint2 = ScaleTouchPoint([sender locationOfTouch:1 inView:sender.view],
        //                                      sender.view);
        
        //Vector2f touch1(touchPoint1.x, touchPoint1.y);
        //Vector2f touch2(touchPoint2.x, touchPoint2.y);
        
        //float ascale =  offset; //sender.scale;
        
        //if (state == 0)
        //{
            //mInitialTouch = Vector2f(glTouch[0], glTouch[1]);//(touch1 + touch1)/2.0f;
            //Vector3f screen(mInitialTouch.X(), mInitialTouch.Y(), 0.0f);
            //mCentroid = Camera::ScreenToCamera(screen);
            //mCentroid.Z() = 0;
            
            //mCurScale = mScale;
        //}
        //else if (state == 1 || state == 2)
        //{
            if (offset > 0)
                mScale *= 1.07;  //mCurScale * ascale;
            else 
                mScale *= 0.93;

            UpdateProjectionMatrix();
            
            //Vector3f screen(mInitialTouch.X(), mInitialTouch.Y(), 0.0f);
            //Vector3f newCentroid = Camera::ScreenToCamera(screen);
            //newCentroid.Z() = 0;
            
            //AVector translation = AVector(newCentroid - mCentroid);
            //HMatrix temp(translation);
            //mTranslationMatrix = temp * mAccumulatedTranslation;
            //UpdateViewMatrix();
            
            //if (state == 2)
            //{
            //    mAccumulatedTranslation = mTranslationMatrix;
            //}
        //}
    }
	
    void iOSCamera::UpdateProjectionMatrix()
    {
        if (mIsOrtho)
        {
            float scale = 1/mScale;
            float left = 0 - mRadius * mAspect * scale;
            float right = 0 + mRadius * mAspect * scale;
            float bottom = 0 - mRadius * scale;
            float top = 0 + mRadius * scale;
            
            Camera::SetOrthoProjection(left, right, bottom, top, 1.0f, 1.0f + 100.0f * mRadius);
        } else {
            //perpcl
        }
    }
    
    void iOSCamera::UpdateViewMatrix()
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
        
        mViewMatrix = mTranslationMatrix * mViewMatrix;
    }
    
    void iOSCamera::Reset()
    {
        mPosition = mInitialPosition;
        mOrientation = HQuaternion::IDENTITY;
        mScale = mInitialScale;
        LookAt(mWorldCenter);
        
        mTranslationMatrix.MakeIdentity();
        mAccumulatedTranslation.MakeIdentity();
        
        UpdateViewMatrix();
        UpdateProjectionMatrix();
    }
    
	/*
    GestureState iOSCamera::GestureStateForRecognizer(UIGestureRecognizer* sender)
    {
        switch (sender.state)
        {
            case UIGestureRecognizerStatePossible: return GestureState::Possible;
            case UIGestureRecognizerStateBegan: return  GestureState::Began;
            case UIGestureRecognizerStateChanged: return GestureState::Changed;
            case UIGestureRecognizerStateEnded: return GestureState::Ended;
            case UIGestureRecognizerStateCancelled: return GestureState::Canceled;
            case UIGestureRecognizerStateFailed: return GestureState::Failed;
            default: return GestureState::Failed;
        }
    }
    
    CGPoint iOSCamera::ScaleTouchPoint(CGPoint touchPoint, UIView* view)
    {
        CGFloat scale = view.contentScaleFactor;
        touchPoint.x = floorf(touchPoint.x * scale);
        touchPoint.y = floorf(touchPoint.y * scale);
        touchPoint.y = floorf(view.frame.size.height*scale - touchPoint.y);
        return touchPoint;
    }
    
    Wm5::Vector2i iOSCamera::TouchPointForGesture(UIGestureRecognizer* sender)
    {
        CGPoint location = [sender locationInView:sender.view];
        location = iOSCamera::ScaleTouchPoint(location, sender.view);
        return Vector2i(location.x, location.y);
    }

    
//    int iOSCamera::WorldToScreen(UIGestureRecognizer* sender, float depth, Wm5::Vector3f& win)
//    {
//
//        return 0;
//    }
//    
//    int iOSCamera::CameraToScreen(UIGestureRecognizer* sender, float depth, Wm5::Vector3f& win)
//    {
//        return 0;
//    }
    
    Wm5::Vector3f iOSCamera::ScreenGestureToWorld(UIGestureRecognizer* sender, float depth) const
    {
        Vector2i touchPoint = TouchPointForGesture(sender);
        Vector3f win(touchPoint.X(), touchPoint.Y(), depth);
        return Camera::ScreenToWorld(win);
    }
    
    Wm5::Vector3f iOSCamera::ScreenGestureToCamera(UIGestureRecognizer* sender, float depth) const
    {
        Vector2i touchPoint = TouchPointForGesture(sender);
        Vector3f win(touchPoint.X(), touchPoint.Y(), depth);
        return Camera::ScreenToCamera(win);
    }
	*/
}