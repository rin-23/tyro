//
//  RASpatial.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-13.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "Wm5Transform.h"
// #include "Wm5Vector3.h"
#include "RAEnginePrerequisites.h"
#include "RAAxisAlignedBBox.h"
#include <memory>
#include <cereal/cereal.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/access.hpp>
#include <cereal/archives/binary.hpp>

namespace tyro
{
    using ObjectID = int;
    
    class Spatial : public std::enable_shared_from_this<Spatial>
    {
    public:
        Spatial() {}
        
        virtual ~Spatial() {}
        
        Spatial(const Spatial& obj) = delete; //prevent copying
        
        Spatial& operator=(const Spatial& obj) = delete; //prevent copying
        
        inline ObjectID GetID() const;
        
        inline SpatialSPtr GetParent();
        
        inline void SetParent (SpatialSPtr parent);
        
        void Update(bool initiator);

        virtual void GetVisibleSet(tyro::VisibleSet* visibleSet);

        virtual void GetHitProxies(tyro::VisibleSet* visibleSet);
    
        inline void SetName(std::string name);

        inline const std::string& GetName() const;
        
        // Wm5::Vector3f ConvertWorldToLocalPosition(const Wm5::Vector3f& pos) const;
        
        // Wm5::Vector3f ConvertWorldToLocalOrientation(const Wm5::Vector3f& vec) const;
        
        // Wm5::Vector3f ConvertLocalToWorldPosition(const Wm5::Vector3f& pos) const;
        
        // Wm5::Vector3f ConvertLocalToWorldOrientation(const Wm5::Vector3f& vec) const;
        
        inline void ShowBBox(bool show);
        
        inline bool GetShowBBox();
        
        bool operator == (const tyro::Spatial &Ref) const
        {
            return (this->GetID() == Ref.GetID());
        }
   
    public: //Serialization
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(LocalTransform, WorldTransform, WorldBoundBox, Visible, MergeWithParentBBox, mObjectID, mParent, mName, mShowBBox);
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(LocalTransform, WorldTransform, WorldBoundBox, Visible, MergeWithParentBBox, mObjectID, mParent, mName, mShowBBox);

            mNextID = std::max(mNextID, mObjectID);
        }
        
    protected:
        
        void Init();
        
        virtual void UpdateBounds () = 0;
        
        virtual void UpdateTransformations ();

        void PropagateBoundToRoot ();

    public:
        Wm5::Transform LocalTransform;
        
        Wm5::Transform WorldTransform;
        
        AxisAlignedBBox WorldBoundBox;
        
        bool Visible;
        
        bool MergeWithParentBBox;
        
    private:
        
        ObjectID mObjectID;
        
        static ObjectID mNextID;
        
        SpatialWPtr mParent;
        
        std::string mName;

        bool mShowBBox;
        
        ES2BBoxSPtr mBBox;
    };
    
    //Inline methods
    inline SpatialSPtr Spatial::GetParent()
    {
        return mParent.lock();
    }
    
    inline void Spatial::SetParent (SpatialSPtr parent)
    {
        mParent = parent;
    }

    inline ObjectID Spatial::GetID() const
    {
        return mObjectID;
    }
    
    inline void Spatial::ShowBBox(bool show)
    {
        mShowBBox = show;
    }
    
    inline bool Spatial::GetShowBBox()
    {
        return mShowBBox;
    }
    
    inline void Spatial::SetName(std::string name)
    {
        mName = name;
    }
    
    inline const std::string& Spatial::GetName() const
    {
        return mName;
    }
}

CEREAL_REGISTER_TYPE(tyro::Spatial);
