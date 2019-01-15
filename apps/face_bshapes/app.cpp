#include "app.h"
#include "RAEnginePrerequisites.h"
#include "RAES2TextOverlay.h"
#include "RAFont.h"
#include "RAVisibleSet.h"
//#include "RAES2StandardMesh.h"
//#include "RAAxisAlignedBBox.h"
#include <stdio.h>
#include <functional>
#include "Wm5APoint.h"
#include "Wm5Vector2.h"
#include "Wm5Vector4.h"
#include "TyroIGLMesh.h"
//#include "TyroFileUtils.h"
#include <random>
#include "load_mesh_sequence.h"
#include "TyroIGLMesh.h"
#include "common.h"
#include <filesystem/path.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace std;

using Wm5::APoint;

#define MESH_COLOR Eigen::Vector3d(109/255.0f, 150/255.0f, 144/255.0f)

namespace tyro
{   
    void console_render_to_image(App* app, const std::vector<std::string>& args) 
    {
        if (args.size()!=1) return;
        auto name = filesystem::path(args[0]);
        app->DrawMeshes();

        // Poll for and process events
        app->m_tyro_window->GetGLContext()->swapBuffers();

        //make sure everything was drawn
        glFlush();
        glFinish();
        GL_CHECK_ERROR;
        
        int v_width, v_height;
        app->m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        u_int8_t* texture = (u_int8_t*) malloc(4*v_width *v_height);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glReadPixels(0, 0, v_width, v_height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
        GL_CHECK_ERROR;

        cv::Mat image1(v_height, v_width, CV_8UC4, texture);
        cv::Mat image2, image3;
        cv::cvtColor(image1, image2, CV_RGBA2BGR);
        cv::flip(image2, image3, 0);
        
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
        
        auto name = filesystem::path(args[0]);
        auto fldr = filesystem::path(RENDER_IMGS_PATH)/name;
        if (!fldr.exists()) 
        {
            filesystem::create_directory(fldr); 
        }
        
        int v_width, v_height;
        app->m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        u_int8_t* texture = (u_int8_t*) malloc(4*v_width *v_height);

        for (int frame = 0; frame < app->mCurAnimation.getNumFrames(); ++frame) 
        { 
            app->m_frame = frame;
            app->DrawMeshes();

            // Poll for and process events
            app->m_tyro_window->GetGLContext()->swapBuffers();

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

    App::App()
    :
    m_tyro_window(nullptr),
    m_gl_rend(nullptr),
    m_camera(nullptr),
    mouse_is_down(false),
    gesture_state(0),
    show_console(false),
    m_frame(0),
    m_state(App::State::None),
    m_need_rendering(false),
    //m_sel_primitive(App::SelectionPrimitive::Faces),
    //m_sel_method(App::SelectionMethod::OneClick),
    m_update_camera(false),
    m_frame_overlay(nullptr),
    m_show_wire(true)
    //add_seg_faces(false),
    //m_video_texture(nullptr),
    //m_frame_offset(0)
    {}

    App::~App() 
    {
        if (m_tyro_window)
        {
            m_tyro_window->Terminate();
            delete m_tyro_window; 
        }

        if (m_gl_rend) 
            delete m_gl_rend;
        
        if (m_camera)
            delete m_camera;
    }
    
    int App::Setup() 
    {
          //setup windowshapes
        m_tyro_window = new Window();
        m_tyro_window->Init(1600,1200);
                
        //setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        m_gl_rend->SetClearColor(Wm5::Vector4f(20/255.0, 20/255.0, 20/255.0, 1));

        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(Wm5::APoint(0,0,0), 1.0, 1.0, 2, viewport, true);
        
        m_timeline = new Timeline(24, 300);
        m_timeline->frameChanged = [&](Timeline& timeline, int frame)->void 
        {   
            //RA_LOG_INFO("Frame Change BEGIN");
            m_frame = frame;
            m_need_rendering = true;
            //glfwPostEmptyEvent();
            //RA_LOG_INFO("Frame Change END");
        };
        
       
        //set up window callbacks
        //@TODO use std::bind instead
        m_tyro_window->callback_mouse_down = [&](Window& window, int button, int modifier)->bool 
        {
            this->mouse_down(window, button, modifier);
        };

        m_tyro_window->callback_mouse_up = [&](Window& window, int button, int modifier)->bool 
        {
            this->mouse_up(window, button, modifier);
        };

        m_tyro_window->callback_mouse_move = [&](Window& window, int mouse_x, int mouse_y)->bool 
        {
            this->mouse_move(window, mouse_x, mouse_y);
        };

        m_tyro_window->callback_window_resize = [&](Window& window, unsigned int w, unsigned int h)->bool 
        {
            this->window_resize(window, w, h);
        };

        m_tyro_window->callback_key_pressed = [&](Window& window, unsigned int key, int modifiers)->bool 
        {
            this->key_pressed(window, key, modifiers);
        };

        m_tyro_window->callback_key_down = [&](Window& window, unsigned int key, int modifiers)->bool 
        {
            this->key_down(window, key, modifiers);
        };
        
        m_tyro_window->callback_mouse_scroll = [&](Window& window, float ydelta)->bool 
        {
            this->mouse_scroll(window, ydelta);
        };
     
        m_state = App::State::Launched;
        // Loop until the user closes the window
       // m_tyro_window->GetGLContext()->swapBuffers();
        
        m_need_rendering = true;
        //render(); //do initial render

        //while (!m_tyro_window->ShouldClose()) 
       // {
            //RA_LOG_INFO("Looping GLFW");
        //    m_tyro_window->Wait();
        //}


        register_console_function("render_to_image", console_render_to_image, ""); 
        register_console_function("render_to_images", console_render_to_images, ""); 



        FontManager* fManager = FontManager::GetSingleton();
        float scale = 1;
        float ppi = 144;
        fManager->Setup(ppi, scale);

          // show current number of frames
        ES2FontSPtr font = FontManager::GetSingleton()->GetSystemFontOfSize12();
        
        m_frame_overlay = ES2TextOverlay::Create(std::string("Not initialized text"), 
                                                 Wm5::Vector2f(0, 0), 
                                                 font, 
                                                 Wm5::Vector4f(0,0,1,1), 
                                                 viewport);
                                                 
        m_frame_overlay->SetTranslate(Wm5::Vector2i(-viewport[2]/2 + 50,-viewport[3]/2 + 50));
    
        //setup imgui
       // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(m_tyro_window->GetGLFWWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 400");

        //load all bshapes and neuteral expression
        mFaceModel.setNeuteralMesh(NEUT);
        mFaceModel.setBshapes(BSHAPES_MAP); // ORDER IS VERY IMPORTANT to match BSHAPES in arig.py

        //load neuteral expression
        Eigen::MatrixXd V, N;
        Eigen::MatrixXi F;
        mFaceModel.getExpression(V, F, N);
        RENDER.mesh = IGLMesh::Create(V, F, N, MESH_COLOR);
    }
    
    int App::Launch()
    {   
        RA_LOG_INFO("Launching the app");

        Setup();

        //load animation
        loadAnimation(ANIM_LIST[0]);
                
        m_update_camera = true;
        m_state = App::State::LoadedModel;

        while (!m_tyro_window->ShouldClose())
        {   
            m_tyro_window->ProcessUserEvents();

            //if (m_need_rendering) 
            {   
                
                DrawUI();
                //if (m_state == App::State::Launched) 
                //{
                    m_gl_rend->ClearScreen();
                //}
                //else if (m_state == App::State::LoadedModel) 
                //{   
                    //if (m_need_rendering)
                    DrawMeshes();
                //}
                
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                // Draw console
                if (show_console) 
                {
                    glUseProgram(0);
                    m_console.display(2);
                }
                // Poll for and process events
                m_tyro_window->GetGLContext()->swapBuffers();
                m_need_rendering = false;             
            }
        
            //m_tyro_window->Wait();
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

	    return 0;
    }

    void App::DrawUI()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //if (show_demo_window)
        //ImGui::ShowDemoWindow();

        if (ImGui::TreeNode("Animations"))
        {
            //ShowHelpMarker("This is a more standard looking tree with selectable nodes.\nClick to select, CTRL+Click to toggle, click on arrows or double-click to open.");
            //static bool align_label_with_current_x_position = false;
            //ImGui::Checkbox("Align label with current X position)", &align_label_with_current_x_position);
            //ImGui::Text("Hello!");
            //if (align_label_with_current_x_position)
                //ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize()*3); // Increase spacing to differentiate leaves from expanded contents.
            for (int i = 0; i < ANIM_LIST.size(); i++)
            {
                // Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
                ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((file_selected == i) ? ImGuiTreeNodeFlags_Selected : 0);
                // Leaf: The only reason we have a TreeNode at all is to allow selection of the leaf. Otherwise we can use BulletText() or TreeAdvanceToLabelPos()+Text().
                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
                auto p = filesystem::path(ANIM_LIST[i]);
                ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "%i %s", i, p.basename().c_str());
                if (ImGui::IsItemClicked()) 
                {
                    file_selected = i;
                    RA_LOG_INFO("file selected %i", file_selected);      
                }      
            }

            ImGui::PopStyleVar();
            if(ImGui::Button("Load Animation"))
            {   
                if (file_selected >= 0)
                    RA_LOG_INFO("load animation %s", ANIM_LIST[file_selected].c_str());
                
                loadAnimation(ANIM_LIST[file_selected]);
                m_frame = 0;
            }
            //if (align_label_with_current_x_position)
            //    ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
            ImGui::TreePop();
        }
        
        //ImGui::End();
        //ImGui::Begin("Animations", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        // ImGui::Text("Hello from another window!");
        //ImGui::End();
        
        ImGui::Render();
    }

    void App::DrawMeshes() 
    {
        //RA_LOG_INFO("RENDER BEGIN");
        VisibleSet vis_set;

        loadFrame(m_frame);
        
        // update face geometry
        Eigen::MatrixXd V, N;
        Eigen::MatrixXi F;
        mFaceModel.getExpression(V, F, N);
        RENDER.mesh->UpdateData(V, F, N, MESH_COLOR);
        RENDER.mesh->Update(true);
        vis_set.Insert(RENDER.mesh.get());

        // update timeline text
        std::string fstr = std::string("Frame ") + std::to_string(m_frame) + std::string("/") + std::to_string(mCurAnimation.getNumFrames());
        m_frame_overlay->SetText(fstr);
        vis_set.Insert(m_frame_overlay.get());
        
        if (m_update_camera) 
        {
            update_camera();
            m_update_camera = false;
        }
        
        m_gl_rend->RenderVisibleSet(&vis_set, m_camera);       
    }

    void App::loadFrame(int frame) 
    {
        std::vector<double> W;
        std::vector<std::string> A;
        mCurAnimation.getWeights(frame, A, W);
        mFaceModel.setWeights(A, W);
    }

    void App::loadAnimation(const std::string& name) 
    {
        mCurAnimation.readPandasCsv(name, 0);
        //mCurAnimation.setCustomAttrs(mFaceModel.getBnames());
        //m_timeline->Stop();
        m_timeline->SetFrameRange(mCurAnimation.getNumFrames()-1);
        m_timeline->SetFrame(0);
    }

    void App::render() 
    {   
        //RA_LOG_INFO("NEED RENDERING");
        m_need_rendering = true;
    }

    void App::update_camera() 
    {
        //setup camera
        AxisAlignedBBox WorldBoundBox = RENDER.mesh->WorldBoundBox;
        Wm5::APoint world_center = WorldBoundBox.GetCenter();
        float radius = std::abs(WorldBoundBox.GetRadius()*1.5);
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

    void App::mouse_down(Window& window, int button, int modifier) 
    {   
        //RA_LOG_INFO("mouse down %i", button);
        if (m_state != App::State::LoadedModel) return;

        mouse_is_down = true;
        m_modifier = modifier;
        m_mouse_btn_clicked = button;

        if (m_modifier == TYRO_MOD_CONTROL) return; //rotating
        if (button == 0 && m_modifier == TYRO_MOD_SHIFT) 
        {   
            //m_square_sel_start_x = current_mouse_x;
            //m_square_sel_start_y = current_mouse_y;
            return; //selection
        }
        if (m_mouse_btn_clicked == 2) return; //translating
        
        // Cast a ray in the view direction starting from the mouse position
        double x = current_mouse_x;
        double y = m_camera->GetViewport()[3] - current_mouse_y;
        //Eigen::Vector2f mouse_pos(x,y);
        //selectVertex(mouse_pos, button, modifier);     
    }

    void App::mouse_up(Window& window, int button, int modifier) 
    {   
        if (m_state != App::State::LoadedModel) return;
        //RA_LOG_INFO("MOUSE_UP");
        if (mouse_is_down && m_modifier == TYRO_MOD_CONTROL) 
        {   
            gesture_state = 2;
            m_camera->HandleOneFingerPanGesture(gesture_state, Wm5::Vector2i(current_mouse_x, current_mouse_y));
            render();
        }
        else if (mouse_is_down && m_mouse_btn_clicked == 2) 
        {
            gesture_state = 2;
            m_camera->HandleTwoFingerPanGesture(gesture_state, Wm5::Vector2i(current_mouse_x, -current_mouse_y));
            render();
        }
        
        mouse_is_down = false;
        gesture_state = 0;
    }

    void App::mouse_move(Window& window, int mouse_x, int mouse_y) 
    {   
        //RA_LOG_INFO("mouse move state %i", m_state);
        if (m_state != App::State::LoadedModel) return;
        
        current_mouse_x = mouse_x;
        current_mouse_y = mouse_y;

        if (mouse_is_down && m_modifier == TYRO_MOD_CONTROL) 
        {   
            m_camera->HandleOneFingerPanGesture(gesture_state, Wm5::Vector2i(mouse_x, mouse_y));
            gesture_state = 1;
            render();
        } 
        else if (mouse_is_down && m_mouse_btn_clicked == 2) 
        {
            m_camera->HandleTwoFingerPanGesture(gesture_state, Wm5::Vector2i(mouse_x, -mouse_y));
            gesture_state = 1;
            render();
        } 
    }

    void App::mouse_scroll(Window& window, float ydelta) 
    {   
        //RA_LOG_INFO("mouse scroll delta %f", ydelta);
        if (m_state != App::State::LoadedModel) return;
        
        m_camera->HandlePinchGesture(gesture_state, Wm5::Vector2i(current_mouse_x, current_mouse_y), ydelta);
        render();
    } 

    void App::window_resize(Window& window, unsigned int w, unsigned int h)
    {
        //RA_LOG_INFO("window resized")
        //float  aspect = m_gl_rend->GetViewWidth()/ (float) m_gl_rend->GetViewHeight();
        //Vector4i viewport(0, 0, m_gl_rend->GetViewWidth(), m_gl_rend->GetViewHeight());
        //m_camera->SetAspect(aspect);
        //m_camera->SetViewport(viewport);
        
        render();
    }

    void App::key_pressed(Window& window, unsigned int key, int modifiers) 
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
    
    void App::key_down(Window& window, unsigned int key, int modifiers) 
   {   
        //RA_LOG_INFO("Key down %i", key)
        // handle not text keys   

        if (key == TYRO_KEY_LEFT){ 
            m_console.key_left();
            render();}
        else if (key == TYRO_KEY_RIGHT){ 
            m_console.key_right();
            render();}
        else if (key == TYRO_KEY_ENTER){
            m_console.key_enter();
            render();
            }
        else if (key == TYRO_KEY_BACKSPACE){
            m_console.key_backspace();render();}
        else if (key == TYRO_KEY_UP){
            m_console.key_up();render();}
        else if (key == TYRO_KEY_DOWN){
            m_console.key_down();render();}
        else if (key == TYRO_KEY_TAB){
            m_console.key_tab();render();}
    }

    void App::register_console_function(const std::string& name,
                                        const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                        const std::string& help_txt)
    {
        std::function<void (const std::vector<std::string>&)> f = bind(con_fun, this, std::placeholders::_1);
        m_console.reg_cmdN(name, f, help_txt);
    }
}