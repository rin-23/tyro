//
//  RANormalMapProxy.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-08-07.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2TriMesh.h"

namespace tyro
{
    class NormalProxy : public ES2TriMesh
    {
    public:
        NormalProxy() {}
        
        virtual ~NormalProxy() {}
        
        static NormalProxySPtr Create(ES2TriMeshSPtr mesh);
        
        virtual void UpdateUniformsWithCamera(const tyro::Camera* camera) override;
        
    protected:
        
        void Init(ES2TriMeshSPtr mesh);
    };
}

