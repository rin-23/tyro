#pragma once

#include "RAES2Renderable.h"
#include "Wm5Vector2.h"
#include "Wm5Vector3.h"
#include "Wm5Vector4.h"
#include "RATexture.h"
#include <opencv2/opencv.hpp>

namespace tyro
{   
    class ES2CameraTexture : public ES2Renderable
    {
    public:
        
        ES2CameraTexture() {}
        
        virtual ~ES2CameraTexture() 
        {
            capture->release();
            delete capture;
        }
        
        static ES2CameraTextureSPtr Create();

        void showFrame(); //asumes cv VideoCapture object is available

        virtual void UpdateUniformsWithCamera(const Camera* camera) override;
    
    protected:

        void Init(int width, 
                  int height, 
                  Texture::TextureFormat texformat);
        void Init();

    private:
        size_t mAllocSize;
        int mWidth;
        int mHeight;
        //int mNumFrames;
        int mCurrentFrame;
        Texture::TextureFormat mTexFormat;
        cv::VideoCapture* capture;
        void _UpdateGeometry();
  };
}

