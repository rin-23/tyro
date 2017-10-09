//
//  OssaSceneNode.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-02-26.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once 

#include <vector>
#include "RASpatial.h"
#include "RAEnginePrerequisites.h"
#include <cereal/types/vector.hpp>

namespace RAEngine
{
    class Node : public Spatial
    {
    public:
        using NodeID = int;
        
    public:
        Node() {}
        virtual ~Node() {}
        
        static NodeSPtr Create();
        
        inline int GetNumChildren() const;
        
        NodeID AddChild(SpatialSPtr child);
        
        NodeID RemoveChild(SpatialSPtr child);
        
        NodeID RemoveChild(ObjectID childID);
        
        void RemoveChildAt(NodeID i);
        
        void SetChild (NodeID i, SpatialSPtr child);
        
        SpatialSPtr GetChildAt(NodeID index) const;
        
        virtual void GetVisibleSet (RAEngine::VisibleSet* visibleSet) override;
        
        virtual void GetHitProxies(RAEngine::VisibleSet* visibleSet) override;
        
    public://Serialization
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<Spatial>(this));
            archive(mChildren);
        }
    
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<Spatial>(this));
            archive(mChildren);
        }
        
    protected:
        virtual void UpdateBounds() override;
        
        virtual void UpdateTransformations() override;

        void Init();
        
    private:
        std::vector<SpatialSPtr> mChildren;
    };
    
    /****INLINE METHODS****/
    
    inline int Node::GetNumChildren() const
    {
        return (int)mChildren.size();
    }    
}

CEREAL_REGISTER_TYPE(RAEngine::Node);
