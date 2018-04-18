//
//  RAES2AlphaState.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-04-07.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

namespace tyro
{
    class ES2AlphaState
    {
    public:
        ES2AlphaState()
        {
            Enabled = false;
        }
        
        bool Enabled;
    };
    
    class ES2CullState
    {
    public:
        ES2CullState()
        {
            Enabled = true;
        }
        
        bool Enabled;
    };
    
    class ES2DepthState
    {
    public:
        ES2DepthState()
        {
            Enabled = true;
            DepthMaskEnabled = true;
        }
        
        bool Enabled;
        bool DepthMaskEnabled;
    };

    class ES2PolygonOffset 
    {
    public:
        ES2PolygonOffset()
        :
        Enabled(false),
        Offset(0.0)
        {}

        ES2PolygonOffset(bool enabled, float offset)
        :
        Enabled(enabled),
        Offset(offset)
        {}

        bool Enabled;
        float Offset;
    };
}
