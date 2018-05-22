//
//  RAES2CoreVisualEffects.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-10.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAEnginePrerequisites.h"

namespace tyro
{
    class ES2CoreVisualEffects
    {
    public:
        static ES2VisualEffectSPtr VideoTexture(); 

        static ES2VisualEffectSPtr PBR();
        
        static ES2VisualEffectSPtr PBR2();
        
        static ES2VisualEffectSPtr ColorPicking();
        
        static ES2VisualEffectSPtr GourandDirectional();
        
        static ES2VisualEffectSPtr GourandDirectionalWithVColor();

        static ES2VisualEffectSPtr WireframeColor();

        static ES2VisualEffectSPtr NormalBuffer();
        
        static ES2VisualEffectSPtr DepthBuffer();
        
        //one of the uniforms is point size. Default value is 1.0f;
        static ES2VisualEffectSPtr NoLightUniformColor();
        
        static ES2VisualEffectSPtr BitmapText();
    };
}

