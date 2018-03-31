

#include <stdlib.h>
#include <stdio.h>

#include "RAEnginePrerequisites.h"
#include "Wm5Vector3.h"
#include "Wm5APoint.h"
#include "RAES2Renderer.h"
#include "RAVisibleSet.h"
#include "RAES2StandardMesh.h"
#include "RAiOSCamera.h"
#include "TyroWindow.h"

using namespace RAEngine;
using namespace Wm5;

void window_close_callback(GLFWwindow* window);

int main(void) 
{	
	RAEngine::Window tyro_window;
	tyro_window.Init();

	/*
    GLFWwindow* window;
	
	Vector3f vec = Vector3f(9, 9, 9);

    // Initialize the library 
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    // Create a windowed mode window and its OpenGL context 
    window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
	

    // Make the window's context current 
    glfwMakeContextCurrent(window);
	glfwSetWindowCloseCallback(window, window_close_callback);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	ES2Context* glContext = new ES2Context(window);
	*/

	ES2Renderer* glRend = new ES2Renderer(tyro_window.GetGLContext());
	glRend->SetClearColor(Vector4f(0.0, 1.0, 0.0, 1.0));
    //setup objects to draw
	//Need to add scene first otherwise no Worldbound
    ES2SphereSPtr object = ES2Sphere::Create(100, 100, 100);
	
	//Wm5::Vector3f points[2] = { Wm5::Vector3f(0,0,0), Wm5::Vector3f(1,0,0) };
	//ES2LineSPtr object = ES2Line::Create(points, 2, true);
	object->Update(true);
	VisibleSet* vsSet = new VisibleSet();
	vsSet->Insert(object.get());
    
    //setup camera
    APoint worldCenter = object->WorldBoundBox.GetCenter();
	float radius = std::abs(object->WorldBoundBox.GetRadius()*2);
    
	int mViewWidth, mViewHeight;
    tyro_window.GetGLContext()->getFramebufferSize(&mViewWidth, &mViewHeight);
	float aspect = 1.0;
	Vector4i viewport(0, 0, mViewWidth, mViewHeight);

	iOSCamera* camera = new iOSCamera(worldCenter, radius, aspect, 2, viewport, true);
    
	/* Loop until the user closes the window */
    while (!tyro_window.ShouldClose())
    {
        /* Render here */
		glRend->RenderVisibleSet(vsSet, camera);

        /* Poll for and process events */
        tyro_window.ProcessUserEvents();
	}

	object = nullptr;
	delete camera;
	delete vsSet;
	delete glRend;
	
	tyro_window.Terminate();
    return 0;
}



