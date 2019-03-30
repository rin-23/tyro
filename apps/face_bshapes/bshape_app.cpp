#include "bshape_app.h"
#include "RAEnginePrerequisites.h"
#include "RAES2TextOverlay.h"
#include "RAFont.h"
#include "RAVisibleSet.h"
#include <stdio.h>
#include <functional>
#include "Wm5APoint.h"
#include "Wm5Vector2.h"
#include "Wm5Vector4.h"
#include "TyroIGLMesh.h"
#include <random>
#include "load_mesh_sequence.h"
#include "TyroIGLMesh.h"
#include <filesystem/path.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "OpenFaceTexture.h"
#include "RATextureBuffer.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include "utils.h"
#include "common.h"

using namespace boost::filesystem;

using namespace std;

using Wm5::APoint;
//255,223,196

namespace tyro
{   
    void console_save_expression(App* app, const std::vector<std::string>& args) 
    {
        BshapeApp* bapp =(BshapeApp*) app;
        //if (args.size()!=1) return;
        auto name = "saved_expression.csv"; //filesystem::path(args[0]);
        const std::string RENDER_IMGS_PATH = "/home/rinat/Workspace/FacialManifoldSource/data_anim";
        auto fldr = filesystem::path(RENDER_IMGS_PATH)/name;

        std::ofstream myfile;
        myfile.open (fldr.str());
        //setup torch model
        
        auto computeValues = [&] (const std::vector<std::string>& up_bnames,
                                  const std::vector<double>& up_values,
                                  const std::vector<std::string>& low_bnames,
                                  const std::vector<double>& low_values)
        {
            for (int i=0; i < up_bnames.size(); ++i) 
            {
                myfile << up_bnames[i]; 
                //if (i != up_bnames.size() -1) 
                myfile << ",";
            }

            for (int i=0; i < low_bnames.size(); ++i)
            {
                myfile << low_bnames[i];
                if (i !=  low_bnames.size() - 1) 
                    myfile << ",";
            }

            myfile << "\n";

            for (int i=0; i<up_values.size(); ++i) 
            {
                myfile << up_values[i];
                //if (i != up_values.size() - 1) 
                myfile << ",";
            }

            for (int i=0; i<low_values.size(); ++i) 
            {
                myfile << low_values[i];
                if (i != low_values.size() - 1) 
                    myfile << ",";
            }
        };

        if (bapp->mExperiment == BshapeApp::Experiment::PS4) 
        {   
            computeValues(bapp->mGamepad.up_bnames, 
                          bapp->mGamepad.up_values, 
                          bapp->mGamepad.low_bnames, 
                          bapp->mGamepad.low_values);
        }
        else if (bapp->mExperiment == BshapeApp::Experiment::SLIDERS)
        {            
            computeValues(bapp->mSliders.up_bnames, 
                          bapp->mSliders.up_values, 
                          bapp->mSliders.low_bnames, 
                          bapp->mSliders.low_values);
        }
        else if (bapp->mExperiment == BshapeApp::Experiment::CAMERA)
        {
            computeValues(bapp->mCameraControl.up_bnames, 
                          bapp->mCameraControl.up_values, 
                          bapp->mCameraControl.low_bnames, 
                          bapp->mCameraControl.low_values);
        } 

        myfile.close();
    }

    void console_render_to_image(App* app, const std::vector<std::string>& args) 
    {   
        BshapeApp* bapp =(BshapeApp*) app;
        if (args.size()!=1) return;
        auto name = filesystem::path(args[0]);
        
        //TODOD DRAW SHIT HERE
        //bapp->DrawMeshes();

        // Poll for and process events
        bapp->m_tyro_window->GetGLContext()->swapBuffers();

        //make sure everything was drawn
        glFlush();
        glFinish();
        GL_CHECK_ERROR;
        
        int v_width, v_height;
        bapp->m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        u_int8_t* texture = (u_int8_t*) malloc(4*v_width *v_height);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glReadPixels(0, 0, v_width, v_height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
        GL_CHECK_ERROR;

        cv::Mat image1(v_height, v_width, CV_8UC4, texture);
        cv::Mat image2, image3;
        cv::cvtColor(image1, image2, CV_RGBA2BGR);
        cv::flip(image2, image3, 0);
        
        const std::string RENDER_IMGS_PATH = "/home/rinat/Workspace/FacialManifoldSource/data_anim/images";

        auto fldr = filesystem::path(RENDER_IMGS_PATH)/name;
        if (!fldr.exists())
            filesystem::create_directory(fldr); 
        
        fldr = fldr/filesystem::path("_1.png");

        cv::imwrite(fldr.str(), image3);
        free(texture);
    }

    void console_render_to_images(App* app, const std::vector<std::string>& args) 
    {
        if (args.size()!=1) 
            return;
        
        
        BshapeApp* bapp =(BshapeApp*) app;
        auto name = filesystem::path(args[0]);
        const std::string RENDER_IMGS_PATH = "/home/rinat/Workspace/FacialManifoldSource/data_anim/images";
        auto fldr = filesystem::path(RENDER_IMGS_PATH)/name;
        if (!fldr.exists()) 
        {
            filesystem::create_directory(fldr); 
        }
        
        int v_width, v_height;
        bapp->m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        u_int8_t* texture = (u_int8_t*) malloc(4*v_width *v_height);

        for (int frame = 0; frame < bapp->mCurAnimation.getNumFrames(); ++frame) 
        { 
            bapp->m_frame = frame;
            /*DRAW SHIT HERE*/
            //bapp->DrawMeshes();

            // Poll for and process events
            bapp->m_tyro_window->GetGLContext()->swapBuffers();

            //make sure everything was drawn
            glFlush();
            glFinish();
            GL_CHECK_ERROR;
            
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glReadPixels(0, 0, v_width, v_height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
            GL_CHECK_ERROR;

            cv::Mat image1(v_height, v_width, CV_8UC4, texture);
            cv::Mat image2, image3;
            cv::cvtColor(image1, image2, CV_RGBA2BGR);
            cv::flip(image2, image3, 0);
                        
            auto fldr_to_write = fldr/filesystem::path(std::to_string(frame)+".png");

            cv::imwrite(fldr_to_write.str(), image3);
        }
        free(texture);
    }

    BshapeApp::BshapeApp(Experiment exp)
    :
    App(),
    m_frame(0),
    m_state(BshapeApp::State::None),
    m_need_rendering(false),
    m_frame_overlay(nullptr),
    m_show_wire(true),
    mExperiment(exp)
    {}

    BshapeApp::~BshapeApp() 
    {}
    
    int BshapeApp::Setup(int width, int height, bool refinment) 
    {   
        App::Setup(width, height);

        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);

        m_state = BshapeApp::State::Launched;
        m_need_rendering = true;
     
        m_timeline = new Timeline(24, 300);
        m_timeline->frameChanged = [&](Timeline& timeline, int frame)->void 
        {   
            //RA_LOG_INFO("Frame Change BEGIN");
            m_frame = frame;
            m_need_rendering = true;
            //glfwPostEmptyEvent();
            //RA_LOG_INFO("Frame Change END");
        };
     
        register_console_function("render_to_image", console_render_to_image, ""); 
        register_console_function("render_to_images", console_render_to_images, ""); 
        register_console_function("save_face", console_save_expression, ""); 

        // show current number of frames
        ES2FontSPtr font = FontManager::GetSingleton()->GetSystemFontOfSize12();
        
        m_frame_overlay = ES2TextOverlay::Create(std::string("Not initialized text"), 
                                                 Wm5::Vector2f(0, 0), 
                                                 font, 
                                                 Wm5::Vector4f(0,0,1,1), 
                                                 viewport);
        m_frame_overlay->SetTranslate(Wm5::Vector2i(-viewport[2]/2 + 50,-viewport[3]/2 + 50));

        m_dist1 = ES2TextOverlay::Create(std::string("Min distance to manifold"), 
                                        Wm5::Vector2f(0, 0), 
                                        font, 
                                        Wm5::Vector4f(0,0,1,1), 
                                        viewport);
        m_dist1->SetTranslate(Wm5::Vector2i(-viewport[2]/2 + 250,-viewport[3]/2 + 150));

        m_dist2 = ES2TextOverlay::Create(std::string("Min distance to manifold"), 
                                         Wm5::Vector2f(0, 0), 
                                         font, 
                                         Wm5::Vector4f(0,0,1,1), 
                                         viewport);

        m_dist2->SetTranslate(Wm5::Vector2i(viewport[2]/2 - 600,-viewport[3]/2 + 150));

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(m_tyro_window->GetGLFWWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 400");

        //load all bshapes and neuteral expression
        std::string modelPath("/home/rinat/Workspace/tyro/apps/face_bshapes/resources/facemodel");
        const bool load_serialized = true;
        if (load_serialized) 
        {
            mFaceModel.deserialize(modelPath);
        }
        else
        {
            mFaceModel.InitDefault();
            mFaceModel.serialize(modelPath);
        }
        
        RENDER.CreateMesh(mFaceModel);
        if (!refinment) RENDER.CreateMesh2(mFaceModel);
        
        update_camera();

        //setup torch model
        if (mExperiment == Experiment::PS4) 
        {
            mTorchModelUp.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced_upper_sliders.pth");
            mTorchModelLow.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced_lower_sliders.pth");
           
            int present = m_tyro_window->JoystickConnected(); 
            RA_LOG_INFO("Found joystik %i", present);

            mGamepad.Init();
        }
        else if (mExperiment == Experiment::SLIDERS)
        {
            mTorchModelUp.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced_upper_sliders.pth");
            mTorchModelLow.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced_lower_sliders.pth");
            
            mSliders.Init();
        }
        else if (mExperiment == Experiment::CAMERA)
        {
            mTorchModelUp.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced_upper_sliders.pth");
            mTorchModelLow.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced_lower_sliders.pth");
           
            // create a video stream
            m_camera_texture = OpenFaceTexture::Create();
            mCameraControl.Init();
        } 

        //init kd tree
        //std::string csv_file = "/home/rinat/Workspace/FacialManifoldSource/data_anim/clusters_lower/augemented.txt";
        //tyro::csvToVector(csv_file, MOTION_DATA);
        //mTree.InitWithData(MOTION_DATA);
    }
    
    int BshapeApp::Launch()
    {   
        m_state = BshapeApp::State::LoadedModel;

        while (!m_tyro_window->ShouldClose())
        {   
            m_gl_rend->ClearScreen();

            m_tyro_window->ProcessUserEvents();

            if (mExperiment == Experiment::PS4) 
                GamepadExample();
            else if (mExperiment == Experiment::SLIDERS)
                SliderExample();    
            else if (mExperiment == Experiment::CAMERA)
                CameraExample();

            //DrawUI();
            //loadOpenFace(); 
            //loadFrame(m_frame);
            //FetchGamepadInputs();
            //ComputeDistanceToManifold();
            //DrawMeshes();

            // Draw console
            if (show_console) 
            {
                glUseProgram(0);
                m_console.display(2);
            }
            
            m_tyro_window->GetGLContext()->swapBuffers();
            m_need_rendering = false;             
            //m_tyro_window->Wait();
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

	    return 0;
    }
    
    int BshapeApp::LaunchRefinment() 
    {
        //load animation
        //loadAnimation(ANIM_LIST[0]);
        double previousTime = m_tyro_window->TimeNow();

        auto render_refine = [&] (bool force = false)
        {
            double currentTime = m_tyro_window->TimeNow();

            if (!force) 
            {
                if (currentTime - previousTime < 0.1 )  
                    return;
            }

            previousTime = currentTime;

            m_gl_rend->ClearScreen();

           /******   DRAW UI  *******/
            //std::vector<double> low_values_denoised, up_values_denoised;
            //mTorchModelLow.Compute(mSliders.low_values, low_values_denoised);
            //mTorchModelUp.Compute(mSliders.up_values, up_values_denoised); 

            /****** RENDER MESH *******/
            VisibleSet vis_set;
            
            mFaceModel.setWeights(mSliders.low_bnames, mSliders.low_values);
            mFaceModel.setWeights(mSliders.up_bnames,  mSliders.up_values);
            
            /*
            Eigen::MatrixXd V, N;
            Eigen::MatrixXi F;
            mFaceModel.getExpression(V, F, N);
            RENDER.mesh->UpdateData(V, F, N, MESH_COLOR);
            RENDER.mesh->Update(true);
            vis_set.Insert(RENDER.mesh.get());
            */
            RENDER.UpdateMesh(mFaceModel);
            RENDER.AddToVisibleSet(vis_set);
    
            //mFaceModel.setWeights(mSliders.low_bnames, low_values_denoised);
            //mFaceModel.setWeights(mSliders.up_bnames, up_values_denoised);
            //Eigen::MatrixXd V2, N2;
            //Eigen::MatrixXi F2;
            //mFaceModel.getExpression(V2, F2, N2);
            //RENDER.mesh2->UpdateData(V2, F2, N2, MESH_COLOR);
            //RENDER.mesh2->Update(true);
            //vis_set.Insert(RENDER.mesh2.get());
    
            m_gl_rend->RenderVisibleSet(&vis_set, m_camera);      
            /*
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::SetNextWindowSize(ImVec2(430,450), ImGuiCond_FirstUseEver);
            bool flag = true;
            ImGui::Begin("Blendshapes", &flag);
            ImGui::Text("Upper");
            for (int i = 0; i < mSliders.up_values.size(); ++i)
            {   
                ImGui::PushID(i);
                //ImGui::Text("%0.2f", up_values_denoised[i]);
                //ImGui::SameLine();
                float val = mSliders.up_values[i];
                ImGui::SliderFloat(mSliders.up_bnames[i].c_str(), &val, 0.0f, 1.0f, "%.2f");
                mSliders.up_values[i] = (double)val;
                
                ImGui::PopID();
            }
            ImGui::Separator();
            ImGui::Text("Lower");
            for (int i = 0; i < mSliders.low_values.size(); ++i)
            {   
                ImGui::PushID(mSliders.up_values.size() + i);
                //ImGui::Text("%0.2f", low_values_denoised[i]);
                //ImGui::SameLine();
                float val = mSliders.low_values[i];
                ImGui::SliderFloat(mSliders.low_bnames[i].c_str(), &val, 0.0f, 1.0f, "%.2f");
                mSliders.low_values[i] = val;
                ImGui::PopID();
            }
            ImGui::End();        
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            */
            m_tyro_window->GetGLContext()->swapBuffers();


        };

        render_refine(true);

        double a = 0;
        int idx = 0;
        while (true)
        {   
            std::cin >> a;
            std::cout << "LUL " << a << "\n"; 
            if (int(a) == -1) 
            {   
                render_refine();
                std::cout << "Starting to read frame\n";
                idx = 0;
            } 
            else if (int(a) == -2) 
            {
                break; 
            }
            else 
            {   
                if (idx < mSliders.up_values.size()) 
                {
                    mSliders.up_values[idx] = a;
                }
                else 
                {
                    int nidx = idx - mSliders.up_values.size();
                    mSliders.low_values[nidx] = a;
                }
                idx++;                
            }
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

	    return 0;
    }

    int BshapeApp::LaunchOffScreen(const std::string& csv_file, 
                                   const std::string& out_fldr, 
                                   const std::string& video_stream, 
                                   bool isOpenFace) 
    {   
        RA_LOG_INFO("Launching the app in offscreen mode");
 
        // setup window
        m_tyro_window = new Window();
        m_tyro_window->InitOffscreen(1920,1080);
                
        // setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        m_gl_rend->SetClearColor(Wm5::Vector4f(220/255.0, 220/255.0, 220/255.0, 1));
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        
        // load all bshapes and neuteral expression
        std::string modelPath("/home/rinat/Workspace/tyro/apps/face_bshapes/resources/facemodel");
        mFaceModel.deserialize(modelPath);
        
        RENDER.CreateMesh(mFaceModel);
        
        if (isOpenFace) 
        {
            RENDER.CreateMesh2(mFaceModel);        

            mCurAnimation.readOpenFaceCsv(csv_file,-1);
            
            mTorchModelUp.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced_upper_sliders.pth");
            mTorchModelLow.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced_lower_sliders.pth");
           
            // create a video stream
            mCameraControl.Init();

            m_video_texture = ES2VideoTexture::Create(video_stream);
        } 
        else
        {
            mCurAnimation.readPandasCsv(csv_file, 0);
        }
        
        assert(mCurAnimation.getNumFrames() > 0);
        std::cout << mCurAnimation.mAttrs << "\n";
        update_camera();

        if (isOpenFace) m_camera->Translate(Wm5::Vector2i(200,0));
 
        u_int8_t* texture = (u_int8_t*) malloc(4*v_width *v_height);
        auto out_path = filesystem::path(out_fldr);
        
        for (int i=0;i<mCurAnimation.getNumFrames(); ++i) 
        {
            m_gl_rend->ClearScreen();

            std::vector<double> W;
            std::vector<std::string> A;
            mCurAnimation.getWeights(i, A, W);
            
            VisibleSet vis_set;

            if (isOpenFace) 
            {
                // Fetch OpenFace AUs 
                mCameraControl.Update(A, W, true);
                mCameraControl.Print();
                
                // Denoise
                std::vector<double> low_values_denoised, up_values_denoised;
                mTorchModelUp.Compute(mCameraControl.up_values,   up_values_denoised); 
                mTorchModelLow.Compute(mCameraControl.low_values, low_values_denoised);
                
                // Render Mesh
                mFaceModel.setWeights(mCameraControl.low_bnames, mCameraControl.low_values);
                mFaceModel.setWeights(mCameraControl.up_bnames,  mCameraControl.up_values);
                Eigen::MatrixXd V, N;
                Eigen::MatrixXi F;
                mFaceModel.getExpression(V, F, N);
                RENDER.mesh->UpdateData(V, F, N, MESH_COLOR);
                RENDER.mesh->Update(true);
                vis_set.Insert(RENDER.mesh.get());
                
                mFaceModel.setWeights(mCameraControl.up_bnames, up_values_denoised);
                mFaceModel.setWeights(mCameraControl.low_bnames, low_values_denoised);
                Eigen::MatrixXd V2, N2;
                Eigen::MatrixXi F2;
                mFaceModel.getExpression(V2, F2, N2);
                RENDER.mesh2->UpdateData(V2, F2, N2, MESH_COLOR);
                RENDER.mesh2->Update(true);
                vis_set.Insert(RENDER.mesh2.get());

                m_video_texture->showFrame(i);
                vis_set.Insert(m_video_texture.get());
            }
            else 
            {
                mFaceModel.setWeights(A, W);
                
                Eigen::MatrixXd V, N;
                Eigen::MatrixXi F;
                mFaceModel.getExpression(V, F, N);
                
                RENDER.mesh->UpdateData(V, F, N, MESH_COLOR);
                RENDER.mesh->Update(true);
                
                vis_set.Insert(RENDER.mesh.get());
            }            
            
            m_gl_rend->RenderVisibleSet(&vis_set, m_camera);       
            // Poll for and process events
            m_tyro_window->GetGLContext()->swapBuffers();
            //make sure everything was drawn
            glFlush();
            glFinish();
            GL_CHECK_ERROR;
            
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glReadPixels(0, 0, v_width, v_height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
            GL_CHECK_ERROR;

            cv::Mat image1(v_height, v_width, CV_8UC4, texture);
            cv::Mat image2, image3;
            cv::cvtColor(image1, image2, CV_RGBA2BGR);
            cv::flip(image2, image3, 0);
                        
            auto fldr_to_write = out_path/filesystem::path(std::to_string(i)+".png");

            cv::imwrite(fldr_to_write.str(), image3);  
         //   break; 
        }

        free(texture);
        
	    return 0;
    }

    int BshapeApp::LaunchOpenFaceImages(const std::string& csv_file, 
                                        const std::string& out_fldr) 
    {   
        RA_LOG_INFO("Launching the app in offscreen mode");
 
        // setup window
        m_tyro_window = new Window();
        m_tyro_window->InitOffscreen(1920,1080);
                
        // setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        m_gl_rend->SetClearColor(Wm5::Vector4f(220/255.0, 220/255.0, 220/255.0, 1));
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        
        // load all bshapes and neuteral expression
        std::string modelPath("/home/rinat/Workspace/tyro/apps/face_bshapes/resources/facemodel");
        mFaceModel.deserialize(modelPath);  
        RENDER.CreateMesh(mFaceModel);
        RENDER.CreateMesh2(mFaceModel);        

        mCurAnimation.readOpenFaceCsv(csv_file,-1);
        assert(mCurAnimation.getNumFrames() > 0);
        std::cout << mCurAnimation.mAttrs << "\n";
        
        mTorchModelUp.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced_upper_sliders.pth");
        mTorchModelLow.Init("/home/rinat/Workspace/FacialManifoldSource/data_anim/traced_lower_sliders.pth");
        
        mCameraControl.Init();
        
        update_camera();

        //if (isOpenFace) m_camera->Translate(Wm5::Vector2i(200,0));
 
        u_int8_t* texture = (u_int8_t*) malloc(4*v_width *v_height);
        auto out_path = filesystem::path(out_fldr);
        
        for (int i=0;i<mCurAnimation.getNumFrames(); ++i) 
        {
            m_gl_rend->ClearScreen();

            std::vector<double> W;
            std::vector<std::string> A;
            mCurAnimation.getWeights(i, A, W);

            // Fetch OpenFace AUs 
            mCameraControl.Update(A, W, true);
            mCameraControl.Print();
            
            // Denoise
            std::vector<double> low_values_denoised, up_values_denoised;
            mTorchModelUp.Compute(mCameraControl.up_values,   up_values_denoised); 
            mTorchModelLow.Compute(mCameraControl.low_values, low_values_denoised);
            
            // Render Mesh
            mFaceModel.setWeights(mCameraControl.low_bnames, mCameraControl.low_values);
            mFaceModel.setWeights(mCameraControl.up_bnames,  mCameraControl.up_values);
 
            RENDER.UpdateMesh(mFaceModel);
            RENDER.UpdateMesh2(mFaceModel);

            VisibleSet vis_set;
            RENDER.AddToVisibleSet(vis_set);
          
            m_gl_rend->RenderVisibleSet(&vis_set, m_camera);       
            m_tyro_window->GetGLContext()->swapBuffers();

            //make sure everything was drawn
            glFlush();
            glFinish();
            GL_CHECK_ERROR;
            
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glReadPixels(0, 0, v_width, v_height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
            GL_CHECK_ERROR;

            cv::Mat image1(v_height, v_width, CV_8UC4, texture);
            cv::Mat image2, image3;
            cv::cvtColor(image1, image2, CV_RGBA2BGR);
            cv::flip(image2, image3, 0);
                        
            auto fldr_to_write = out_path/filesystem::path(std::to_string(i)+".png");

            cv::imwrite(fldr_to_write.str(), image3);  
        }

        free(texture);
        
	    return 0;
    }
    
    void BshapeApp::GamepadExample() 
    {   
        /*******  FETCH GAMEPAD INPUTS ********/
        std::map<std::string, double> axes; 
        std::map<std::string, bool> btns;
        m_tyro_window->JoystickAxes(axes);
        m_tyro_window->JoystickButtons(btns);
        mGamepad.UpdateFrame(axes, btns);

        std::vector<double> low_values_denoised, up_values_denoised;
        mTorchModelLow.Compute(mGamepad.low_values, low_values_denoised);
        mTorchModelUp.Compute(mGamepad.up_values, up_values_denoised); 

        /****** RENDER MESH *******/
        VisibleSet vis_set;
        
        mFaceModel.setWeights(mGamepad.low_bnames, mGamepad.low_values);
        mFaceModel.setWeights(mGamepad.up_bnames, mGamepad.up_values);

        RENDER.UpdateMesh(mFaceModel);
        RENDER.UpdateMesh2(mFaceModel);
        RENDER.AddToVisibleSet(vis_set);

        m_gl_rend->RenderVisibleSet(&vis_set, m_camera);      


        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(430,450), ImGuiCond_FirstUseEver);
        bool flag = true;
        ImGui::Begin("Blendshapes", &flag);
        
        ImGui::Text("Upper");
        
        for (int i = 0; i < mGamepad.up_values.size(); ++i)
        {   
            ImGui::PushID(i);
            ImGui::Text("%0.2f den %0.2f", mGamepad.up_values[i], up_values_denoised[i]);
            ImGui::SameLine();
            ImGui::Text("%s", mGamepad.up_bnames[i].c_str());
            ImGui::PopID();
        }
        
        ImGui::Separator();
        
        ImGui::Text("Lower");
        for (int i = 0; i < mGamepad.low_values.size(); ++i)
        {   
            ImGui::PushID(i);
            ImGui::Text("%0.2f den %0.2f", mGamepad.low_values[i], low_values_denoised[i]);
            ImGui::SameLine();
            ImGui::Text("%s", mGamepad.low_bnames[i].c_str());
            ImGui::PopID();
        }
        
        ImGui::End();        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        //DrawUI();
        //loadOpenFace(); 
        //loadFrame(m_frame);
        //FetchGamepadInputs();
        //ComputeDistanceToManifold();
        //DrawMeshes();
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void BshapeApp::SliderExample() 
    {   
        /******   DRAW UI  *******/
        std::vector<double> low_values_denoised, up_values_denoised;
        mTorchModelLow.Compute(mSliders.low_values, low_values_denoised);
        mTorchModelUp.Compute(mSliders.up_values, up_values_denoised); 

        /****** RENDER MESH *******/
        VisibleSet vis_set;
        
        mFaceModel.setWeights(mSliders.low_bnames, mSliders.low_values);
        mFaceModel.setWeights(mSliders.up_bnames,  mSliders.up_values);
   
        RENDER.UpdateMesh(mFaceModel);
        RENDER.UpdateMesh2(mFaceModel);
        RENDER.AddToVisibleSet(vis_set);
        m_gl_rend->RenderVisibleSet(&vis_set, m_camera);      

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //bool p_open = true;
        //ImGui::ShowDemoWindow(&p_open);

        ImGui::SetNextWindowSize(ImVec2(430,450), ImGuiCond_FirstUseEver);
        bool flag = true;
        ImGui::Begin("Blendshapes", &flag);
        ImGui::Text("Upper");
        for (int i = 0; i < mSliders.up_values.size(); ++i)
        {   
            ImGui::PushID(i);
            ImGui::Text("%0.2f", up_values_denoised[i]);
            ImGui::SameLine();
            float val = mSliders.up_values[i];
            ImGui::SliderFloat(mSliders.up_bnames[i].c_str(), &val, 0.0f, 1.0f, "%.2f");
            mSliders.up_values[i] = (double)val;
            
            ImGui::PopID();
        }
        ImGui::Separator();
        ImGui::Text("Lower");
        for (int i = 0; i < mSliders.low_values.size(); ++i)
        {   
            ImGui::PushID(mSliders.up_values.size() + i);
            ImGui::Text("%0.2f", low_values_denoised[i]);
            ImGui::SameLine();
            float val = mSliders.low_values[i];
            ImGui::SliderFloat(mSliders.low_bnames[i].c_str(), &val, 0.0f, 1.0f, "%.2f");
            mSliders.low_values[i] = val;
            ImGui::PopID();
        }
        ImGui::End();        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //DrawUI();
        //loadOpenFace(); 
        //loadFrame(m_frame);
        //FetchGamepadInputs();
        //ComputeDistanceToManifold();
        //DrawMeshes();
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void BshapeApp::CameraExample() 
    {      
        // Fetch OpenFace frame 
        VisibleSet vis_set;
        m_camera_texture->showFrame();
        vis_set.Insert(m_camera_texture.get());

        // Fetch OpenFace AUs 
        std::vector<std::string> names; 
        std::vector<double> values;
        m_camera_texture->getAUs(names, values);
        mCameraControl.Update(names, values);

        // Denoise
        std::vector<double> low_values_denoised, up_values_denoised;
        mTorchModelLow.Compute(mCameraControl.low_values, low_values_denoised);
        mTorchModelUp.Compute(mCameraControl.up_values, up_values_denoised); 
  
        // Render Mesh
        mFaceModel.setWeights(mCameraControl.low_bnames, mCameraControl.low_values);
        mFaceModel.setWeights(mCameraControl.up_bnames,  mCameraControl.up_values);

        RENDER.UpdateMesh(mFaceModel);
        RENDER.UpdateMesh2(mFaceModel);
        RENDER.AddToVisibleSet(vis_set);
        m_gl_rend->RenderVisibleSet(&vis_set, m_camera);      
    }

    void BshapeApp::update_camera() 
    {
        //setup camera
        AxisAlignedBBox WorldBoundBox = RENDER.mesh->WorldBoundBox;
        if (RENDER.mesh2)
            WorldBoundBox.Merge(RENDER.mesh2->WorldBoundBox);
        
        Wm5::APoint world_center = WorldBoundBox.GetCenter();
        float radius = std::abs(WorldBoundBox.GetRadius()*2);
        //Wm5::APoint world_center = Wm5::APoint::ORIGIN;
        //float radius = 1;
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        float aspect = (float)v_width/v_height;
        
        if (m_camera)
            delete m_camera;
        
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
    }

    void BshapeApp::key_pressed(Window& window, unsigned int key, int modifiers) 
    {   
        //RA_LOG_INFO("Key pressed %c", key);
        
        if (key == '`') 
        {   
            //RA_LOG_INFO("Pressed %c", key);
            show_console = !show_console;
            render();
            return;
        }

        if (show_console) 
        {  
           m_console.keyboard(key);
           render();
           return;
        }
        else 
        {
            if (key == 'p') 
            {
                if (m_timeline->state == Timeline::State::Running)
                    m_timeline->Pause();
                else
                    m_timeline->Start();
            }
            else if (key == ']') //next frame
            {
                m_timeline->NextFrame();
                render();
            }
            else if (key == '[')
            {
                m_timeline->PrevFrame();
                render();
            }            
        }
    }
}


