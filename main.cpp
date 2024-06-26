#include "bshape_app.h"
#include <cxxopts.hpp>

//#include "torch_model.h"
//#include <Eigen/Dense>
//#include "kdtree.h"

/*
void test_kdtree()
{

    std::vector<std::vector<double>> f = {{1,0,0},{0,1,0},{0,0,1}};
    //f.resize(5,5);
    //f.setIdentity();

    tyro::KDTree tree;
    tree.InitWithData(f);
    std::vector<double> point = {0,1,0};

    double c_dist;
    tree.FindClosest(point, c_dist);
}

void test_tensor()
{
    std::string path_to_model("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced.pth"); 
    tyro::TorchModel model;
    model.Init(path_to_model);

    //Eigen::MatrixXd tensor_in(1,33);
    //tensor_in.setOnes();
    std::vector<double> tensor_in(33, 1.0);
    
    //Eigen::MatrixXd tensor_out;
    std::vector<double> tensor_out;

    model.Compute(tensor_in, tensor_out);

    std::cout << tensor_out << "\n";
}
*/

int main(int argc, char **argv) 
{
    cxxopts::Options options("Bshape program", "One line description of MyProgram");
    options.add_options()
    ("of_v",       "Run in openface video mode")
    ("of_i",       "Run in openface video mode")
    ("ref",        "Run in refinment mode")
    ("panda",      "Render panda data offsreen")
    ("csv",        "Path to csv file", cxxopts::value<std::string>())
    ("out_fldr",   "Path to output folder", cxxopts::value<std::string>())
    ("video",      "Path to video", cxxopts::value<std::string>())
    ;

    auto result = options.parse(argc, argv);

    /***********TODO REMOVE********************/
    tyro::BshapeApp::Experiment exper = tyro::BshapeApp::Experiment::SLIDERS; 
    tyro::BshapeApp appr(exper);

    //std::string path_ss1( "/home/rinat/Workspace/FacialManifoldSource/data_anim/OF_export/result2/frames_out/00560.csv");
    //std::string out_fldr1("/home/rinat/Workspace/FacialManifoldSource/data_anim/OF_export/result2/frames_rendered");
    //appr.LaunchOpenFaceImages(path_ss1, out_fldr1);

    
    if (result.count("of_v"))  
    {
    
    }
    else if (result.count("of_i"))  
    {   
        //std::string path_ss1(argv[1]);
        //std::string out_fldr1(argv[2]);
        //std::string video(argv[3]);
        //std::cout << path_ss1 << "\n";
        //appr.LaunchOpenFaceImages(path_ss1, out_fldr1, video, true);
    }
    else if (result.count("panda"))  
    {   
        std::string path_ss1(argv[1]);
        std::string out_fldr1(argv[2]);
        std::string video(argv[3]);
        std::cout << path_ss1 << "\n";
        appr.LaunchOffScreen(path_ss1, out_fldr1, video, true);
    }
    else if (result.count("ref"))  
    {   
        appr.Setup(800, 800, true);
        int r_code = appr.LaunchRefinment();
    }
    else 
    {   
        std::cout << "here" << "\n";
        
        appr.Setup(1920, 1020, false);
        int r_code = appr.Launch();
    }
    

    /*
    if (argc==1) 
    {

    }
    else if (argc == 2) 
    {

    }
    else if (argc == 3)
    {
        std::string path_ss1(argv[1]);
        std::string out_fldr1(argv[2]);
        std::cout << path_ss1 << "\n";
        appr.LaunchOffScreen(path_ss1, out_fldr1, "", false);
    }
    else if (argc == 4)
    {
        
    }
    */

    // int r_code = appr.VideoToImages();
    // return r_code;
	
    return 0;
}
/*


//std::string path_ss("/home/rinat/Workspace/FacialManifoldSource/data_anim/results/test/2019-01-17_16-27-24/clean");
//
//        RA_LOG_INFO("Files read")
//        for (directory_iterator itr(csv_fldr); itr!=directory_iterator(); ++itr)
//        {   
//            
//            std::cout << itr->path().string() << ' '; 
//            std::cout << '\n';
//        }


///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017, Carnegie Mellon University and University of Cambridge,
// all rights reserved.
//
// ACADEMIC OR NON-PROFIT ORGANIZATION NONCOMMERCIAL RESEARCH USE ONLY
//
// BY USING OR DOWNLOADING THE SOFTWARE, YOU ARE AGREEING TO THE TERMS OF THIS LICENSE AGREEMENT.  
// IF YOU DO NOT AGREE WITH THESE TERMS, YOU MAY NOT USE OR DOWNLOAD THE SOFTWARE.
//
// License can be found in OpenFace-license.txt

//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite at least one of the following works:
//
//       OpenFace 2.0: Facial Behavior Analysis Toolkit
//       Tadas Baltru�aitis, Amir Zadeh, Yao Chong Lim, and Louis-Philippe Morency
//       in IEEE International Conference on Automatic Face and Gesture Recognition, 2018  
//
//       Convolutional experts constrained local model for facial landmark detection.
//       A. Zadeh, T. Baltru�aitis, and Louis-Philippe Morency,
//       in Computer Vision and Pattern Recognition Workshops, 2017.    
//
//       Rendering of Eyes for Eye-Shape Registration and Gaze Estimation
//       Erroll Wood, Tadas Baltru�aitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling 
//       in IEEE International. Conference on Computer Vision (ICCV),  2015 
//
//       Cross-dataset learning and person-specific normalisation for automatic Action Unit detection
//       Tadas Baltru�aitis, Marwa Mahmoud, and Peter Robinson 
//       in Facial Expression Recognition and Analysis Challenge, 
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015 
//
///////////////////////////////////////////////////////////////////////////////
// FaceTrackingVid.cpp : Defines the entry point for the console application for tracking faces in videos.

// Libraries for landmark detection (includes CLNF and CLM modules)
#include "LandmarkCoreIncludes.h"
#include "GazeEstimation.h"

#include <SequenceCapture.h>
#include <Visualizer.h>
#include <VisualizationUtils.h>
#include <FaceAnalyser.h>

#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

static void printErrorAndAbort(const std::string & error)
{
	std::cout << error << std::endl;
	abort();
}

#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )

using namespace std;

vector<string> get_arguments(int argc, char **argv)
{

	vector<string> arguments;

	for (int i = 0; i < argc; ++i)
	{
		arguments.push_back(string(argv[i]));
	}
	return arguments;
}

int main(int argc, char **argv)
{

	vector<string> arguments = get_arguments(argc, argv);

	// no arguments: output usage
	if (arguments.size() == 1)
	{
		cout << "For command line arguments see:" << endl;
		cout << "https://github.com/TadasBaltrusaitis/OpenFace/wiki/Command-line-arguments";
		return 0;
	}

	LandmarkDetector::FaceModelParameters det_parameters(arguments);

	// The modules that are being used for tracking
	LandmarkDetector::CLNF face_model(det_parameters.model_location);
	if (!face_model.loaded_successfully)
	{
		cout << "ERROR: Could not load the landmark detector" << endl;
		return 1;
	}

	if (!face_model.eye_model)
	{
		cout << "WARNING: no eye model found" << endl;
	}

	// Open a sequence
	Utilities::SequenceCapture sequence_reader;

	// A utility for visualizing the results (show just the tracks)
	Utilities::Visualizer visualizer(true, false, false, true);

	// Tracking FPS for visualization
	Utilities::FpsTracker fps_tracker;
	fps_tracker.AddFrame();

	int sequence_number = 0;

	while (true) // this is not a for loop as we might also be reading from a webcam
	{

		// The sequence reader chooses what to open based on command line arguments provided
		if (!sequence_reader.Open(arguments))
			break;

		INFO_STREAM("Device or file opened");

        FaceAnalysis::FaceAnalyserParameters face_analysis_params(arguments);
        face_analysis_params.OptimizeForVideos();
		FaceAnalysis::FaceAnalyser face_analyser(face_analysis_params);
        
        cv::Mat rgb_image = sequence_reader.GetNextFrame();

		INFO_STREAM("Starting tracking");
		while (!rgb_image.empty()) // this is not a for loop as we might also be reading from a webcam
		{

			// Reading the images
			cv::Mat_<uchar> grayscale_image = sequence_reader.GetGrayFrame();

			// The actual facial landmark detection / tracking
			bool detection_success = LandmarkDetector::DetectLandmarksInVideo(rgb_image, face_model, det_parameters, grayscale_image);
            face_analyser.AddNextFrame(rgb_image, face_model.detected_landmarks, face_model.detection_success,sequence_reader.time_stamp, sequence_reader.IsWebcam());
            //face_analyser.PredictStaticAUsAndComputeFeatures(rgb_image, face_model.detected_landmarks);

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
			fps_tracker.AddFrame();

			// Displaying the tracking visualizations
			visualizer.SetImage(rgb_image, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy);
			visualizer.SetObservationLandmarks(face_model.detected_landmarks, face_model.detection_certainty, face_model.GetVisibilities());
			visualizer.SetObservationPose(pose_estimate, face_model.detection_certainty);
			visualizer.SetObservationGaze(gazeDirection0, gazeDirection1, LandmarkDetector::CalculateAllEyeLandmarks(face_model), LandmarkDetector::Calculate3DEyeLandmarks(face_model, sequence_reader.fx, sequence_reader.fy, sequence_reader.cx, sequence_reader.cy), face_model.detection_certainty);
			visualizer.SetFps(fps_tracker.GetFPS());
            visualizer.SetObservationActionUnits(face_analyser.GetCurrentAUsReg(), face_analyser.GetCurrentAUsClass());
            for (auto& a : face_analyser.GetCurrentAUsReg())
            {
                std:cout << a.first << " " << a.second << "\n";
            }
            // detect key presses (due to pecularities of OpenCV, you can get it when displaying images)
			char character_press = visualizer.ShowObservation();

			// restart the tracker
			if (character_press == 'r')
			{
				face_model.Reset();
			}
			// quit the application
			else if (character_press == 'q')
			{
				return(0);
			}

			// Grabbing the next frame in the sequence
			rgb_image = sequence_reader.GetNextFrame();

		}

		// Reset the model, for the next video
		face_model.Reset();
		sequence_reader.Close();

		sequence_number++;

	}
	return 0;
}
*/
