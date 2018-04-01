#include "RAES2Context.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace tyro 
{
ES2Context::ES2Context(GLFWwindow* window)
:
mWindow(window)
{}

void ES2Context::setCurrent() 
{
	glfwMakeContextCurrent(mWindow);
}

void ES2Context::endCurrent() 
{
	glfwMakeContextCurrent(NULL);
}

void ES2Context::swapBuffers() 
{
	glfwSwapBuffers(mWindow);
}

void ES2Context::getFramebufferSize(int* width, int* height) 
{
	glfwGetFramebufferSize(mWindow, width, height);
}
}