//
//  RAES2Polypoint.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-02-25.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2Renderable.h"

namespace RAEngine
{
    class ES2Polypoint : public ES2Renderable
    {
    public:
        ES2Polypoint() {}
        virtual ~ES2Polypoint() {}

//        static ES2PolypointSPtr Create();
        
        int GetNumberOfPoints();
        
        virtual void UpdateUniformsWithCamera(const RAEngine::Camera* camera) override = 0;
   
    public: //Serialization
        
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<RAEngine::ES2Renderable>(this));
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<RAEngine::ES2Renderable>(this));
        }
    protected:
        void Init();
    };
}

CEREAL_REGISTER_TYPE(RAEngine::ES2Polypoint);