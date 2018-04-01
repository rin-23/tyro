#pragma once

#include <GL/glew.h>
//#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "RAES2Context.h"

namespace tyro 
{
    class Window 
    {
    public:
        Window() {}
        ~Window() {}

        //setup glfw
        //return -1 if failed to initialize glfw window
        int Init();

        int Terminate();
  
        inline int ShouldClose() { return glfwWindowShouldClose(m_glfw_window); }

        inline ES2Context* GetGLContext() { return m_gl_context; }

        inline void ProcessUserEvents() { glfwPollEvents();}

    private:
        GLFWwindow* m_glfw_window;
        ES2Context* m_gl_context;
    };
}