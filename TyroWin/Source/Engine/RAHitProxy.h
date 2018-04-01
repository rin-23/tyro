//
//  RAHitProxy.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-08-04.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2TriMesh.h"

namespace tyro
{   
    //Abstract class thats meant to be subclassed
    class HitProxy : public ES2TriMesh
    {
    public:
        enum class HitProxyPriority
        {
            TRANSPARENT, // transparent bone
            REGULAR, //bone, plates, screws
            UIELEMENT // control points for screws and plates
        };

        HitProxy() {}
        
        virtual ~HitProxy() {}
        
//        static HitProxySPtr Create(TouchableSPtr touchable, ObjectID touchable_id, HitProxyPriority priority = HitProxyPriority::REGULAR);
        
        const Wm5::Vector3uc& GetColorID() const;
        
        static ObjectID GetIDFromColor(const Wm5::Vector3uc& color);
        
        TouchableSPtr GetTouchable() const;
        
        ObjectID GetTouchableID() const;
        
        void SetPriority(HitProxyPriority priority);
        
        HitProxyPriority GetPriority() const;

    protected:
        
        void Init(TouchableSPtr touchable, ObjectID touchable_id, HitProxyPriority priority = HitProxyPriority::REGULAR);
        
    private:
        
        Wm5::Vector3uc m_color_id;
        
        TouchableWPtr m_touchable; //weak
        
        ObjectID m_touchable_id;
        
        HitProxyPriority mPriority;

        static Wm5::Vector3uc CreateColorID(ObjectID objectID);
    };
    
    class RectHitProxy : public HitProxy
    {
    public:
        RectHitProxy() {}
        virtual ~RectHitProxy() {}

        static RectHitProxySPtr Create(TouchableSPtr touchable, ObjectID touchable_id);
        
        virtual void UpdateUniformsWithCamera(const tyro::Camera* camera) override;
    protected:
        void Init(TouchableSPtr touchable, ObjectID touchable_id);
    };
    
    class TriHitProxy : public HitProxy
    {
    public:
        TriHitProxy() {}
        virtual ~TriHitProxy(){}

        static TriHitProxySPtr Create(ES2TriMeshSPtr mesh, TouchableSPtr touchable, ObjectID touchable_id);

        virtual void UpdateUniformsWithCamera(const tyro::Camera* camera) override;
    protected:
        void Init(ES2TriMeshSPtr mesh, TouchableSPtr touchable, ObjectID touchable_id);
    };
}

