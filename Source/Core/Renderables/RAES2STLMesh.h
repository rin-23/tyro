//
//  RAES2STLMesh.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-01-17.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include <stdio.h>
#include "RAES2TriMesh.h"

namespace tyro
{
    class ES2STLMesh : public ES2TriMesh
    {
    public:
        ES2STLMesh();
        virtual ~ES2STLMesh();

        int createWithSTL(const std::string& filePath);
        virtual void UpdateUniformsWithCamera(const tyro::Camera* camera) override;
    private:
    
    };
}