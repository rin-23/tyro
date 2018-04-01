//
//  RAES2DefaultPolyline.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-11.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2Polyline.h"
#include "Wm5Vector4.h"

namespace tyro
{
    class ES2DefaultPolyline : public ES2Polyline
    {
    public:
        
        ES2DefaultPolyline() {}
        
        virtual ~ES2DefaultPolyline() {}
        
        static ES2DefaultPolylineSPtr Create(bool isContigious);
        
        void SetColor(Wm5::Vector4f color);
        
        virtual void UpdateUniformsWithCamera(const Camera* camera) override;
        
    public: //Serialization
        
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<tyro::ES2Polyline>(this));
            archive(mColor);
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<tyro::ES2Polyline>(this));
            archive(mColor);
            InitSerialized();
        }
    
    protected:
        Wm5::Vector4f mColor;
        
        void Init(bool isContigious);
        
        void InitSerialized();
    };
}

CEREAL_REGISTER_TYPE(tyro::ES2DefaultPolyline);

