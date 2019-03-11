#pragma once

#include "TyroWindow.h"
#include "RAES2Renderer.h"
#include "RAiOSCamera.h"
#include "Console.h"
#include <vector>
#include <atomic>
#include "Wm5Transform.h"

namespace tyro
{   
    class App 
    {
    public:
        App();
        
        virtual ~App();
        
        virtual int Launch();
        virtual int Setup(int width, int height);
        
        Window* m_tyro_window;
        ES2Renderer* m_gl_rend;
        iOSCamera* m_camera;

        virtual void mouse_down(Window& window, int button, int modifier);
        virtual void mouse_up(Window& window, int button, int modifier);
        virtual void mouse_move(Window& window, int mouse_x, int mouse_y);
        virtual void mouse_scroll(Window& window, float ydelta); 
        virtual void window_resize(Window& window, unsigned int w, unsigned int h);    
        virtual void key_pressed(Window& window, unsigned int key, int modifiers); 
        virtual void key_down(Window& window, unsigned int key, int modifiers); 
        virtual void register_console_function(const std::string& name,
                                       const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                       const std::string& help_txt);
        virtual void update_camera();
        virtual void render();
        
        bool mouse_is_down;
        int gesture_state; 
        int current_mouse_x;
        int current_mouse_y;
        bool show_console;        
        int m_modifier;
        int m_mouse_btn_clicked;
        Console m_console;

      
    };
}