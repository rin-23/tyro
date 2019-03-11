#pragma once

#include "TyroWindow.h"
#include "RAES2Renderer.h"
#include "RAiOSCamera.h"
#include "Console.h"
#include <vector>
#include <atomic>
#include "Wm5Transform.h"
#include "bshapes.h"
#include "TyroTimeLine.h"
#include "animation.h"
#include <opencv2/opencv.hpp>
#include "torch_model.h"
#include "kdtree.h"
#include "gamepad.h"

namespace tyro
{   
    class App 
    {
    public:

        enum State {None, Launched, LoadedModel, PlayingAnimation};

        App();
        
        ~App();
        
        int Launch();
        int LaunchOffScreen(const std::string& csv_file, const std::string& out_fldr); 
        int Setup();
        State m_state;
        
        Timeline* m_timeline;
        Window* m_tyro_window;
        ES2Renderer* m_gl_rend;
        iOSCamera* m_camera;

        ES2TextOverlaySPtr m_frame_overlay;
        ES2TextOverlaySPtr m_dist1;
        ES2TextOverlaySPtr m_dist2;

        std::atomic<bool> m_need_rendering;
        void mouse_down(Window& window, int button, int modifier);
        void mouse_up(Window& window, int button, int modifier);
        void mouse_move(Window& window, int mouse_x, int mouse_y);
        void mouse_scroll(Window& window, float ydelta); 
        void window_resize(Window& window, unsigned int w, unsigned int h);    
        void key_pressed(Window& window, unsigned int key, int modifiers); 
        void key_down(Window& window, unsigned int key, int modifiers); 
        
        bool mouse_is_down;
        int gesture_state; 
        int current_mouse_x;
        int current_mouse_y;
        bool show_console;        
        int m_modifier;
        int m_mouse_btn_clicked;
        Console m_console;
        //bool m_update_camera;
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

        void register_console_function(const std::string& name,
                                       const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                       const std::string& help_txt);
        void update_camera();
        void render();
        void DrawMeshes();
        //void DrawUI();
        void FetchGamepadInputs();

        //void loadAnimation(const std::string& name);
        //void loadFrame(int frame);
        //void loadOpenFace();
        
        //void ComputeDistanceToManifold();

        std::vector<std::vector<double>> MOTION_DATA;


        
    };

    
}