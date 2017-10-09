//
//  RAES2LineMesh.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-27.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2Renderable.h"

namespace RAEngine
{
    class ES2Polyline : public ES2Renderable
    {
    public:
        ES2Polyline() {}
        virtual ~ES2Polyline() {}
        
//        static ES2PolylineSPtr Create(bool isContigious = false);
        
        int GetNumberOfSegments() const;
        
        virtual void UpdateUniformsWithCamera(const RAEngine::Camera* camera) override = 0;

    public: //Serialization
        
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<RAEngine::ES2Renderable>(this));
            archive(mIsContigious);
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<RAEngine::ES2Renderable>(this));
            archive(mIsContigious);
        }

    protected:
        
        void Init(bool isContigious);

    private:
        
        bool mIsContigious;
    };
}

CEREAL_REGISTER_TYPE(RAEngine::ES2Polyline);


