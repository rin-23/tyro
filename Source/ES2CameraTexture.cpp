
#include "ES2CameraTexture.h"
#include "RAES2CoreVisualEffects.h"
#include "GLStructures.h"
#include "RACamera.h"

namespace tyro
{

void ES2CameraTexture::Init(int width, 
                           int height, 
                           Texture::TextureFormat texformat)
{
    ES2Renderable::Init(PT_TRIANGLES);
    mAllocSize = 0;
    mWidth = width;
    mHeight = height;
    mTexFormat = texformat;
    mCurrentFrame = 0;
    
    SetVisualEffect(ES2CoreVisualEffects::VideoTexture());
    
    ES2Texture2DSPtr texture = std::make_shared<ES2Texture2D>(mTexFormat, mWidth, mHeight);
    void* data = nullptr; //TODO load data here;
    texture->LoadData(data);

    GetVisualEffect()->SetTexture2D(texture);
    _UpdateGeometry();
}


void ES2CameraTexture::Init()
{
    using namespace cv;
    // Create a VideoCapture object and open the input file
    capture = new VideoCapture(0); 
        
    // Check if camera opened successfully
    if(!capture->isOpened())
    {
        RA_LOG_ERROR_ASSERT("Failed to open video capture");
        return;
    }
    
    int width = capture->get(CV_CAP_PROP_FRAME_WIDTH);
    int height = capture->get(CV_CAP_PROP_FRAME_HEIGHT);
    //int num_frames = capture->get(CV_CAP_PROP_FRAME_COUNT);
    //int frame_rat = capture->get(CV_CAP_PROP_FPS);
    //RA_LOG_INFO("Frame Rate %i", frame_rat);
    capture->set(CV_CAP_PROP_CONVERT_RGB, 1);

    this->Init(width, height, Texture::TextureFormat::TF_R8G8B8);
}

ES2CameraTextureSPtr ES2CameraTexture::Create() 
{
    ES2CameraTextureSPtr ptr = std::make_shared<ES2CameraTexture>();
    ptr->Init(); 
    return ptr;   
}

void ES2CameraTexture::showFrame() 
{
    if (capture == nullptr) 
    { 
        RA_LOG_ERROR_ASSERT("Capture is null")
    }

    // Capture frame-by-frame
    cv::Mat frame_image_grb;
    (*capture) >> frame_image_grb;

    //If the frame is empty, break immediately
    if (frame_image_grb.empty()) 
    {
        RA_LOG_ERROR_ASSERT("frame data is empty");
        return;
    }

    cv::Mat frame_image_rgb;
    cv::Mat frame_image;
    
    cv::cvtColor(frame_image_grb, frame_image_rgb, CV_BGR2RGB);
    cv::flip(frame_image_rgb, frame_image, -1);

    GetVisualEffect()->GetTexture2D()->LoadSubData(0, 0, mWidth, mHeight, frame_image.data);
    
}


void ES2CameraTexture::UpdateUniformsWithCamera(const Camera* camera)
{
    int uSampler = 0;
    GetVisualEffect()->GetUniforms()->UpdateIntUniform(0, &uSampler);
}


/*
GLfloat vertices[] = {-1, -1, 0, // bottom left corner
                      -1,  1, 0, // top left corner
                       1,  1, 0, // top right corner
                       1, -1, 0}; // bottom right corner

GLubyte indices[] = {0,1,2, // first triangle (bottom left - top left - top right)
                     0,2,3}; // second triangle (bottom left - top right - bottom right)

glVertexPointer(3, GL_FLOAT, 0, vertices);
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, indices);
*/

void ES2CameraTexture::_UpdateGeometry()
{   
    using Wm5::Vector3f;
    using Wm5::Vector2f;

    int numOfVerticies = 6; //6 verticies to draw quad
    float offset = 0.75;
    float xoffset = 0.05;
    float k = 0; //720/1280
    VertexTexture coords[] = 
    {   
        //VertexTexture(Vector3f(0.0f,  0.5f,  0.0f),  Vector2f(0, 0)),
        //VertexTexture(Vector3f(-0.5f, -0.5f,  0.0f),  Vector2f(0, 0)),
        //VertexTexture(Vector3f(0.5f, -0.5f,  0.0f),  Vector2f(0, 0))
        
        
        VertexTexture(Vector3f( (-1 + xoffset), -1+offset, 0),  Vector2f(0, 0)),
        VertexTexture(Vector3f( k+(-0.5 + xoffset), -1+offset, 0),  Vector2f(1, 0)),
        VertexTexture(Vector3f( k+(-0.5  + xoffset),  -0.5+offset, 0),  Vector2f(1, 1)),

        VertexTexture(Vector3f((-1 + xoffset), -1+offset, 0),  Vector2f(0, 0)),
        VertexTexture(Vector3f( k+(-0.5  + xoffset), -0.5+offset, 0),  Vector2f(1, 1)),
        VertexTexture(Vector3f((-1 + xoffset), -0.5+offset, 0),  Vector2f(0, 1))
    };

    SetVertexBuffer(std::make_shared<ES2VertexHardwareBuffer>(sizeof(VertexTexture), numOfVerticies, coords, HardwareBuffer::BU_STATIC));
    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), this->GetVertexBuffer());
    SetVertexArray(varray);
}

}