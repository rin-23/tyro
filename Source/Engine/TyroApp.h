#include "TyroWindow.h"
#include "RAES2Renderer.h"
#include "RAiOSCamera.h"
#include "Console.h"
#include <vector>

namespace tyro
{
    class App 
    {
    public:
        App();
        
        ~App();
        
        int Launch();

        void load_hiroki();

    private:
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
        
        Console m_console;
        //VisibleSet* vis_set;

        IGLMeshSPtr igl_mesh;
        IGLMeshWireframeSPtr igl_mesh_wire;

        std::vector<SpatialSPtr> object_list;

        void register_console_function(const std::string& name,
                                   const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                   const std::string& help_txt);


    };
}