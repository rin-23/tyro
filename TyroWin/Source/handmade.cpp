
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

#include "RAEnginePrerequisites.h"

#include "Wm5Vector3.h"
#include "Wm5APoint.h"
#include "RAES2Context.h"
#include "RAES2Renderer.h"
#include "RAVisibleSet.h"
#include "RAES2StandardMesh.h"
#include "RACamera.h"

using namespace RAEngine;
using namespace Wm5;

void window_close_callback(GLFWwindow* window);

int main(void)
{

    GLFWwindow* window;
	
	Vector3f vec = Vector3f(9, 9, 9);

    /* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
	glfwSetWindowCloseCallback(window, window_close_callback);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	ES2Context* glContext = new ES2Context(window);
	ES2Renderer* glRend = new ES2Renderer(glContext);
		
    //setup objects to draw
	// Need to add scene first otherwise no Worldbound
    ES2SphereSPtr sphere = ES2Sphere::Create(10, 10, 10);
	sphere->Update(true);
	VisibleSet* vsSet = new VisibleSet();
	vsSet->Insert(sphere.get());
    
    //setup camera
    APoint worldCenter = sphere->WorldBoundBox.GetCenter();
	float radius = sphere->WorldBoundBox.GetRadius()*2;
	float aspect = 1.0;
    
	int mViewWidth, mViewHeight;
    glContext->getFramebufferSize(&mViewWidth, &mViewHeight);
    
	Vector4i viewport(0, 0, mViewWidth, mViewHeight);
    APoint mInitialPosition = worldCenter;
	mInitialPosition.Z() += radius;
    
    Camera* camera = new Camera(mInitialPosition, worldCenter, viewport, aspect, true);
    
	/* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        //glClear(GL_COLOR_BUFFER_BIT);
		//glClearColor(1.0, 0.0, 0.0,  1.0);
        /* Swap front and back buffers */
        //glfwSwapBuffers(window);
		
		glRend->RenderVisibleSet(vsSet, camera);

        /* Poll for and process events */
        glfwPollEvents();
    }

	//delete glRenderer;
	//delete glContext;
    glfwTerminate();
    return 0;
}

void window_close_callback(GLFWwindow* window)
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}


