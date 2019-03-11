#pragma once

#include "tyroapp.h"
#include "bshapes.h"
#include "TyroTimeLine.h"
#include "animation.h"
#include <opencv2/opencv.hpp>
#include "torch_model.h"
#include "kdtree.h"
#include "gamepad.h"

namespace tyro
{   
    class BshapeApp : public App 
    {
    public:
        BshapeApp();
        virtual ~BshapeApp();
        
        int Launch() override;
        int LaunchOffScreen(const std::string& csv_file, const std::string& out_fldr); 
        int Setup(int width, int height) override;
        void key_pressed(Window& window, unsigned int key, int modifiers) override; 
        void update_camera() override;
        //void render();
        void DrawMeshes();
        //void DrawUI();
        void FetchGamepadInputs();
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

        KDTree mTree;

        struct MRenderData 
        {
            IGLMeshSPtr mesh;
            IGLMeshSPtr mesh2;
        };
        MRenderData RENDER;

        FaceModel mFaceModel;
        FaceModel mFaceModel2;

        Animation mCurAnimation;
        TorchModel mTorchModelLow;
        TorchModel mTorchModelUp;

        Gamepad mGamepad;

        std::vector<double> low_values_denoised;
        std::vector<std::string> low_bnames;
        std::vector<double> low_values; 

        std::vector<double> up_values_denoised;
        std::vector<std::string> up_bnames;
        std::vector<double> up_values; 

        std::vector<std::vector<double>> MOTION_DATA;
    };

    
}