
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

#include "Wm5Mathematics.h"

void window_close_callback(GLFWwindow* window);

int main() {
	// start GL context and O/S window using the GLFW helper library
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return 1;
	}

	// uncomment these lines if on Apple OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);


	GLFWwindow* window = glfwCreateWindow(640, 480, "Hello Triangle", NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

						  /* OTHER STUFF GOES HERE NEXT */
	float points[] = 
    {
		0.0f,  0.5f,  0.0f,
		0.5f, -0.5f,  0.0f,
		-0.5f, -0.5f,  0.0f
	};

	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);


	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	const char* vertex_shader =
		"#version 400\n"
		"in vec3 vp;"
		"void main() {"
		"  gl_Position = vec4(vp, 1.0);"
		"}";

	const char* fragment_shader =
		"#version 400\n"
		"out vec4 frag_colour;"
		"void main() {"
		"  frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
		"}";

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);
	
	GLuint shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);

	while (!glfwWindowShouldClose(window)) {
		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_programme);
		glBindVertexArray(vao);
		// draw points 0-3 from the currently bound VAO with current in-use shader
		glDrawArrays(GL_TRIANGLES, 0, 3);
		// update other events like input handling 
		glfwPollEvents();
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

/*
int asdasdmain(void)
{

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
	ES2Renderer* glRend = new ES2Renderer(glContext);
	glRend->SetClearColor(Vector4f(0.0, 1.0, 0.0, 1.0));
    //setup objects to draw
	// Need to add scene first otherwise no Worldbound
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
    glContext->getFramebufferSize(&mViewWidth, &mViewHeight);
	float aspect = 1.0;
	Vector4i viewport(0, 0, mViewWidth, mViewHeight);

	iOSCamera* camera = new iOSCamera(worldCenter, radius, aspect, 2, viewport, true);
    
	// Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        //Render here
			
		glRend->RenderVisibleSet(vsSet, camera);

        //Poll for and process events 
        glfwPollEvents();
		
		//Swap front and back buffers 
		//glfwSwapBuffers(window);
	}

	object = nullptr;
	delete camera;
	delete vsSet;
	delete glRend;
	delete glContext;

	glfwTerminate();
    return 0;
}
*/

void window_close_callback(GLFWwindow* window)
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}


