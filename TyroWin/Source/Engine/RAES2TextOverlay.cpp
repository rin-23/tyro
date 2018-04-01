//
//  RAES2TextOverlay.cpp
//  Ossa
//
//  Created by Rinat Abdrashitov on 2015-05-01.
//  Copyright (c) 2015 Rinat Abdrashitov. All rights reserved.
//

#include "RAES2TextOverlay.h"
#include "RAES2CoreVisualEffects.h"
#include "GLStructures.h"
#include "RAFont.h"
#include "RACamera.h"

namespace tyro
{
    
void ES2TextOverlay::Init(const std::string& text, const Wm5::Vector2f& origin, ES2FontSPtr font, const Wm5::Vector4f& color, const Wm5::Vector4i& viewport)
{
    ES2Renderable::Init(PT_TRIANGLES);
    mText = text;
    if (mText.length() == 0)
    {
        mText = " ";
    }
    mOrigin = origin;
    mFont = font;
    mColor = color;
    mViewport = viewport;
    mAllocSize = 0;
    ScreenTransform.MakeIdentity();
    mWorldPos = Wm5::Vector3f(0,0,0);
    
    SetVisualEffect(ES2CoreVisualEffects::BitmapText());
    GetVisualEffect()->SetTexture2D(font->GetTextureAtlas());
    _UpdateGeometry();
}
    
ES2TextOverlaySPtr ES2TextOverlay::Create(const std::string& text, const Wm5::Vector2f& origin, ES2FontSPtr font, const Wm5::Vector4f& color, const Wm5::Vector4i& viewport)
{
    ES2TextOverlaySPtr ptr = std::make_shared<ES2TextOverlay>();
    ptr->Init(text, origin, font, color, viewport);
    return ptr;
}


void ES2TextOverlay::SetWorldPos(const Wm5::Vector3f& worldPos)
{
    mWorldPos = worldPos;
}

void ES2TextOverlay::Translate(const Wm5::Vector2i& pixels)
{
    float sx = 2.0 / mViewport[2];
    float sy = 2.0 / mViewport[3];
    Translate(Wm5::Vector2f(pixels.X()*sx, pixels.Y()*sy));
}

void ES2TextOverlay::Translate(const Wm5::Vector2f& ndc)
{
    Wm5::Transform trans;
    trans.SetTranslate(Wm5::APoint(ndc.X(), ndc.Y(), 0));
    ScreenTransform = trans * ScreenTransform;
}

void ES2TextOverlay::SetTranslate(const Wm5::Vector2i& pixels)
{
    ScreenTransform.MakeIdentity();
    Translate(pixels);
}

void ES2TextOverlay::SetTranslate(const Wm5::Vector2f& ndc)
{
    ScreenTransform.MakeIdentity();
    Translate(ndc);
}

void ES2TextOverlay::SetText(const std::string& text)
{
    mText = text;
    if (mText.length() == 0)
    {
        mText = " ";
    }
    _UpdateGeometry();
}

void ES2TextOverlay::SetViewport (const Wm5::Vector4i& viewport)
{
    mViewport = viewport;
    _UpdateGeometry();
}

void ES2TextOverlay::UpdateUniformsWithCamera(const Camera* camera)
{
    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(0, mColor);
    int uSampler = 0;
    GetVisualEffect()->GetUniforms()->UpdateIntUniform(1, &uSampler);

//    Transform viewProjTransform;
//    viewProjTransform.SetMatrix(camera->GetViewProjectionMatrix());
//
//    Vector3f wCorners[8];
//    Vector2f sCorners[8];
//    mSpatial->WorldBoundBox.ComputeVertices(wCorners);
//    HMatrix VPMatrix = camera->GetViewProjectionMatrix();
//    for (int i = 0; i < 8; i++)
//    {
//        APoint p = VPMatrix * APoint(wCorners[i]);
//        sCorners[i] = Vector2f(p[0], p[1]);
//    }
//
//    Wm5::AxisAlignedBox2f bbox;
//    bbox.ComputeExtremes(8, sizeof(Vector2f), sCorners);
//
//    Vector2f center;
//    float extent[2];
//    bbox.GetCenterExtents(center, extent);
//    Vector2f textPoint = center + Vector2f(extent[0], extent[1]);
//    Transform tr;
//    tr.SetTranslate(Vector3f(textPoint.X(), textPoint.Y(), 0));
    
//    Vector3f center = mSpatial->WorldBoundBox.GetCenter() + mSpatial->WorldBoundBox.GetHalfSize();
//    center = (camera->GetViewProjectionMatrix() * APoint(center)).operator Wm5::Vector3<float> &();
    
    Wm5::Vector3f center = mWorldPos;
    Wm5::APoint pCenter = (camera->GetViewProjectionMatrix() * Wm5::APoint(center));
    pCenter.X() = pCenter.X()/pCenter.W();
    pCenter.Y() = pCenter.Y()/pCenter.W();
    Wm5::Transform tr;
    tr.SetTranslate(Wm5::Vector3f(pCenter.X(), pCenter.Y(), 0));

    GetVisualEffect()->GetUniforms()->UpdateFloatUniform(2, tr.Matrix().Transpose());
}

void ES2TextOverlay::_UpdateGeometry()
{
    if (mText.length() == 0)
    {
        RA_LOG_ERROR_ASSERT("Zero length text");
        return;
    }
    VertexTexture* coords = new VertexTexture[6 * mText.length()];
    
    float sx = 2.0 / mViewport[2];
    float sy = 2.0 / mViewport[3];
    float x = mOrigin.X();
    float y = mOrigin.Y();
    int n = 0;
    
    for (auto it = mText.begin(); it != mText.end(); ++it)
    {
        const ES2Font::GlyphInfo* gInfo = mFont->GetGlyphInfo(*it);
     
//        RA_LOG_INFO("Glyph %c %c %f %f", *i, gInfo->character, gInfo->uv_lb.X(), gInfo->uv_lb.Y());
        
        float x2 = x + gInfo->bl * sx;
        float y2 = y + gInfo->bt * sy;
        float w = gInfo->bw * sx;
        float h = gInfo->bh * sy;
        
        /* Advance the cursor to the start of the next character */
        x += gInfo->ax * sx;
        y += gInfo->ay * sy;

        using Wm5::Vector3f;
        coords[n++] = {Vector3f(x2,     y2,     0), gInfo->uv_lb};
        coords[n++] = {Vector3f(x2,     y2 - h, 0), gInfo->uv_lt};
        coords[n++] = {Vector3f(x2 + w, y2,     0), gInfo->uv_rb};

        coords[n++] = {Vector3f(x2 + w, y2    , 0), gInfo->uv_rb};
        coords[n++] = {Vector3f(x2,     y2 - h, 0), gInfo->uv_lt};
        coords[n++] = {Vector3f(x2 + w, y2 - h, 0), gInfo->uv_rt};
    }
    
    int numOfVerticies = mText.length() * 6; //6 verticies per glyph
    if (mText.length() != mAllocSize)
    {
        SetVertexBuffer(std::make_shared<ES2VertexHardwareBuffer>(sizeof(VertexTexture), numOfVerticies, coords, HardwareBuffer::BU_DYNAMIC));
    }
    else
    {
        GetVertexBuffer()->WriteData(0, numOfVerticies * sizeof(VertexTexture), coords);
    }

    mAllocSize = mText.length();
    delete [] coords;
}
}