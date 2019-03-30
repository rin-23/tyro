#pragma once

#include "tyroapp.h"
#include "bshapes.h"
#include "TyroTimeLine.h"
#include "animation.h"
#include <opencv2/opencv.hpp>
#include "torch_model.h"
#include "kdtree.h"
#include "gamepad.h"
#include "sliders.h"
#include "cameracontrol.h"
#include "ES2VideoTexture.h"
#include "renderdata.h"

namespace tyro
{   
    class BshapeApp : public App 
    {
    public:
        
        typedef enum 
        {   
            PS4,
            CAMERA,
            SLIDERS
        } Experiment;
        
        BshapeApp(Experiment exp);
        virtual ~BshapeApp();
        
        int Setup(int width, int height, bool refinment=false);
        int Launch() override;
        int LaunchOffScreen(const std::string& csv_file, const std::string& out_fldr, const std::string& video_stream, bool isOpenFace); 
        int LaunchRefinment();
        int LaunchOpenFaceImages(const std::string& csv_file, const std::string& out_fldr);  
        void key_pressed(Window& window, unsigned int key, int modifiers) override; 
        void update_camera() override;

        void GamepadExample();
        void SliderExample();
        void CameraExample();

        //void render();
        //void DrawUI();
        //void loadAnimation(const std::string& name);
        //void loadFrame(int frame);
        //void loadOpenFace();
        //void ComputeDistanceToManifold();

        enum State {None, Launched, LoadedModel, PlayingAnimation};
        State m_state;
        
        Timeline* m_timeline;

        ES2TextOverlaySPtr m_frame_overlay;
        ES2TextOverlaySPtr m_dist1;
        ES2TextOverlaySPtr m_dist2;

        std::atomic<bool> m_need_rendering;
        bool m_show_wire;
        int m_frame;
        int file_selected = -1; //(1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.

        OpenFaceTextureSPtr m_camera_texture;
        ES2VideoTextureSPtr m_video_texture;

        KDTree mTree;
        
        MRenderData RENDER;

        FaceModel mFaceModel;

        Animation mCurAnimation;
        TorchModel mTorchModelLow;
        TorchModel mTorchModelUp;

        Gamepad mGamepad;
        Sliders mSliders;
        CameraControl mCameraControl;

        //std::vector<double> low_values_denoised;
        //std::vector<std::string> low_bnames;
        //std::vector<double> low_values; 

        //std::vector<double> up_values_denoised;
        //std::vector<std::string> up_bnames;
        //std::vector<double> up_values; 

        std::vector<std::vector<double>> MOTION_DATA;

        Experiment mExperiment;
    };

    
}