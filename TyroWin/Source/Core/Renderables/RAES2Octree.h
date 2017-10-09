//
//  RAES2Octree.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-27.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2DefaultPolyline.h"

namespace RAEngine
{
    class ES2Octree : public ES2DefaultPolyline
    {
    public:
        ES2Octree() {}
        
        virtual ~ES2Octree() {}
        
        static ES2OctreeSPtr Create();
        
        int CreateWithTriOctree(const TriOctree* triOctree);

    public: //Serialization
        
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<RAEngine::ES2DefaultPolyline>(this));
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<RAEngine::ES2DefaultPolyline>(this));
        }
    protected:
        void Init();
    };
}

CEREAL_REGISTER_TYPE(RAEngine::ES2Octree);
