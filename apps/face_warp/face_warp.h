#pragma once

#include "TyroWindow.h"
#include "RAES2Renderer.h"
#include "RAiOSCamera.h"
#include "Console.h"
#include <vector>
//#include <Eigen/Core>
#include <atomic>
#include "Wm5Transform.h"
#include "RAEnginePrerequisites.h"

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
        void selectVertex(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier);

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
          
        struct MGeometry 
        {
            Eigen::MatrixXd VS; 
            Eigen::MatrixXd VT;
            Eigen::MatrixXi FS;
            Eigen::MatrixXi FT;
            Eigen::MatrixXd NS;
            Eigen::MatrixXd NT;
        };
        MGeometry GEOMETRY;        

        struct MRenderData 
        {
            IGLMeshSPtr scan;
            IGLMeshSPtr template_mesh;
            IGLMeshWireframeSPtr scan_wire;
            IGLMeshWireframeSPtr template_mesh_wire;
        };
        MRenderData RENDER;

            
        std::vector<SpatialSPtr> ball_list_scan;
        std::vector<int> vid_list_scan;
        std::vector<SpatialSPtr> ball_list_template;
        std::vector<int> vid_list_template;
        std::vector<ES2TextOverlaySPtr> vid_numbers_template;


        void register_console_function(const std::string& name,
                                       const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                       const std::string& help_txt);
        void update_camera();
        void render();
        void DrawMeshes();
        void add_vertex(int vid, bool isScan);
        void addSphere(int vid, const Eigen::MatrixXd& V, Wm5::Vector4f color, Wm5::Transform worldT, bool isScan);        
        bool testScanClicked(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier, int& fid, Eigen::Vector3f& bc);
        bool testTemplateClicked(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier, int& fid, Eigen::Vector3f& bc);

        void addPointText(int vid, const Eigen::MatrixXd& V, Wm5::Vector4f color, bool isScan);
    };

    
}