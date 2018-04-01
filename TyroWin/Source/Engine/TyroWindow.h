#pragma once

#include <GL/glew.h>
//#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "RAES2Context.h"
#include <functional>

namespace tyro 
{
    class Window 
    {
    public:
        enum MouseButton 
        {
            MB_Left,
            MB_Right,
            MB_Middle
        };
        
        Window() {}
        
        ~Window() {}

        //setup glfw
        //return -1 if failed to initialize glfw window
        int Init();
    
        int Terminate();

        inline int ShouldClose() { return glfwWindowShouldClose(m_glfw_window); }

        inline ES2Context* GetGLContext() { return m_gl_context; }

        inline void ProcessUserEvents() { glfwPollEvents();}

        std::function<bool(Window& window)> callback_init;
        std::function<bool(Window& window)> callback_pre_draw;
        std::function<bool(Window& window)> callback_post_draw;
        std::function<bool(Window& window, int button, int modifier)> callback_mouse_down;
        std::function<bool(Window& window, int button, int modifier)> callback_mouse_up;
        std::function<bool(Window& window, int mouse_x, int mouse_y)> callback_mouse_move;
        std::function<bool(Window& window, float delta_y)> callback_mouse_scroll;
        std::function<bool(Window& window, unsigned int key, int modifiers)> callback_key_pressed;
        std::function<bool(Window& window, unsigned int key, int modifiers)> callback_key_down;
        std::function<bool(Window& window, unsigned int key, int modifiers)> callback_key_up;

        bool mouse_down(MouseButton button, int modifier);
        bool mouse_up(MouseButton button, int modifier);

    private:
        GLFWwindow* m_glfw_window;
        ES2Context* m_gl_context;
    };
}