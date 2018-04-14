#pragma once

#include "TyroWindow.h"
#include "RAES2Renderer.h"
#include "RAiOSCamera.h"
#include "Console.h"
#include <vector>
#include <Eigen/Core>
#include "TyroTimeLine.h"
#include <atomic>

namespace tyro
{
    class App 
    {
    public:
        App();
        
        ~App();
        
        int Launch();

        void load_blobby();
        void load_oldman();
        void compute_average();
        void compute_deformation();
        
    private:
        Timeline* m_timeline;
        Window* m_tyro_window;
        ES2Renderer* m_gl_rend;
        iOSCamera* m_camera;
        
        void mouse_down(Window& window, int button, int modifier);
        void mouse_up(Window& window, int button, int modifier);
        void mouse_move(Window& window, int mouse_x, int mouse_y);
        void window_resize(Window& window, unsigned int w, unsigned int h);    
        void key_pressed(Window& window, unsigned int key, int modifiers); 
        void key_down(Window& window, unsigned int key, int modifiers); 

        bool mouse_is_down;
        int gesture_state; 
        int current_mouse_x;
        int current_mouse_y;
        bool show_console;        
        int m_modifier;

        Console m_console;

        IGLMeshSPtr igl_mesh;
        IGLMeshWireframeSPtr igl_mesh_wire;

        std::vector<SpatialSPtr> object_list;
        std::vector<SpatialSPtr> ball_list;
        std::atomic<int> m_frame;
        std::atomic<int> m_old_frame;

        typedef struct MAnimation 
        {   
            std::vector<Eigen::MatrixXd> v_data; // Vertex data. 3*num_vert by num_frames. 
            std::vector<Eigen::MatrixXd> n_data; // Normal data. 3*num_vert by num_frames. 
            Eigen::MatrixXi f_data; // Face data.   num_faces by 3
            Eigen::MatrixXd avg_v_data;

            //Eigen::MatrixXd v_data;
            //Eigen::MatrixXd n_data;

        } MAnimation;

        MAnimation m_frame_data;

        std::vector<int> vid_list;

        void register_console_function(const std::string& name,
                                   const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                   const std::string& help_txt);
        void load_mesh_sequence(const std::string& obj_list_file); 
        void update_camera(const Spatial& spatial);
    };
}