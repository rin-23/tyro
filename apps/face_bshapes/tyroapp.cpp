#include "tyroapp.h"
#include "RAEnginePrerequisites.h"
#include "RAVisibleSet.h"
#include "RAFont.h"
#include <stdio.h>
#include <functional>
#include "Wm5APoint.h"
#include "Wm5Vector2.h"
#include "Wm5Vector4.h"
#include <random>
#include <iostream>

using namespace std;

namespace tyro
{   
    App::App()
    :
    m_tyro_window(nullptr),
    m_gl_rend(nullptr),
    m_camera(nullptr),
    mouse_is_down(false),
    gesture_state(0),
    show_console(false)
    {}

    App::~App() 
    {
        if (m_tyro_window)
        {
            m_tyro_window->Terminate();
            delete m_tyro_window; 
        }

        if (m_gl_rend) 
            delete m_gl_rend;
        
        if (m_camera)
            delete m_camera;
    }
    
    int App::Setup(int width, int height) 
    {
        //setup windowshapes
        m_tyro_window = new Window();
        m_tyro_window->Init(width, height);
                
        //setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        m_gl_rend->SetClearColor(Wm5::Vector4f(150/255.0, 150/255.0, 150/255.0, 1));

        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(Wm5::APoint(0,0,0), 1.0, 1.0, 2, viewport, true);
       
        //set up window callbacks
        //@TODO use std::bind instead
        m_tyro_window->callback_mouse_down = [&](Window& window, int button, int modifier)->bool 
        {
            this->mouse_down(window, button, modifier);
        };

        m_tyro_window->callback_mouse_up = [&](Window& window, int button, int modifier)->bool 
        {
            this->mouse_up(window, button, modifier);
        };

        m_tyro_window->callback_mouse_move = [&](Window& window, int mouse_x, int mouse_y)->bool 
        {
            this->mouse_move(window, mouse_x, mouse_y);
        };

        m_tyro_window->callback_window_resize = [&](Window& window, unsigned int w, unsigned int h)->bool 
        {
            this->window_resize(window, w, h);
        };

        m_tyro_window->callback_key_pressed = [&](Window& window, unsigned int key, int modifiers)->bool 
        {
            this->key_pressed(window, key, modifiers);
        };

        m_tyro_window->callback_key_down = [&](Window& window, unsigned int key, int modifiers)->bool 
        {
            this->key_down(window, key, modifiers);
        };
        
        m_tyro_window->callback_mouse_scroll = [&](Window& window, float ydelta)->bool 
        {
            this->mouse_scroll(window, ydelta);
        };
     

        FontManager* fManager = FontManager::GetSingleton();
        float scale = 1;
        float ppi = 144;
        fManager->Setup(ppi, scale);
    }
    
    int App::Launch()
    {   
        RA_LOG_INFO("Launching the app");
     
        while (!m_tyro_window->ShouldClose())
        {   
            m_gl_rend->ClearScreen();
            m_tyro_window->ProcessUserEvents();

            // Draw console
            if (show_console) 
            {
                glUseProgram(0);
                m_console.display(2);
            }
            
            m_tyro_window->GetGLContext()->swapBuffers();
        }

	    return 0;
    }
    
    void App::render() 
    {}

    void App::update_camera() 
    {
        //setup camera
        Wm5::APoint world_center = Wm5::APoint::ORIGIN;
        float radius = 1.0;
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        float aspect = (float)v_width/v_height;
        
        if (m_camera)
            delete m_camera;
        
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
    }

    void App::mouse_down(Window& window, int button, int modifier) 
    {   
        //RA_LOG_INFO("mouse down %i", button);

        mouse_is_down = true;
        m_modifier = modifier;
        m_mouse_btn_clicked = button;

        if (m_modifier == TYRO_MOD_CONTROL) return; //rotating
        if (button == 0 && m_modifier == TYRO_MOD_SHIFT) 
        {   
            //m_square_sel_start_x = current_mouse_x;
            //m_square_sel_start_y = current_mouse_y;
            return; //selection
        }
        if (m_mouse_btn_clicked == 2) return; //translating
        
        // Cast a ray in the view direction starting from the mouse position
        double x = current_mouse_x;
        double y = m_camera->GetViewport()[3] - current_mouse_y;
        //Eigen::Vector2f mouse_pos(x,y);
        //selectVertex(mouse_pos, button, modifier);     
    }

    void App::mouse_up(Window& window, int button, int modifier) 
    {   
        //RA_LOG_INFO("MOUSE_UP");
        if (mouse_is_down && m_modifier == TYRO_MOD_CONTROL) 
        {   
            gesture_state = 2;
            m_camera->HandleOneFingerPanGesture(gesture_state, Wm5::Vector2i(current_mouse_x, current_mouse_y));
            render();
        }
        else if (mouse_is_down && m_mouse_btn_clicked == 2) 
        {
            gesture_state = 2;
            m_camera->HandleTwoFingerPanGesture(gesture_state, Wm5::Vector2i(current_mouse_x, -current_mouse_y));
            render();
        }
        
        mouse_is_down = false;
        gesture_state = 0;
    }

    void App::mouse_move(Window& window, int mouse_x, int mouse_y) 
    {   
        //RA_LOG_INFO("mouse move state %i", m_state);
        
        current_mouse_x = mouse_x;
        current_mouse_y = mouse_y;

        if (mouse_is_down && m_modifier == TYRO_MOD_CONTROL) 
        {   
            m_camera->HandleOneFingerPanGesture(gesture_state, Wm5::Vector2i(mouse_x, mouse_y));
            gesture_state = 1;
            render();
        } 
        else if (mouse_is_down && m_mouse_btn_clicked == 2) 
        {
            m_camera->HandleTwoFingerPanGesture(gesture_state, Wm5::Vector2i(mouse_x, -mouse_y));
            gesture_state = 1;
            render();
        } 
    }

    void App::mouse_scroll(Window& window, float ydelta) 
    {   
        //RA_LOG_INFO("mouse scroll delta %f", ydelta);
        
        m_camera->HandlePinchGesture(gesture_state, Wm5::Vector2i(current_mouse_x, current_mouse_y), ydelta);
        render();
    } 

    void App::window_resize(Window& window, unsigned int w, unsigned int h)
    {
        //RA_LOG_INFO("window resized")
        //float  aspect = m_gl_rend->GetViewWidth()/ (float) m_gl_rend->GetViewHeight();
        //Vector4i viewport(0, 0, m_gl_rend->GetViewWidth(), m_gl_rend->GetViewHeight());
        //m_camera->SetAspect(aspect);
        //m_camera->SetViewport(viewport);
        //render();
        //update_camera();
    }

    void App::key_pressed(Window& window, unsigned int key, int modifiers) 
    {   
        //RA_LOG_INFO("Key pressed %c", key);
        
        if (key == '`') 
        {   
            //RA_LOG_INFO("Pressed %c", key);
            show_console = !show_console;
            render();
            return;
        }

        if (show_console) 
        {  
           m_console.keyboard(key);
           render();
           return;
        }
    }
    
    void App::key_down(Window& window, unsigned int key, int modifiers) 
   {   
        //RA_LOG_INFO("Key down %i", key)
        // handle not text keys   

        if (key == TYRO_KEY_LEFT){ 
            m_console.key_left();
            render();}
        else if (key == TYRO_KEY_RIGHT){ 
            m_console.key_right();
            render();}
        else if (key == TYRO_KEY_ENTER){
            m_console.key_enter();
            render();
            }
        else if (key == TYRO_KEY_BACKSPACE){
            m_console.key_backspace();render();}
        else if (key == TYRO_KEY_UP){
            m_console.key_up();render();}
        else if (key == TYRO_KEY_DOWN){
            m_console.key_down();render();}
        else if (key == TYRO_KEY_TAB){
            m_console.key_tab();render();}
    }

    void App::register_console_function(const std::string& name,
                                        const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                        const std::string& help_txt)
    {
        std::function<void (const std::vector<std::string>&)> f = bind(con_fun, this, std::placeholders::_1);
        m_console.reg_cmdN(name, f, help_txt);
    }
}