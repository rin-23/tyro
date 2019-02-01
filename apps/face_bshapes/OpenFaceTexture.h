#pragma once

#include "RAES2Renderable.h"
#include "Wm5Vector2.h"
#include "Wm5Vector3.h"
#include "Wm5Vector4.h"
#include "RATexture.h"
#include <opencv2/opencv.hpp>
#include <SequenceCapture.h>
#include "LandmarkCoreIncludes.h"
#include <Visualizer.h>
#include <VisualizationUtils.h>


namespace tyro
{   
    class OpenFaceTexture : public ES2Renderable
    {
    public:
        
        OpenFaceTexture() {}
        
        virtual ~OpenFaceTexture() 
        {
          	face_model.Reset();
		    sequence_reader.Close();
        }
        
        static OpenFaceTextureSPtr Create();

        void showFrame(); //asumes cv VideoCapture object is available

        virtual void UpdateUniformsWithCamera(const Camera* camera) override;
    
    protected:

        int Init(int width, 
                  int height, 
                  Texture::TextureFormat texformat);
        int Init();

    private:
        size_t mAllocSize;
        int mWidth;
        int mHeight;
        //int mNumFrames;
        int mCurrentFrame;
        Texture::TextureFormat mTexFormat;
        //cv::VideoCapture* capture;
        void _UpdateGeometry();
        Utilities::Visualizer* visualizer;
        Utilities::SequenceCapture sequence_reader;
        LandmarkDetector::CLNF face_model;
        LandmarkDetector::FaceModelParameters det_parameters;


  };
}

