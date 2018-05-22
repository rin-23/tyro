//////////////////////////////////////////////////////////////////////////////
// Modified from "Video Texture" code
// Copyright (C) 2009  Arsalan Malik (arsalank2@hotmail.com)
//                                                                            
// On Mac OS X, compile with:
// g++ -o VideoTexture VideoTexture.cpp -framework OpenGL -framework Glut -I
// /usr/local/include/opencv/ $(pkg-config --libs opencv)
//////////////////////////////////////////////////////////////////////////////


// Open CV includes
#include cv;
#include &lt;highgui.h&gt;

// Standard includes
#include &lt;stdio.h&gt;
#include &lt;string.h&gt;
#include &lt;assert.h&gt;

// OpenGL/Glut includes
#ifdef __APPLE__
#  include &lt;GLUT/glut.h&gt;
#else
#  include &lt;GL/glut.h&gt;
#endif

// Timing includes
#include &lt;sys/time.h&gt;

#define KEY_ESCAPE 27


CvCapture* g_Capture;
GLint g_hWindow;

// Frame size
int frame_width  = 640;
int frame_height = 480;

// current frames per second, slightly smoothed over time
double fps;
// show mirror image
bool mirror = true;

// Return current time in seconds
double current_time_in_seconds();
// Initialize glut window
GLvoid init_glut();
// Glut display callback, draws a single rectangle using video buffer as
// texture
GLvoid display();
// Glut reshape callback
GLvoid reshape(GLint w, GLint h);
// Glut keyboard callback
GLvoid key_press (unsigned char key, GLint x, GLint y);
// Glut idle callback, fetches next video frame
GLvoid idle();

double current_time_in_seconds()
{
  timeval timer;
  gettimeofday(&timer,NULL);
  double seconds = 1e-6 * timer.tv_usec + timer.tv_sec;
  return seconds;
}


GLvoid init_glut()
{  

  glClearColor (0.0, 0.0, 0.0, 0.0);

  // Set up callbacks
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(key_press);
  glutIdleFunc(idle);
}

GLvoid display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  // These are necessary if using glTexImage2D instead of gluBuild2DMipmaps
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  // Set Projection Matrix
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, frame_width, frame_height, 0);

  // Switch to Model View Matrix
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Draw a textured quad
  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
  glTexCoord2f(1.0f, 0.0f); glVertex2f(frame_width, 0.0f);
  glTexCoord2f(1.0f, 1.0f); glVertex2f(frame_width, frame_height);
  glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, frame_height);
  glEnd();

  glFlush();
  glutSwapBuffers();
}


GLvoid reshape(GLint w, GLint h)
{
  glViewport(0, 0, w, h);
}

GLvoid key_press(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 'f':
      printf("fps: %g\n",fps);
      break;
    case 'm':
      mirror = !mirror;
      break;
    case KEY_ESCAPE:
      cvReleaseCapture(&g_Capture);
      glutDestroyWindow(g_hWindow);
      exit(0);
      break;
  }
  glutPostRedisplay();
}


GLvoid idle()
{
  // start timer
  double start_seconds = current_time_in_seconds();

  // Capture next frame, this will almost always be the limiting factor in the
  // framerate, my webcam only gets ~15 fps
  IplImage * image = cvQueryFrame(g_Capture);

  // Of course there are faster ways to do this with just opengl but this is to
  // demonstrate filtering the video before making the texture
  if(mirror)
  {
    cvFlip(image, NULL, 1);
  }

  // Image is memory aligned which means we there may be extra space at the end
  // of each row. gluBuild2DMipmaps needs contiguous data, so we buffer it here
  char * buffer = new char[image-&gt;width*image-&gt;height*image-&gt;nChannels];
  int step     = image-&gt;widthStep;
  int height   = image-&gt;height;
  int width    = image-&gt;width;
  int channels = image-&gt;nChannels;
  char * data  = (char *)image-&gt;imageData;
  // memcpy version below seems slightly faster
  //for(int i=0;i&lt;height;i++)
  //for(int j=0;j&lt;width;j++)
  //for(int k=0;k&lt;channels;k++)
  //{
  //  buffer[i*width*channels+j*channels+k] = data[i*step+j*channels+k];
  //}
  for(int i=0;i&lt;height;i++)
  {
    memcpy(&buffer[i*width*channels],&(data[i*step]),width*channels);
  }

  // Create Texture
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    image-&gt;width,
    image-&gt;height,
    0,
    GL_BGR,
    GL_UNSIGNED_BYTE,
    buffer);


  // Clean up buffer
  delete[] buffer;

  // Update display
  glutPostRedisplay();

  double stop_seconds = current_time_in_seconds();
  fps = 0.9*fps + 0.1*1.0/(stop_seconds-start_seconds);
}

int main(int argc, char* argv[])
{

  // Create OpenCV camera capture
  // If multiple cameras are installed, this takes "first" one
  g_Capture = cvCaptureFromCAM(0);
  assert(g_Capture);
  // capture properties
  frame_height = (int)cvGetCaptureProperty(g_Capture, CV_CAP_PROP_FRAME_HEIGHT);
  frame_width  = (int)cvGetCaptureProperty(g_Capture, CV_CAP_PROP_FRAME_WIDTH);

  // Create GLUT Window
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(frame_width, frame_height);

  g_hWindow = glutCreateWindow("Video Texture");

  // Initialize OpenGL
  init_glut();

  glutMainLoop();

  return 0;
}
</code></pre>
<p>Which on my mac I compile with:</p>
<pre><code>
g++ -o VideoTexture VideoTexture.cpp -framework OpenGL -framework Glut -I /usr/local/include/opencv/ $(pkg-config --libs opencv) -Os
</code></pre>
<p>Notice the little bit that grabs all of the opencv libraries for me:</p>
<pre><code>
pkg-config --libs opencv
</code></pre>
<p><strong>Update:</strong> <a href="http://www.willpatera.com">Will Patera</a> has generously allowed me to post his python port of the above code. To run it you&#8217;ll need the python bindings for openGL and openCV and the numpy library.</p>
<pre style="height:200px"><code>
import cv
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
import numpy as np
import sys


#window dimensions
width = 1280
height = 720
nRange = 1.0

global capture
capture = None

def cv2array(im): 
  depth2dtype = { 
    cv.IPL_DEPTH_8U: 'uint8', 
    cv.IPL_DEPTH_8S: 'int8', 
    cv.IPL_DEPTH_16U: 'uint16', 
    cv.IPL_DEPTH_16S: 'int16', 
    cv.IPL_DEPTH_32S: 'int32', 
    cv.IPL_DEPTH_32F: 'float32', 
    cv.IPL_DEPTH_64F: 'float64', 
    } 

  arrdtype=im.depth 
  a = np.fromstring( 
     im.tostring(), 
     dtype=depth2dtype[im.depth], 
     count=im.width*im.height*im.nChannels) 
  a.shape = (im.height,im.width,im.nChannels) 
  return a

def init():
  #glclearcolor (r, g, b, alpha)
  glClearColor(0.0, 0.0, 0.0, 1.0)
  
  glutDisplayFunc(display)
  glutReshapeFunc(reshape)
  glutKeyboardFunc(keyboard)
  glutIdleFunc(idle)  
  
def idle():
  #capture next frame
  
  global capture
  image = cv.QueryFrame(capture)
  image_size = cv.GetSize(image)
  cv.Flip(image, None, 0)
  cv.Flip(image, None, 1)
  cv.CvtColor(image, image, cv.CV_BGR2RGB)
  #you must convert the image to array for glTexImage2D to work
  #maybe there is a faster way that I don't know about yet...
  image_arr = cv2array(image)
  #print image_arr

  
  # Create Texture
  glTexImage2D(GL_TEXTURE_2D, 
    0, 
    GL_RGB, 
    image_size[0], 
    image_size[1], 
    0,
    GL_RGB, 
    GL_UNSIGNED_BYTE, 
    image_arr)
  
  glutPostRedisplay()

def display():
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
  glEnable(GL_TEXTURE_2D)
  #glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
  #glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
  #glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL)
  #this one is necessary with texture2d for some reason
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
  
  # Set Projection Matrix
  glMatrixMode(GL_PROJECTION)
  glLoadIdentity()
  gluOrtho2D(0, width, 0, height)
  
  # Switch to Model View Matrix
  glMatrixMode(GL_MODELVIEW)
  glLoadIdentity()
  
  # Draw textured Quads
  glBegin(GL_QUADS)
  glTexCoord2f(0.0, 0.0)
  glVertex2f(0.0, 0.0)
  glTexCoord2f(1.0, 0.0)
  glVertex2f(width, 0.0)
  glTexCoord2f(1.0, 1.0)
  glVertex2f(width, height)
  glTexCoord2f(0.0, 1.0)
  glVertex2f(0.0, height)
  glEnd()
    
  glFlush()
  glutSwapBuffers()
  
def reshape(w, h):
  if h == 0:
    h = 1
  
  glViewport(0, 0, w, h)
  glMatrixMode(GL_PROJECTION)
  
  glLoadIdentity()
  # allows for reshaping the window without distoring shape
  
  if w &lt;= h:
    glOrtho(-nRange, nRange, -nRange*h/w, nRange*h/w, -nRange, nRange)
  else:
    glOrtho(-nRange*w/h, nRange*w/h, -nRange, nRange, -nRange, nRange)
  
  glMatrixMode(GL_MODELVIEW)
  glLoadIdentity()
  
def keyboard(key, x, y):
  global anim
  if key == chr(27):
    sys.exit()
  
def main():
  global capture
  #start openCV capturefromCAM
  capture = cv.CaptureFromCAM(0)
  print capture
  cv.SetCaptureProperty(capture, cv.CV_CAP_PROP_FRAME_WIDTH, width)
  cv.SetCaptureProperty(capture, cv.CV_CAP_PROP_FRAME_HEIGHT, height)
  
  glutInit(sys.argv)
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH)
  glutInitWindowSize(width, height)
  glutInitWindowPosition(100, 100)
  glutCreateWindow("OpenGL + OpenCV")
  
  init()
  glutMainLoop()
  
main()
