#include "TyroWindow.h"
#include "RAEnginePrerequisites.h"

static double highdpi = 1;

/*
 * C-style callbacks for GLFW
 */
void glfw_mouse_press(GLFWwindow* window, int button, int action, int modifier)
{
    tyro::Window::MouseButton mb;
    tyro::Window* tyro_window = nullptr;
    tyro_window = static_cast<tyro::Window*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_1)
        mb = tyro::Window::MouseButton::MB_Left;
    else if (button == GLFW_MOUSE_BUTTON_2)
        mb = tyro::Window::MouseButton::MB_Right;
    else //if (button == GLFW_MOUSE_BUTTON_3)
        mb = tyro::Window::MouseButton::MB_Middle;
    
    if (action == GLFW_PRESS)
        tyro_window->mouse_down(mb, modifier);
    else
        tyro_window->mouse_up(mb, modifier);

}

void glfw_error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

void glfw_char_mods_callback(GLFWwindow* window, unsigned int codepoint, int modifier)
{   
    tyro::Window* tyro_window = nullptr;
    tyro_window = static_cast<tyro::Window*>(glfwGetWindowUserPointer(window));
    tyro_window->key_pressed(codepoint, modifier);
}

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int modifier)
{
    //if (key == /*
    //C-style callbacks for GLFW
    //GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    //  glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_UNKNOWN) 
        return;
    
    tyro::Window* tyro_window = nullptr;
    tyro_window = static_cast<tyro::Window*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS)
        tyro_window->key_down(key, modifier);
    else if(action == GLFW_RELEASE)
        tyro_window->key_up(key, modifier);
}

void glfw_window_size(GLFWwindow* window, int width, int height)
{   
    int w = width;
    int h = height;
    tyro::Window* tyro_window = nullptr;
    tyro_window = static_cast<tyro::Window*>(glfwGetWindowUserPointer(window));
    tyro_window->window_resize(highdpi*w, highdpi*h);
}

void glfw_mouse_move(GLFWwindow* window, double x, double y)
{   
    tyro::Window* tyro_window = nullptr;
    tyro_window = static_cast<tyro::Window*>(glfwGetWindowUserPointer(window));
    tyro_window->mouse_move(highdpi*x, highdpi*y);
}

void glfw_mouse_scroll(GLFWwindow* window, double x, double y)
{
    //scroll_x += x;
    //scroll_y += y;
    tyro::Window* tyro_window = nullptr;
    tyro_window = static_cast<tyro::Window*>(glfwGetWindowUserPointer(window));
    tyro_window->mouse_scroll(y);
}

void glfw_drop_callback(GLFWwindow *window,int count,const char **filenames)
{
}

void glfw_window_close_callback(GLFWwindow* window)
{
    //glfwSetWindowShouldClose(window, GLFW_TRUE);
}

namespace tyro
{

Window::Window()
{
    // C-style callbacks
    callback_init           = nullptr;
    callback_pre_draw       = nullptr;
    callback_post_draw      = nullptr;
    callback_mouse_down     = nullptr;
    callback_mouse_up       = nullptr;
    callback_mouse_move     = nullptr;
    callback_mouse_scroll   = nullptr;
    callback_key_down       = nullptr;
    callback_key_up         = nullptr;
    callback_window_resize  = nullptr;
}

Window::~Window()
{
}

int Window::InitOffscreen(int w, int h) 
{   
     /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    #ifdef __APPLE__
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      
      //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
      //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    #endif
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    /* Create a windowed mode window and its OpenGL context */
    m_glfw_window = glfwCreateWindow(w, h, "tyro", NULL, NULL);
    
    if (!m_glfw_window)
    {
        glfwTerminate();
        return -1;
    }

     /* Make the window's context current */
    glfwMakeContextCurrent(m_glfw_window);

	// start GLEW extension handler
	glewExperimental = true;
	GLenum err = glewInit();
    if (GLEW_OK != err)
        RA_LOG_ERROR_ASSERT("Failed to initialize GLEW %s\n", glewGetErrorString(err));
    
    if (GLEW_VERSION_4_4) 
        RA_LOG_INFO("Yay! OpenGL 4.4 is supported!");
    
    // create opengl context
    // @TODO: make genetal opengl context
	m_gl_context = new ES2Context(m_glfw_window);

    //@TODO seems to be a bug with glew
    GL_CHECK_ERROR_GLEW_HACK;

    return 0;
}

int Window::Init(int w, int h)
{
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    #ifdef __APPLE__
    //   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
      glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
      //glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
      //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    #endif
    
    

    /* Create a windowed mode window and its OpenGL context */
    m_glfw_window = glfwCreateWindow(w, h, "tyro", NULL, NULL);
    //m_glfw_window = glfwCreateWindow(1200, 1200, "Hello World", NULL, NULL);
    
    if (!m_glfw_window)
    {
        glfwTerminate();
        return -1;
    }

     /* Make the window's context current */
    glfwMakeContextCurrent(m_glfw_window);

	// start GLEW extension handler
	glewExperimental = true;
	GLenum err = glewInit();
    if (GLEW_OK != err)
        RA_LOG_ERROR_ASSERT("Failed to initialize GLEW %s\n", glewGetErrorString(err));
    
    
    if (GLEW_VERSION_4_1) 
        RA_LOG_INFO("Yay! OpenGL 4.1 is supported!");

    if (GLEW_VERSION_4_3) 
        RA_LOG_INFO("Yay! OpenGL 4.3 is supported!");
    
    // create opengl context
    // @TODO: make genetal opengl context
	m_gl_context = new ES2Context(m_glfw_window);

    // allows to avoid static classes
    glfwSetWindowUserPointer(m_glfw_window, this);

    // Register callbacks
    glfwSetWindowCloseCallback(m_glfw_window, glfw_window_close_callback);
    glfwSetKeyCallback(m_glfw_window, glfw_key_callback);
    glfwSetCursorPosCallback(m_glfw_window, glfw_mouse_move);
    glfwSetWindowSizeCallback(m_glfw_window, glfw_window_size);
    glfwSetMouseButtonCallback(m_glfw_window, glfw_mouse_press);
    glfwSetScrollCallback(m_glfw_window, glfw_mouse_scroll);
    glfwSetCharModsCallback(m_glfw_window, glfw_char_mods_callback);
    glfwSetDropCallback(m_glfw_window, glfw_drop_callback);
    

    // Handle retina displays (windows and mac)
    int width, height;
    glfwGetFramebufferSize(m_glfw_window, &width, &height);
    int width_window, height_window;
    glfwGetWindowSize(m_glfw_window, &width_window, &height_window);
    highdpi = width/width_window;
    //glfw_window_size(window,width_window,height_window);
    int w1 = width_window*highdpi;
    int h1 = height_window*highdpi;

    RA_LOG_INFO("viewport %i %i %i %i %i %i", width, height, width_window, height_window, w1, h1);
   // core.viewport = Eigen::Vector4f(0,0,w,h);

    //@TODO seems to be a bug with glew
    GL_CHECK_ERROR_GLEW_HACK;

    return 0;
}

int Window::Terminate()
{
    delete m_gl_context;
    glfwTerminate();    
}


//@TODO move all of this inside glfw methods

int Window::JoystickConnected() 
{
    int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
    return present;
}

int Window::JoystickAxes(std::map<std::string, double>& axes_map) 
{   
    if (!JoystickConnected()) 
    {
        RA_LOG_ERROR_ASSERT("Gamepad not found");
        return 0;
    }
    
    int axesCount;
    const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
    
    //todo differentite between pos and neg values

    axes_map["L_S_X_RIGHT"] = axes[0]>0 ? axes[0] : 0;
    axes_map["L_S_X_LEFT"] = axes[0]<0 ? -1*axes[0] : 0;
    axes_map["L_S_Y_DOWN"] = axes[1]>0 ? axes[1] : 0;
    axes_map["L_S_Y_UP"] = axes[1]<0 ?-1*axes[1] : 0;
    axes_map["R_S_X_RIGHT"] = axes[3]>0 ? axes[3] : 0;
    axes_map["R_S_X_LEFT"] = axes[3]<0 ? -1*axes[3] : 0;
    axes_map["R_S_Y_DOWN"] = axes[4]>0 ? axes[4] : 0;
    axes_map["R_S_Y_UP"] = axes[4]<0 ? -1*axes[4] : 0;
    axes_map["L_T"] = (axes[2] + 1.0)/2.0;
    axes_map["R_T"] = (axes[5] + 1.0)/2.0;

    if (true) 
    {
        RA_LOG_INFO("%i axes found", axesCount);
        std::cout << "Left Stick X Axis: "  << axes[0] << std::endl; 
        std::cout << "Left Stick Y Axis: "  << axes[1] << std::endl; 
        std::cout << "Right Stick X Axis: " << axes[3] << std::endl; 
        std::cout << "Right Stick Y Axis: " << axes[4] << std::endl; 
        std::cout << "Left Trigger/L2: "    << axes[2] << std::endl; 
        std::cout << "Right Trigger/R2: "   << axes[5] << std::endl; 

        std::cout << "L_S_X_RIGHT: "   << axes_map["L_S_X_RIGHT"] << std::endl; 
        std::cout << "L_S_X_LEFT: "   << axes_map["L_S_X_LEFT"] << std::endl; 
        std::cout << "L_S_Y_DOWN: "   << axes_map["L_S_Y_DOWN"] << std::endl; 
        std::cout << "L_S_Y_UP: "   << axes_map["L_S_Y_UP"] << std::endl; 
        std::cout << "R_S_X_RIGHT: "   << axes_map["R_S_X_RIGHT"] << std::endl; 
        std::cout << "R_S_X_LEFT: "   << axes_map["R_S_X_LEFT"] << std::endl; 
        std::cout << "R_S_Y_DOWN: "   << axes_map["R_S_Y_DOWN"] << std::endl; 
        std::cout << "R_S_Y_UP: "   << axes_map["R_S_Y_UP"] << std::endl; 
        std::cout << "L_T: "   << axes_map["L_T"] << std::endl; 
        std::cout << "R_T: "   << axes_map["R_T"] << std::endl; 
    }

    return 1;
}

int Window::JoystickButtons(std::map<std::string, bool>& buttons_map)
{   
    if (!JoystickConnected()) 
    {
        RA_LOG_ERROR_ASSERT("Gamepad not found");
        return 0;
    }

    int buttonCount;
    const unsigned char *buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
    
    buttons_map["triangle"] = buttons[2];
    buttons_map["square"] = buttons[3];
    buttons_map["circle"] = buttons[1];
    buttons_map["cross"] = buttons[0];
    buttons_map["R1"] = buttons[5];
    buttons_map["L1"] = buttons[4];
    buttons_map["R3"] = buttons[12];
    buttons_map["L3"] = buttons[11];
    buttons_map["options"] = buttons[7];
    buttons_map["share"] = buttons[7];

    
    for (int i=0;i<buttonCount;++i) 
    {      
        //if (GLFW_PRESS == buttons[i])
        //    std::cout << "Button " << i << "Pressed" << std::endl;
        //else if (GLFW_RELEASE == buttons[i] )
        //    std::cout << "Button " << i << "Released" << std::endl;
    }
    return 1;
    
}



bool Window::mouse_down(MouseButton button, int modifier)
{   
    //printf("mouse down pressed\n");
    if (callback_mouse_down)
        if (callback_mouse_down(*this,static_cast<int>(button), modifier))
            return true;
    
    return true;
}

bool Window::mouse_up(MouseButton button, int modifier)
{   
    //printf("mouse up pressed\n");

    if (callback_mouse_up)
        if (callback_mouse_up(*this,static_cast<int>(button),modifier))
            return true;

    return true;
}

bool Window::mouse_move(int mouse_x, int mouse_y)
{
    if (callback_mouse_move)
        if (callback_mouse_move(*this, mouse_x, mouse_y))
            return true;

    return true;
}

bool Window::mouse_scroll(float delta_y)
{
    if (callback_mouse_scroll)
        if (callback_mouse_scroll(*this,delta_y))
            return true;
    
    return true;
}

bool Window::key_pressed(unsigned int unicode_key, int modifiers)
{
    if (callback_key_pressed)
        if (callback_key_pressed(*this, unicode_key,modifiers))
            return true;

    return false;
  }

bool Window::key_down(int key, int modifiers)
{
    if (callback_key_down)
        if (callback_key_down(*this,key,modifiers))
            return true;

    return false;
}

bool Window::key_up(int key, int modifiers)
{
    if (callback_key_up)
        if (callback_key_up(*this,key,modifiers))
            return true;

    return false;
}

bool Window::window_resize(unsigned int w, unsigned int h)
{
    if (callback_window_resize)
        if (callback_window_resize(*this, w, h))
            return true;

    return false;
}

double Window::TimeNow() 
{
    return glfwGetTime();
}

}
