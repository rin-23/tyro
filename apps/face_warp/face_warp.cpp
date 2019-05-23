#include "face_warp.h"
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
#include <igl/readOBJ.h>
#include <igl/per_vertex_normals.h>
#include <igl/unique_edge_map.h>
#include <igl/unproject_onto_mesh.h>
#include "RAES2StandardMesh.h"

using namespace std;

using Wm5::APoint;

namespace tyro
{   
    App::App()
    :
    m_tyro_window(nullptr),
    m_gl_rend(nullptr),
    m_camera(nullptr),
    mouse_is_down(false),
    gesture_state(0),
    show_console(false),
    //m_frame(0),
    m_state(App::State::None),
    m_need_rendering(false),
    m_computed_deformation(false),
    m_computed_avg(false),
    //m_sel_primitive(App::SelectionPrimitive::Faces),
    //m_sel_method(App::SelectionMethod::OneClick),
    m_computed_stop_motion(false),
    m_update_camera(false),
    //m_frame_overlay(nullptr),
    m_computed_parts(false),
    m_show_wire(true)
    //add_seg_faces(false),
    //m_video_texture(nullptr),
    //m_frame_offset(0)
    {}

    void color_matrix(int rows, const Eigen::Vector3d& cv, Eigen::MatrixXd& uC)
    {
        uC.resize(rows, 3);
        for (int e = 0; e<uC.rows(); ++e) 
        {
            uC.row(e) = cv;
        }
    }

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
    
    int App::Launch()
    {   
        RA_LOG_INFO("Launching the app");

        //setup windowshapes
        m_tyro_window = new Window();
        m_tyro_window->Init();
                
        //setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        m_gl_rend->SetClearColor(Wm5::Vector4f(0, 0, 1, 1));

        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(Wm5::APoint(0,0,0), 1.0, 1.0, 2, viewport, true);
        
        /* m_timeline = new Timeline(40, 300);
        m_timeline->frameChanged = [&](Timeline& timeline, int frame)->void 
        {   
            //RA_LOG_INFO("Frame Change BEGIN");
            m_frame = frame;
            m_need_rendering = true;
            glfwPostEmptyEvent();
            //RA_LOG_INFO("Frame Change END");
        };
        */
       
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
        // FontManager* fManager = FontManager::GetSingleton();
        // float scale = 1;
        // float ppi = 144;
        // fManager->Setup(ppi, scale);

        // ES2FontSPtr font = FontManager::GetSingleton()->GetSystemFontOfSize12();
        // std::string strrr("Framasdasdasdsaddasde 0/9000");
        // m_frame_overlay = ES2TextOverlay::Create(strrr, 
        //                                          Wm5::Vector2f(0, 0), 
        //                                          font, 
        //                                          Wm5::Vector4f(0,0,1,1), 
        //                                          viewport);
                                                 
        // m_frame_overlay->SetTranslate(Wm5::Vector2i(-viewport[2]/2 ,-viewport[3]/2 ));
        // m_frame_overlay->SetText(strrr);
        
        //m_shaderbox = ShaderBox::Create();

        // load obj file
        //Eigen::MatrixXd V,N; // Vertex data. 3*num_vert by num_frames. 
        Eigen::MatrixXd UEC;
        Eigen::MatrixXi E,UE; // Face data. 
        Eigen::VectorXi EMAP; // Map directed edges to unique edges. 
        std::vector<std::vector<int> > uE2E;

        std::string obj_list = "/Users/rinat/Workspace/TemplateDeform/data/scan.obj";
        //tyro::load_mesh(obj_list, V, N, F, E, UE, EMAP);
        igl::readOBJ(obj_list, GEOMETRY.V,GEOMETRY.F);
        igl::per_vertex_normals(GEOMETRY.V,GEOMETRY.F,GEOMETRY.N);
        igl::unique_edge_map(GEOMETRY.F,E,UE,EMAP,uE2E);

        Eigen::Vector3d clr(1,0,0);
        RENDER.mesh = IGLMesh::Create(GEOMETRY.V, GEOMETRY.F, GEOMETRY.N, clr);

        color_matrix(UE.rows(), Eigen::Vector3d(0.5,0.5,0.5), UEC);
        RENDER.mesh_wire = IGLMeshWireframe::Create(GEOMETRY.V, UE, UEC);
        
        // addSphere(100, GEOMETRY.V, Wm5::Vector4f(0,1,0,1), RENDER.mesh->WorldTransform);
        // addSphere(200, GEOMETRY.V, Wm5::Vector4f(0,1,0,1), RENDER.mesh->WorldTransform);
        // addSphere(300, GEOMETRY.V, Wm5::Vector4f(0,1,0,1), RENDER.mesh->WorldTransform);
        // addSphere(400, GEOMETRY.V, Wm5::Vector4f(0,1,0,1), RENDER.mesh->WorldTransform);

        
        m_update_camera = true;
        m_state = App::State::LoadedModel;

        while (!m_tyro_window->ShouldClose())
        {   
            if (m_need_rendering) 
            {
                if (m_state == App::State::Launched) 
                {
                    m_gl_rend->ClearScreen();
                }
                else if (m_state == App::State::LoadedModel) 
                {   
                   DrawMeshes();
                }
                
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
            
            m_tyro_window->Wait();
            //m_tyro_window->ProcessUserEvents();

        }

	    return 0;
    }

    
    void App::DrawMeshes() 
    {
        //RA_LOG_INFO("RENDER BEGIN");

        //create renderable for mesh
        VisibleSet vis_set;
   
        //std::string fstr = std::string("Frame ") + std::to_string(m_frame) + std::string("/") + std::to_string(ANIM.VD.size());
        //fstr =fstr+ std::string(" Scene ") + std::to_string(scene_id);
        //m_frame_overlay->SetText(fstr);
        
        RENDER.mesh->Update(true);
        vis_set.Insert(RENDER.mesh.get());

        RENDER.mesh_wire->Update(true);
        vis_set.Insert(RENDER.mesh_wire.get());

        for (auto object_sptr : ball_list) 
        {   
            vis_set.Insert(object_sptr.get());
        }
        //vis_set.Insert(m_frame_overlay.get());
        //vis_set.Insert(m_shaderbox.get());
        if (m_update_camera) 
        {
            update_camera();
            m_update_camera = false;
        }
        
        m_gl_rend->RenderVisibleSet(&vis_set, m_camera);       
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
        float radius = std::abs(WorldBoundBox.GetRadius()*5);
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
        else if (mouse_is_down && m_modifier == TYRO_MOD_SHIFT ) 
        {
            // Cast a ray in the view direction starting from the mouse position
            // double x = current_mouse_x;
            // double y = ;
            Eigen::Vector2f mouse_pos(current_mouse_x, m_camera->GetViewport()[3] - current_mouse_y);
            RA_LOG_INFO("mouse up  %f, %f %f", m_camera->GetViewport()[3], mouse_pos[0],mouse_pos[1]);

            selectVertex(mouse_pos, m_mouse_btn_clicked, m_modifier);
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
        
        if (m_state != App::State::LoadedModel) return;
        current_mouse_x = mouse_x;
        current_mouse_y = mouse_y;
        //RA_LOG_INFO("mouse move state %i %i %i", m_state,current_mouse_x, current_mouse_y);
        
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

    void App::selectVertex(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier) 
    {   
        RA_LOG_INFO("here1");

        int fid;
        Eigen::Vector3f bc;
        Wm5::HMatrix modelViewMatrix = m_camera->GetViewMatrix() * RENDER.mesh->WorldTransform.Matrix();
        Wm5::HMatrix projectMatrix = m_camera->GetProjectionMatrix();
        Eigen::Matrix4f e1 = Eigen::Map<Eigen::Matrix4f>(modelViewMatrix.mEntry);
        Eigen::Matrix4f e2 = Eigen::Map<Eigen::Matrix4f>(projectMatrix.mEntry);
        Eigen::Vector4f e3 = Eigen::Vector4f(m_camera->GetViewport()[0],
                                             m_camera->GetViewport()[1],
                                             m_camera->GetViewport()[2],
                                             m_camera->GetViewport()[3]);

        RA_LOG_INFO("here2");
        RA_LOG_INFO("%f %f ",mouse_pos[0],mouse_pos[1]);
        Eigen::Vector2f mouse_pos2(current_mouse_x, current_mouse_y);
        //RA_LOG_INFO("%f %f ",mouse_pos2[0],mouse_pos2[1]);
        if (igl::unproject_onto_mesh(mouse_pos, 
                                     e1.transpose(),
                                     e2.transpose(),
                                     e3,
                                     GEOMETRY.V,
                                     GEOMETRY.F,
                                     fid,
                                     bc)) 
        {   
            RA_LOG_INFO("here3");

            if (true) 
            {   
                RA_LOG_INFO("here4");
                long c;
                bc.maxCoeff(&c);
                int vid = GEOMETRY.F(fid, c);
                auto it = std::find(vid_list.begin(), vid_list.end(), vid);
                if (it == vid_list.end()) 
                {       
                    Eigen::Vector3d vec = GEOMETRY.V.row(vid);
                    RA_LOG_INFO("Picked face_id %i vertex_id %i coord %f %f %f", fid, vid, vec(0), vec(1), vec(2));
                    //addSphere(vid,  GEOMETRY.V);
                    addSphere(vid, GEOMETRY.V, Wm5::Vector4f(0,1,0,1), RENDER.mesh->WorldTransform);

                    add_vertex(vid);
                }  
                else
                {   
                    RA_LOG_INFO("remove vertex %i %i", fid, vid);
                    auto index = std::distance(vid_list.begin(), it);
                    ball_list.erase(ball_list.begin() + index);
                    vid_list.erase(vid_list.begin() + index);
                }
                render(); 
            } 
            /*
            else if (m_sel_primitive == App::SelectionPrimitive::Faces) 
            {   
                if (mouse_button == 0) 
                {
                    auto it = std::find(fid_list.begin(), fid_list.end(), fid);
                    if (m_selection_type == Select) 
                    {
                        if (it == fid_list.end()) 
                        {
                            add_face(fid);
                            setFaceColor(fid, true); 
                        }  
                        else
                        {   
                            auto index = std::distance(fid_list.begin(), it);
                            fid_list.erase(fid_list.begin() + index);
                            setFaceColor(fid, false);
                        }
                    }
                    else 
                    {
                        if (it != fid_list.end()) 
                        {
                            auto index = std::distance(fid_list.begin(), it);
                            fid_list.erase(fid_list.begin() + index);
                            setFaceColor(fid, false);
                        }
                    }
                } 
                else if (mouse_button == 1 && modifier == TYRO_MOD_NONE) 
                {
                    auto it = std::find(fid_list2.begin(), fid_list2.end(), fid);
                    if (it == fid_list2.end()) 
                    {
                        fid_list2.push_back(fid);
                        setFaceColor(fid, Eigen::Vector3d(0.5,0,0)); 
                    }  
                    else
                    {   
                        auto index = std::distance(fid_list2.begin(), it);
                        fid_list2.erase(fid_list2.begin() + index);
                        setFaceColor(fid, false);
                    }
                }
                else if (mouse_button == 1 && modifier == TYRO_MOD_SHIFT) 
                {
                    auto it = std::find(fid_list3.begin(), fid_list3.end(), fid);
                    if (it == fid_list3.end()) 
                    {
                        fid_list3.push_back(fid);
                        setFaceColor(fid, Eigen::Vector3d(0, 1.0, 0)); 
                    }  
                    else
                    {   
                        auto index = std::distance(fid_list3.begin(), it);
                        fid_list3.erase(fid_list3.begin() + index);
                        setFaceColor(fid, false);
                    }
                }
                render();
            } 
            */                 
        }   
    }

    void App::add_vertex(int vid)
    {   
        auto it = std::find(vid_list.begin(), vid_list.end(), vid);
        if (it == vid_list.end())
        {
            vid_list.push_back(vid);
        }        
    }

    void App::addSphere(int vid, const Eigen::MatrixXd& V, Wm5::Vector4f color, Wm5::Transform world) 
    {   
        Eigen::RowVector3d new_c = V.row(vid);
        //float s =  (V.size() > 3000) ? 0.0005 : 0.007;
        //float s = 0.001;
        float m_ball_size = 0.005;
        ES2SphereSPtr object = ES2Sphere::Create(10, 10, m_ball_size);
        Wm5::Transform tr;
        tr.SetTranslate(Wm5::APoint(new_c(0), new_c(1), new_c(2)));
        object->LocalTransform = world * tr * object->LocalTransform;
        object->Update(true);
        object->SetColor(color);
        ball_list.push_back(object);
    }     

}