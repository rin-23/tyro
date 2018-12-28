//
//  RAES2StandardMesh.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-09.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2TriMesh.h"
#include "RAES2DefaultPolyline.h"
#include "RAEnginePrerequisites.h"

namespace tyro 
{
class ShaderBox : public ES2TriMesh
{
    public:
        ShaderBox() {}
        virtual ~ShaderBox() {}
        
        static ShaderBoxSPtr Create();

        virtual void UpdateUniformsWithCamera(const Camera* camera) override;

    public: //Serialization
    
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<ES2DefaultTriMesh>(this));
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<ES2DefaultTriMesh>(this));
        }

    protected:
        void Init();

};
}