#include "TyroWindow.h"

//#define GLFW_DLL


namespace tyro
{

void window_close_callback(GLFWwindow* window)
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int Window::Init()
{
    /* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    /* Create a windowed mode window and its OpenGL context */
    m_glfw_window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);
    if (!m_glfw_window)
    {
        glfwTerminate();
        return -1;
    }

     /* Make the window's context current */
    glfwMakeContextCurrent(m_glfw_window);
	glfwSetWindowCloseCallback(m_glfw_window, window_close_callback);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	m_gl_context = new ES2Context(m_glfw_window);

    return 0;
}

int Window::Terminate()
{
    delete m_gl_context;
    glfwTerminate();    
}

}
