//
//  RAES2BBox.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-03-06.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2DefaultPolyline.h"

namespace RAEngine
{
    class ES2BBox : public ES2DefaultPolyline
    {
    public:
        ES2BBox() {}
        
        virtual ~ES2BBox() {}
        
        static ES2BBoxSPtr Create();
        
        void SetupWithBBox(const AxisAlignedBBox& bbox);

    public: //Serialization
        
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<ES2DefaultPolyline>(this));
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<ES2DefaultPolyline>(this));
        }

    protected:
        void Init();

    };
}

CEREAL_REGISTER_TYPE(RAEngine::ES2BBox);