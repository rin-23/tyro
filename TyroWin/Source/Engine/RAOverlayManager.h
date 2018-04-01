//
//  RAOverlayManager.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-05-04.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAEnginePrerequisites.h"
#include "RAES2TextOverlay.h"

namespace tyro
{
    class OverlayManager
    {
    public:
        static OverlayManager* GetSingleton();
        
        ~OverlayManager() {}
        
        tyro::ObjectID AddOverlay(ES2TextOverlaySPtr overlay);
        
        ES2TextOverlaySPtr GetOverlay(ObjectID objID);
        
        void RemoveOverlay(ObjectID objID);
        
        std::map<ObjectID, ES2TextOverlaySPtr> GetOverlays();
        
        void GetVisibleSet(VisibleSet* visibleSet);
        
        const Wm5::Vector4i& GetViewPort() const;

        void SetViewPort(const Wm5::Vector4i& viewport);
        
        void Hide(bool hide) {mHide = hide;};
        
    private:
        OverlayManager()
        {
            mHide = false;
        }
        std::map<ObjectID, ES2TextOverlaySPtr> mOverlays;
        static OverlayManager* mInstance;
        Wm5::Vector4i mViewport;
        bool mHide;
    };
}
