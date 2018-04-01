//
//  RAOverlayManager.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-05-04.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAOverlayManager.h"
#include "RALogManager.h"
#include "RAES2TextOverlay.h"
#include "RAVisibleSet.h"

namespace tyro
{

OverlayManager* OverlayManager::mInstance = 0;

OverlayManager* OverlayManager::GetSingleton()
{
    if (!mInstance)
    {
        mInstance = new OverlayManager();
    }
    return mInstance;
}

tyro::ObjectID OverlayManager::AddOverlay(ES2TextOverlaySPtr overlay)
{
    auto it = mOverlays.find(overlay->GetID());
    if (it == mOverlays.end())
    {
        mOverlays[overlay->GetID()] = overlay;
        return overlay->GetID();
    }
    else
    {
        RA_LOG_ERROR_ASSERT("Overlay with ID %i exist", overlay->GetID());
        return -1;
    }
}

ES2TextOverlaySPtr OverlayManager::GetOverlay(ObjectID objID)
{
    return mOverlays[objID];
}

void OverlayManager::RemoveOverlay(ObjectID objID)
{
    for (auto it = mOverlays.begin(); it != mOverlays.end(); ++it)
    {
        if (it->second->GetID() == objID)
        {
            mOverlays.erase(it);
            return;
        }
    }
    
    RA_LOG_WARN_ASSERT("Didn't find overlay with id %i", objID);
}

std::map<ObjectID, ES2TextOverlaySPtr> OverlayManager::GetOverlays()
{
    return mOverlays;
}

void OverlayManager::GetVisibleSet(VisibleSet* visibleSet)
{
    if (!mHide)
    {
        for (auto it = mOverlays.begin(); it != mOverlays.end(); ++it)
        {
            if (it->second->Visible)
            {
                visibleSet->Insert(it->second.get());
            } 
        }
    }
}

const Wm5::Vector4i& OverlayManager::GetViewPort() const
{
    return mViewport;
}

void OverlayManager::SetViewPort(const Wm5::Vector4i& viewport)
{
    mViewport = viewport;
    
    for (auto it = mOverlays.begin(); it != mOverlays.end(); ++it)
    {
        it->second->SetViewport(viewport);
    }
    
}
}