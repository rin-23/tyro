#pragma once

#include <GL/glew.h>
//#define GLFW_DLL
#include <GLFW/glfw3.h>
#include "RAES2Context.h"
#include <functional>

/* Printable keys */
#define TYRO_KEY_SPACE              32
#define TYRO_KEY_APOSTROPHE         39  /* ' */
#define TYRO_KEY_COMMA              44  /* , */
#define TYRO_KEY_MINUS              45  /* - */
#define TYRO_KEY_PERIOD             46  /* . */
#define TYRO_KEY_SLASH              47  /* / */
#define TYRO_KEY_0                  48
#define TYRO_KEY_1                  49
#define TYRO_KEY_2                  50
#define TYRO_KEY_3                  51
#define TYRO_KEY_4                  52
#define TYRO_KEY_5                  53
#define TYRO_KEY_6                  54
#define TYRO_KEY_7                  55
#define TYRO_KEY_8                  56
#define TYRO_KEY_9                  57
#define TYRO_KEY_SEMICOLON          59  /* ; */
#define TYRO_KEY_EQUAL              61  /* = */
#define TYRO_KEY_A                  65
#define TYRO_KEY_B                  66
#define TYRO_KEY_C                  67
#define TYRO_KEY_D                  68
#define TYRO_KEY_E                  69
#define TYRO_KEY_F                  70
#define TYRO_KEY_G                  71
#define TYRO_KEY_H                  72
#define TYRO_KEY_I                  73
#define TYRO_KEY_J                  74
#define TYRO_KEY_K                  75
#define TYRO_KEY_L                  76
#define TYRO_KEY_M                  77
#define TYRO_KEY_N                  78
#define TYRO_KEY_O                  79
#define TYRO_KEY_P                  80
#define TYRO_KEY_Q                  81
#define TYRO_KEY_R                  82
#define TYRO_KEY_S                  83
#define TYRO_KEY_T                  84
#define TYRO_KEY_U                  85
#define TYRO_KEY_V                  86
#define TYRO_KEY_W                  87
#define TYRO_KEY_X                  88
#define TYRO_KEY_Y                  89
#define TYRO_KEY_Z                  90
#define TYRO_KEY_LEFT_BRACKET       91  /* [ */
#define TYRO_KEY_BACKSLASH          92  /* \ */
#define TYRO_KEY_RIGHT_BRACKET      93  /* ] */
#define TYRO_KEY_GRAVE_ACCENT       96  /* ` */
#define TYRO_KEY_WORLD_1            161 /* non-US #1 */
#define TYRO_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define TYRO_KEY_ESCAPE             256
#define TYRO_KEY_ENTER              257
#define TYRO_KEY_TAB                258
#define TYRO_KEY_BACKSPACE          259
#define TYRO_KEY_INSERT             260
#define TYRO_KEY_DELETE             261
#define TYRO_KEY_RIGHT              262
#define TYRO_KEY_LEFT               263
#define TYRO_KEY_DOWN               264
#define TYRO_KEY_UP                 265
#define TYRO_KEY_PAGE_UP            266
#define TYRO_KEY_PAGE_DOWN          267
#define TYRO_KEY_HOME               268
#define TYRO_KEY_END                269
#define TYRO_KEY_CAPS_LOCK          280
#define TYRO_KEY_SCROLL_LOCK        281
#define TYRO_KEY_NUM_LOCK           282
#define TYRO_KEY_PRINT_SCREEN       283
#define TYRO_KEY_PAUSE              284
#define TYRO_KEY_F1                 290
#define TYRO_KEY_F2                 291
#define TYRO_KEY_F3                 292
#define TYRO_KEY_F4                 293
#define TYRO_KEY_F5                 294
#define TYRO_KEY_F6                 295
#define TYRO_KEY_F7                 296
#define TYRO_KEY_F8                 297
#define TYRO_KEY_F9                 298
#define TYRO_KEY_F10                299
#define TYRO_KEY_F11                300
#define TYRO_KEY_F12                301
#define TYRO_KEY_F13                302
#define TYRO_KEY_F14                303
#define TYRO_KEY_F15                304
#define TYRO_KEY_F16                305
#define TYRO_KEY_F17                306
#define TYRO_KEY_F18                307
#define TYRO_KEY_F19                308
#define TYRO_KEY_F20                309
#define TYRO_KEY_F21                310
#define TYRO_KEY_F22                311
#define TYRO_KEY_F23                312
#define TYRO_KEY_F24                313
#define TYRO_KEY_F25                314
#define TYRO_KEY_KP_0               320
#define TYRO_KEY_KP_1               321
#define TYRO_KEY_KP_2               322
#define TYRO_KEY_KP_3               323
#define TYRO_KEY_KP_4               324
#define TYRO_KEY_KP_5               325
#define TYRO_KEY_KP_6               326
#define TYRO_KEY_KP_7               327
#define TYRO_KEY_KP_8               328
#define TYRO_KEY_KP_9               329
#define TYRO_KEY_KP_DECIMAL         330
#define TYRO_KEY_KP_DIVIDE          331
#define TYRO_KEY_KP_MULTIPLY        332
#define TYRO_KEY_KP_SUBTRACT        333
#define TYRO_KEY_KP_ADD             334
#define TYRO_KEY_KP_ENTER           335
#define TYRO_KEY_KP_EQUAL           336
#define TYRO_KEY_LEFT_SHIFT         340
#define TYRO_KEY_LEFT_CONTROL       341
#define TYRO_KEY_LEFT_ALT           342
#define TYRO_KEY_LEFT_SUPER         343
#define TYRO_KEY_RIGHT_SHIFT        344
#define TYRO_KEY_RIGHT_CONTROL      345
#define TYRO_KEY_RIGHT_ALT          346
#define TYRO_KEY_RIGHT_SUPER        347
#define TYRO_KEY_MENU               348

//modifiers
#define TYRO_MOD_NONE            0x0000
#define TYRO_MOD_SHIFT           0x0001
#define TYRO_MOD_CONTROL         0x0002
#define TYRO_MOD_ALT             0x0004
#define TYRO_MOD_SUPER           0x0008

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
        
        Window();
        
        ~Window();

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
        std::function<bool(Window& window, unsigned int w, unsigned int h)> callback_window_resize;

        bool mouse_down(MouseButton button, int modifier);
        bool mouse_up(MouseButton button, int modifier);
        bool mouse_move(int mouse_x, int mouse_y);
        bool mouse_scroll(float delta_y);
        bool key_pressed(unsigned int unicode_key, int modifiers);
        bool key_down(int key, int modifiers);
        bool key_up(int key, int modifiers);
        bool window_resize(unsigned w, unsigned int h);
        
    private:
        GLFWwindow* m_glfw_window;
        ES2Context* m_gl_context;
    };


}