#pragma once

#include "RAES2Renderable.h"
#include "Wm5Vector2.h"
#include "Wm5Vector3.h"
#include "Wm5Vector4.h"
#include "RATexture.h"
#include <opencv2/opencv.hpp>

namespace tyro
{   
    class ES2VideoTexture : public ES2Renderable
    {
    public:
        
        ES2VideoTexture() {}
        
        virtual ~ES2VideoTexture() 
        {
            capture->release();
        }
        
        static ES2VideoTextureSPtr Create(const std::string& video_path);
        static ES2VideoTextureSPtr Create(int width, 
                                          int height, 
                                          int num_frames, 
                                          Texture::TextureFormat texformat);
        
        virtual void UpdateUniformsWithCamera(const Camera* camera) override;
        
        void showFrame(int frame, const void* data);
        void showFrame(int frame); //asumes cv VideoCapture object is available
    
    protected:

        void Init(int width, 
                  int height, 
                  int num_frames, 
                  Texture::TextureFormat texformat);
        void Init(const std::string& video_path);

    private:
        size_t mAllocSize;
        int mWidth;
        int mHeight;
        int mNumFrames;
        int mCurrentFrame;
        Texture::TextureFormat mTexFormat;
        cv::VideoCapture* capture;
        void _UpdateGeometry();
  };
}

