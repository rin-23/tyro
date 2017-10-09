//
//  RAES2TextOverlay.h
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-05-01.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#pragma once

#include "RAES2Renderable.h"
#include "Wm5Vector2.h"
#include "Wm5Vector3.h"
#include "Wm5Vector4.h"

namespace RAEngine
{
    class ES2TextOverlay : public ES2Renderable
    {
    public:
        
        ES2TextOverlay() {}
        
        virtual ~ES2TextOverlay() {}
        
        static ES2TextOverlaySPtr Create(const std::string& text, const Wm5::Vector2f& origin, ES2FontSPtr font, const Wm5::Vector4f& color, const Wm5::Vector4i& viewport);

        void SetWorldPos(const Wm5::Vector3f& worldPos);
        
        void SetViewport (const Wm5::Vector4i& viewport);
        
        void SetTranslate(const Wm5::Vector2i& pixels);

        void SetTranslate(const Wm5::Vector2f& ndc);
        
        void Translate(const Wm5::Vector2i& pixels);

        void Translate(const Wm5::Vector2f& ndc);
        
        virtual void SetText(const std::string& text);
        
        virtual void UpdateUniformsWithCamera(const Camera* camera) override;
        
        Wm5::Transform ScreenTransform;
        
    protected:

        void Init(const std::string& text, const Wm5::Vector2f& origin, ES2FontSPtr font, const Wm5::Vector4f& color, const Wm5::Vector4i& viewport);

    private:

        ES2FontSPtr mFont;
        std::string mText;
        size_t mAllocSize;
        Wm5::Vector4f mColor;
        Wm5::Vector2f mOrigin;
        Wm5::Vector4i mViewport;
        Wm5::Vector3f mWorldPos;

        void _UpdateGeometry();
    };
}

