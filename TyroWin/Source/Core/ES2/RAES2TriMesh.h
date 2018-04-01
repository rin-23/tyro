//
//  RAES2TriMesh.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-14.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2Renderable.h"

namespace tyro
{
    class ES2TriMesh : public ES2Renderable
    {
    public:
        ES2TriMesh() {}
        
        virtual ~ES2TriMesh() {}
        
//        static ES2TriMeshSPtr Create();

        int GetNumTriangles() const;
        
        void SetLocalBoundBoxFromVertexBuffer();
        
        virtual void UpdateUniformsWithCamera(const tyro::Camera* camera) override = 0;
    
    public: //Serialization
        
        template<class Archive>
        void save(Archive & archive, std::uint32_t const version) const
        {
            archive(cereal::virtual_base_class<tyro::ES2Renderable>(this));
        }
        
        template<class Archive>
        void load(Archive & archive, std::uint32_t const version)
        {
            archive(cereal::virtual_base_class<tyro::ES2Renderable>(this));
        }
    protected:

        void Init();

    };
}

CEREAL_REGISTER_TYPE(tyro::ES2TriMesh);