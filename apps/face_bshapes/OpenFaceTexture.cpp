
#include "OpenFaceTexture.h"
#include "RAES2CoreVisualEffects.h"
#include "GLStructures.h"
#include "RACamera.h"
#include "GazeEstimation.h"
#include "FaceAnalyser.h"

namespace tyro
{

int OpenFaceTexture::Init(int width, 
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


int OpenFaceTexture::Init()
{   
  
    vector<string> arguments = {"./tyro", "-device", "0"};

	// no arguments: output usage
	if (arguments.size() == 1)
	{
		cout << "For command line arguments see:" << endl;
		cout << "https://github.com/TadasBaltrusaitis/OpenFace/wiki/Command-line-arguments";
		return 0;
	}

	det_parameters = LandmarkDetector::FaceModelParameters(arguments);
  
	// The modules that are being used for tracking
	face_model = LandmarkDetector::CLNF(det_parameters.model_location);
	if (!face_model.loaded_successfully)
	{
		cout << "ERROR: Could not load the landmark detector" << endl;
		return 1;
	}

	if (!face_model.eye_model)
	{
		cout << "WARNING: no eye model found" << endl;
	}

    visualizer = new Utilities::Visualizer(true, false, false ,false);

    // The sequence reader chooses what to open based on command line arguments provided
    if(!sequence_reader.Open(arguments)) 
    {
        RA_LOG_ERROR_ASSERT("failed to open sequence reader");
        return 1;
    }
    int width = sequence_reader.frame_width;
    int height = sequence_reader.frame_height;
    
    RA_LOG_INFO("Width %i Height %i", width, height);
    RA_LOG_INFO("Device or file opened");

    FaceAnalysis::FaceAnalyserParameters face_analysis_params(arguments);
    face_analysis_params.OptimizeForVideos();
    face_analyser = new FaceAnalysis::FaceAnalyser (face_analysis_params);

    this->Init(width, height, Texture::TextureFormat::TF_R8G8B8);
    
}

OpenFaceTextureSPtr OpenFaceTexture::Create() 
{
    OpenFaceTextureSPtr ptr = std::make_shared<OpenFaceTexture>();
    ptr->Init(); 
    return ptr;   
}

void OpenFaceTexture::getAUs(std::vector<std::string>& names, std::vector<double>& values)
{
    for (auto& a : face_analyser->GetCurrentAUsReg())
    {   
        names.push_back(a.first);
        values.push_back(a.second);
    }
}

void OpenFaceTexture::showFrame() 
{   
    
    // Reading the images
    cv::Mat rgb_image = sequence_reader.GetNextFrame();
    cv::Mat_<uchar> grayscale_image = sequence_reader.GetGrayFrame();

    // The actual facial landmark detection / tracking
    bool detection_success = LandmarkDetector::DetectLandmarksInVideo(rgb_image, face_model, det_parameters, grayscale_image);
    face_analyser->AddNextFrame(rgb_image, face_model.detected_landmarks, face_model.detection_success,sequence_reader.time_stamp, sequence_reader.IsWebcam());

    // Gaze tracking, absolute gaze direction
    cv::Point3f gazeDirection0(0, 0, -1);
    cv::Point3f gazeDirection1(0, 0, -1);

    // If tracking succeeded and we have an eye model, estimate gaze
    if (detection_success && face_model.eye_model)
    {
        GazeAnalysis::EstimateGaze(face_model, gazeDirection0, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, true);
        GazeAnalysis::EstimateGaze(face_model, gazeDirection1, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy, false);
    }

    // Work out the pose of the head from the tracked model
    cv::Vec6d pose_estimate = LandmarkDetector::GetPose(face_model, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy);

    // Keeping track of FPS
    //fps_tracker.AddFrame();

    // Displaying the tracking visualizations
    visualizer->SetImage(rgb_image, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy);
    visualizer->SetObservationLandmarks(face_model.detected_landmarks, face_model.detection_certainty, face_model.GetVisibilities());
    visualizer->SetObservationPose(pose_estimate, face_model.detection_certainty);
    visualizer->SetObservationGaze(gazeDirection0, gazeDirection1, LandmarkDetector::CalculateAllEyeLandmarks(face_model), LandmarkDetector::Calculate3DEyeLandmarks(face_model, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy), face_model.detection_certainty);
    //visualizer->SetFps(fps_tracker.GetFPS());
    //visualizer->SetObservationActionUnits(face_analyser->GetCurrentAUsReg(), face_analyser->GetCurrentAUsClass());
   
   

    // Capture frame-by-frame
    cv::Mat frame_image_grb = visualizer->GetVisImage();

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


void OpenFaceTexture::UpdateUniformsWithCamera(const Camera* camera)
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

void OpenFaceTexture::_UpdateGeometry()
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