#pragma once

#include "TyroWindow.h"
#include "RAES2Renderer.h"
#include "RAiOSCamera.h"
#include "Console.h"
#include <vector>
//#include <Eigen/Core>
#include <atomic>
#include "Wm5Transform.h"
#include "bshapes.h"

namespace tyro
{   
    class App 
    {
    public:

        enum State {None, Launched, LoadedModel, PlayingAnimation};

        App();
        
        ~App();
        
        int Launch();
        State m_state;
        
        //Timeline* m_timeline;
        Window* m_tyro_window;
        ES2Renderer* m_gl_rend;
        iOSCamera* m_camera;
        ES2TextOverlaySPtr m_frame_overlay;
        std::atomic<bool> m_need_rendering;
        //ShaderBoxSPtr m_shaderbox;
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
        bool m_computed_deformation;
        bool m_computed_avg;
        bool m_computed_stop_motion;
        bool m_computed_parts;
        Console m_console;
        bool m_update_camera;
        bool m_show_wire;
          
        struct MRenderData 
        {
            IGLMeshSPtr mesh;
        };
        MRenderData RENDER;

        FaceModel fmodel;

        void register_console_function(const std::string& name,
                                       const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                       const std::string& help_txt);
        void update_camera();
        void render();
        void DrawMeshes();
    };

    
}