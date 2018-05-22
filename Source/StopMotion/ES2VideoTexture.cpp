
#include "ES2VideoTexture.h"
#include "RAES2CoreVisualEffects.h"
#include "GLStructures.h"
#include "RACamera.h"

namespace tyro
{

void ES2VideoTexture::Init(int width, 
                           int height, 
                           int num_frames, 
                           Texture::TextureFormat texformat)
{
    ES2Renderable::Init(PT_TRIANGLES);
    mAllocSize = 0;
    mWidth = width;
    mHeight = height;
    mTexFormat = texformat;
    mNumFrames = num_frames;
    mCurrentFrame = 0;
    
    SetVisualEffect(ES2CoreVisualEffects::VideoTexture());
    
    ES2Texture2DSPtr texture = std::make_shared<ES2Texture2D>(mTexFormat, mWidth, mHeight);
    void* data = nullptr; //TODO load data here;
    texture->LoadData(data);

    GetVisualEffect()->SetTexture2D(texture);
    _UpdateGeometry();
}

void ES2VideoTexture::Init(const std::string& video_path)
{
    using namespace cv;
    // Create a VideoCapture object and open the input file
    capture = new VideoCapture(video_path); 
        
    // Check if camera opened successfully
    if(!capture->isOpened())
    {
        RA_LOG_ERROR_ASSERT("Failed to open video capture");
        return;
    }
    
    int width = capture->get(CV_CAP_PROP_FRAME_WIDTH);
    int height = capture->get(CV_CAP_PROP_FRAME_HEIGHT);
    int num_frames = capture->get(CV_CAP_PROP_FRAME_COUNT);
    int frame_rat = capture->get(CV_CAP_PROP_FPS);
    RA_LOG_INFO("Frame Rate %i", frame_rat);
    capture->set(CV_CAP_PROP_CONVERT_RGB, 1);

    this->Init(width, height, num_frames, Texture::TextureFormat::TF_R8G8B8);
}

ES2VideoTextureSPtr ES2VideoTexture::Create(const std::string& video_path) 
{
    ES2VideoTextureSPtr ptr = std::make_shared<ES2VideoTexture>();
    ptr->Init(video_path); 
    return ptr;   
}

ES2VideoTextureSPtr ES2VideoTexture::Create(int width, 
                                            int height, 
                                            int num_frames, 
                                            Texture::TextureFormat texformat) 
{
    ES2VideoTextureSPtr ptr = std::make_shared<ES2VideoTexture>();
    ptr->Init(width, height, num_frames, texformat);
    return ptr;
}

void ES2VideoTexture::showFrame(int frame) 
{
    assert(capture != nullptr);
    assert(frame < mNumFrames);

    if (frame < mNumFrames && capture != nullptr) 
    {   
        capture->set(CV_CAP_PROP_POS_FRAMES, frame);
        cv::Mat frame_image_grb;
        // Capture frame-by-frame
        capture->read(frame_image_grb);
        //RA_LOG_INFO("frame %i milliseconds %f", frame, capture->get(CV_CAP_PROP_POS_MSEC));

        cv::Mat frame_image_rgb;
        cv::Mat frame_image;

        cv::cvtColor(frame_image_grb, frame_image_rgb, CV_BGR2RGB);
        cv::flip(frame_image_rgb, frame_image, 0);

        //auto tp = type2str(frame_image.type());
        
        //If the frame is empty, break immediately
        if (frame_image.empty()) 
        {
            RA_LOG_ERROR_ASSERT("frame data is empty");
            return;
        }
    
        // Display the resulting frame
        //imshow( "Frame", frame );
        this->showFrame(frame, frame_image.data);
    }
}

void ES2VideoTexture::showFrame(int frame, const void* data) 
{
    assert(frame < mNumFrames);

    if (frame < mNumFrames) 
    {
        mCurrentFrame = frame;
        //void* data = nullptr;
        GetVisualEffect()->GetTexture2D()->LoadSubData(0, 0, mWidth, mHeight, data);
    }
}

void ES2VideoTexture::UpdateUniformsWithCamera(const Camera* camera)
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

void ES2VideoTexture::_UpdateGeometry()
{   
    using Wm5::Vector3f;
    using Wm5::Vector2f;

    int numOfVerticies = 6; //6 verticies to draw quad
    
    VertexTexture coords[] = 
    {   
        //VertexTexture(Vector3f(0.0f,  0.5f,  0.0f),  Vector2f(0, 0)),
        //VertexTexture(Vector3f(-0.5f, -0.5f,  0.0f),  Vector2f(0, 0)),
        //VertexTexture(Vector3f(0.5f, -0.5f,  0.0f),  Vector2f(0, 0))
        
        VertexTexture(Vector3f(-1, -1, 0),  Vector2f(0, 0)),
        VertexTexture(Vector3f( 1, -1, 0),  Vector2f(1, 0)),
        VertexTexture(Vector3f( 1,  1, 0),  Vector2f(1, 1)),

        VertexTexture(Vector3f(-1, -1, 0),  Vector2f(0, 0)),
        VertexTexture(Vector3f( 1,  1, 0),  Vector2f(1, 1)),
        VertexTexture(Vector3f(-1,  1, 0),  Vector2f(0, 1))
    };

    SetVertexBuffer(std::make_shared<ES2VertexHardwareBuffer>(sizeof(VertexTexture), numOfVerticies, coords, HardwareBuffer::BU_STATIC));
    ES2VertexArraySPtr varray = std::make_shared<ES2VertexArray>(this->GetVisualEffect(), this->GetVertexBuffer());
    SetVertexArray(varray);
}

}