//
//  Touchable.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-08-04.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAEnginePrerequisites.h"
#include "Wm5Vector2.h"
#include "Wm5Vector3.h"

#include <cereal/cereal.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>

namespace RAEngine
{
    struct TouchInfo
    {
        GestureState state;
        Wm5::Vector2i touch; //screen space
        float depth; //depth value under touch
        Wm5::Vector3f normal; //normal vector under the cursor
        float rotation;
        float scale;
        Wm5::Vector3f touch_world;
        Wm5::Vector2f nativeDeviceTouch;
    };
    
    class Touchable
    {
    public:
        virtual ~Touchable()
        {}
        
        enum InteractionFlag : int
        {
            NoneFlag = 0,
            OneFingerTapFlag = 1 << 0,
            OneFingerPanFlag = 1 << 1,
            TwoFingerPanFlag = 1 << 2,
            TwoFingerPinchFlag = 1 << 3,
            TwoFingerRotationFlag = 1 << 4,
        };

        InteractionFlag InteractionMask = NoneFlag;
        
        bool RespondsToOneFingerTap()
        {
            return (InteractionMask & OneFingerTapFlag) == OneFingerTapFlag;
        }
        
        bool RespondsToOneFingerPan()
        {
            return (InteractionMask & OneFingerPanFlag) == OneFingerPanFlag;
        }
        
        bool RespondsToTwoFingerPan()
        {
            return (InteractionMask & TwoFingerPanFlag) == TwoFingerPanFlag;
        }

        bool RespondsToTwoFingerPinch()
        {
            return (InteractionMask & TwoFingerPinchFlag) == TwoFingerPinchFlag;
        }
        
        bool RespondsToTwoFingerRotation()
        {
            return (InteractionMask & TwoFingerRotationFlag) == TwoFingerRotationFlag;
        }
        
        void TurnOnFlag(InteractionFlag flag)
        {
            InteractionMask = (InteractionFlag)(InteractionMask & ~flag);
        }
        
        void TurnOffFlag(InteractionFlag flag)
        {
            InteractionMask = (InteractionFlag)(InteractionMask ^ flag);
        }
        
        virtual bool NeedsDepthInfo()
        {
            return false;
        }
        
        virtual bool NeedsDepthNormalInfo()
        {
            return false;
        }
        
        virtual void HandleOneFingerTap(RAEngine::HitProxy* proxy, RAEngine::TouchInfo& tInfo, const RAEngine::Camera* camera)
        {}
        
        virtual void HandleOneFingerPan(RAEngine::HitProxy* proxy, RAEngine::TouchInfo& tInfo, const RAEngine::Camera* camera)
        {}
        
        virtual void HandleTwoFingerPan(RAEngine::HitProxy* proxy, RAEngine::TouchInfo& tInfo, const RAEngine::Camera* camera)
        {}
        
        virtual void HandleTwoFingerPinch(RAEngine::HitProxy* proxy, RAEngine::TouchInfo& tInfo, const RAEngine::Camera* camera)
        {}
        
        virtual void HandleTwoFingerRotation(RAEngine::HitProxy* proxy, RAEngine::TouchInfo& tInfo, const RAEngine::Camera* camera)
        {}

//        virtual bool ContainsHitProxy(RAEngine::HitProxy* proxy)
//        {
//            return false;
//        }
        
    public://Serialization
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(InteractionMask);
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(InteractionMask);
        }
        
    private:
        
    };
}

CEREAL_REGISTER_TYPE(RAEngine::Touchable);

