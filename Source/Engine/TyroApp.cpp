#include "TyroApp.h"
#include "RAEnginePrerequisites.h"
#include "Wm5APoint.h"
#include "Wm5Vector3.h"
#include "RAVisibleSet.h"
#include "RAES2StandardMesh.h"
#include <functional>
#include <igl/readOBJ.h>
#include <igl/per_vertex_normals.h>
#include "TyroIGLMesh.h"
#include <igl/unproject_onto_mesh.h>
#include <igl/min_quad_with_fixed.h>
#include <igl/cotmatrix.h>
#include <igl/massmatrix.h>
#include "stop_motion_data.h"

using namespace Wm5;
using namespace std;

namespace tyro
{   
    namespace
    {   
        void console_load_oldman(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Loading oldman obj sequence");

            if (args.size() == 0)
            {   
                app->load_oldman();
                return;
            }
        }
        
        void console_load_bunny(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Loading bunny obj sequence");

            if (args.size() == 0)
            {   
                app->load_bunny();
                return;
            }
        }

        void console_load_blobby(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Loading blobby obj sequence");

            if (args.size() == 0)
            {   
                app->load_blobby();
                return;
            }
        }

        void console_compute_average(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("compute_average");

            if (args.size() == 0)
            {   
                app->compute_average();
                return;
            }
        }

        void console_compute_deformation(App* app, const std::vector<std::string> & args)
        {
            RA_LOG_INFO("deform");

            if (args.size() == 0)
            {   
                app->compute_deformation();
                return;
            }
        }
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
    m_computed_deformation(false),
    m_computed_avg(false),
    m_sel_mode(App::SelectionMode::Faces)
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

    int App::Launch()
    {   
        RA_LOG_INFO("Launching the app");

        //setup windowshapes
        m_tyro_window = new Window();
        m_tyro_window->Init();
                
        //setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        m_gl_rend->SetClearColor(Vector4f(0.0, 153.0/255.0, 153.0/255.0, 1.0));
 
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(APoint(0,0,0), 1.0, 1.0, 2, viewport, true);
        
        m_timeline = new Timeline(24, 300);
        m_timeline->frameChanged = [&](Timeline& timeline, int frame)->void 
        {   
            RA_LOG_INFO("Frame Change BEGIN");
            m_frame = frame;
            m_need_rendering = true;
            glfwPostEmptyEvent();
            RA_LOG_INFO("Frame Change END");
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

        register_console_function("load_blobby", console_load_blobby, "");
        register_console_function("load_oldman", console_load_oldman, "");
        register_console_function("load_bunny", console_load_bunny, "");
        register_console_function("compute_average", console_compute_average, "");
        register_console_function("compute_deformation", console_compute_deformation, "");

        m_state = App::State::Launched;
        // Loop until the user closes the window
        m_tyro_window->GetGLContext()->swapBuffers();
        
        m_need_rendering = true;
        //render(); //do initial render

        //while (!m_tyro_window->ShouldClose()) 
       // {
            //RA_LOG_INFO("Looping GLFW");
        //    m_tyro_window->Wait();
        //}
        
        while (!m_tyro_window->ShouldClose())
        {   
            if (m_need_rendering) 
           {
                RA_LOG_INFO("RENDER BEGIN");

                if (m_state == App::State::Launched) 
                {
                    m_gl_rend->ClearScreen();
                }
                else if (m_state == App::State::LoadedModel) 
                {  
                    //create renderable for mesh
                    VisibleSet vis_set;

                    if (!m_computed_deformation) 
                    {
                        //ORIGNAL MODEL
                        //Eigen::Vector3d cr(0.5,0.5,0.5);
                        render_data.org_mesh = IGLMesh::Create(m_frame_data.v_data[m_frame], 
                                                               m_frame_data.f_data, 
                                                               m_frame_data.n_data[m_frame],
                                                               m_frame_data.c_data);
                        render_data.org_mesh->Update(true);
                        vis_set.Insert(render_data.org_mesh.get());

                        //create renderable for mesh wireframe
                        Eigen::Vector3d cr2(0,0,0);
                        render_data.org_mesh_wire = IGLMeshWireframe::Create(m_frame_data.v_data[m_frame], 
                                                                             m_frame_data.f_data,
                                                                             cr2);
                        render_data.org_mesh_wire->Update(true);
                        vis_set.Insert(render_data.org_mesh_wire.get());
                    }
                    //DEFORMED MODEL
                    if (m_computed_deformation)
                    {
                        Eigen::Vector3d cr(0.5,0.5,0.5);
                        render_data.dfm_mesh = IGLMesh::Create(m_frame_deformed_data.v_data[m_frame], 
                                                               m_frame_deformed_data.f_data, 
                                                               m_frame_data.n_data[m_frame],
                                                               cr);
                        render_data.dfm_mesh->Update(true);
                        vis_set.Insert(render_data.dfm_mesh.get());

                        //create renderable for mesh wireframe
                        Eigen::Vector3d cr2(0,0,0);
                        render_data.dfm_mesh_wire = IGLMeshWireframe::Create(m_frame_deformed_data.v_data[m_frame], 
                                                                             m_frame_deformed_data.f_data,
                                                                             cr2);
                        render_data.dfm_mesh_wire->Update(true);
                        vis_set.Insert(render_data.dfm_mesh_wire.get());
                    }

                    if (m_computed_avg) 
                    {
                        vis_set.Insert(render_data.avg_mesh.get());
                        vis_set.Insert(render_data.avg_mesh_wire.get());
                    }

                    for (auto object_sptr : ball_list) 
                    {
                        vis_set.Insert(object_sptr.get());
                    }

                    m_gl_rend->RenderVisibleSet(&vis_set, m_camera);         
                }
                
                // Draw console
                if (show_console) 
                {
                    glUseProgram(0);
                    m_console.display(1);
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

    void App::render() 
    {   
        RA_LOG_INFO("NEED RENDERING");
        m_need_rendering = true;
    }

    void App::compute_deformation()
    {
        assert(vid_list.size() > 0);
        
        m_frame_deformed_data.v_data.reserve(m_frame_data.v_data.size());
        m_frame_deformed_data.f_data = m_frame_data.f_data;

        int num_frames = m_frame_data.v_data.size();
        for (int i =0; i < num_frames; ++i) 
        {   
            RA_LOG_INFO("Compute deformation for mesh %i out of %i", i, num_frames)
            Eigen::MatrixXd V = m_frame_data.v_data[i];
            Eigen::MatrixXi F = m_frame_data.f_data;
            Eigen::VectorXi b;
            b.resize(vid_list.size());
            int b_index = 0;
            
            for (int vid : vid_list)
            {
                b(b_index++) = vid;
                //RA_LOG_INFO("vide %i", vid)
            }
           
            //do biharmonic stuff here.
            igl::min_quad_with_fixed_data<double> data;
            Eigen::SparseMatrix<double> L;
            
            igl::cotmatrix(V, F, L);

            Eigen::SparseMatrix<double> M;	
            igl::massmatrix(V, F, igl::MASSMATRIX_TYPE_DEFAULT, M);

            //Computer M inverse
            Eigen::SimplicialLDLT <Eigen::SparseMatrix<double> > solver;
            solver.compute(M);
            Eigen::SparseMatrix<double> I(M.rows(), M.cols());
            I.setIdentity();
            Eigen::SparseMatrix<double> M_inv = solver.solve(I);
            Eigen::SparseMatrix<double> Q = L.transpose() * M_inv * L;
            Eigen::SparseMatrix<double> Aeq;
            Q = 2 * Q;
            bool result = igl::min_quad_with_fixed_precompute(Q, b, Aeq, false, data);
            assert(result);

            Eigen::MatrixXd D;
            Eigen::MatrixXd bc;
            bc.resize(vid_list.size(), 3);
            b_index = 0;
            for (int vid : vid_list)
            {
                bc.row(b_index++) = m_frame_data.avg_v_data.row(vid) - m_frame_data.v_data[i].row(vid);
            }

            Eigen::MatrixXd B = Eigen::MatrixXd::Zero(data.n, bc.cols());
            Eigen::VectorXd Beq;
            //Beq.resize(bc.)
            result = igl::min_quad_with_fixed_solve(data, B, bc, Beq, D);
            assert(result);

            Eigen::MatrixXd V_prime = V + D;
            m_frame_deformed_data.v_data.push_back(V_prime);
            
        }
        m_computed_deformation = true;
        
    }

    void App::compute_average()
    {
        if (m_frame_data.v_data.size()==0)
            RA_LOG_ERROR_ASSERT("cant compute average");
        
        m_frame_data.avg_v_data.resize(m_frame_data.v_data[0].rows(), 
                                       m_frame_data.v_data[0].cols());
        m_frame_data.avg_v_data.setZero();

        for (auto& mat : m_frame_data.v_data) 
        {
            m_frame_data.avg_v_data += mat;
        }
        m_frame_data.avg_v_data = (1.0/m_frame_data.v_data.size()) * m_frame_data.avg_v_data;
        
        Eigen::Vector3d cr(0.5,0,0);
        render_data.avg_mesh = IGLMesh::Create(m_frame_data.avg_v_data, 
                                               m_frame_data.f_data, 
                                               m_frame_data.n_data[0],
                                               cr);
        render_data.avg_mesh->Update(true);

        Wm5::Transform tr;
        tr.SetTranslate(APoint(render_data.avg_mesh->WorldBoundBox.GetRadius(), 0, 0));
        render_data.avg_mesh->LocalTransform = tr * render_data.avg_mesh->LocalTransform;
        render_data.avg_mesh->Update(true);

        Eigen::Vector3d cr2(0,0,0);
        render_data.avg_mesh_wire = IGLMeshWireframe::Create(m_frame_data.avg_v_data, m_frame_data.f_data, cr2);
        render_data.avg_mesh_wire->Update(true);

        render_data.avg_mesh_wire->LocalTransform = tr * render_data.avg_mesh_wire->LocalTransform;
        render_data.avg_mesh_wire->Update(true);

        m_computed_avg = true;
    }

    void App::update_camera(const AxisAlignedBBox& WorldBoundBox) 
    {
        //setup camera
        APoint world_center = WorldBoundBox.GetCenter();
        float radius = std::abs(WorldBoundBox.GetRadius()*2);
        float aspect = 1.0;
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Vector4i viewport(0, 0, v_width, v_height);
        
        if (m_camera)
            delete m_camera;
        
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
    }

    void App::load_mesh_sequence(const std::string& obj_list_file, bool use_igl_loader) 
    {   
        RA_LOG_INFO("LOAD MESH SEQUENCE")
        stop::load_mesh_sequence(obj_list_file, 
                                 m_frame_data.v_data, 
                                 m_frame_data.n_data, 
                                 m_frame_data.f_data,
                                 use_igl_loader);
        
        //@TODO need this to update camera
        Eigen::Vector3d cr(0.5,0.5,0.5);

        
        m_frame_data.c_data.resize(m_frame_data.v_data[0].rows(), 
                                   m_frame_data.v_data[0].cols());
        
        for (int i =0 ; i <  m_frame_data.v_data[0].rows(); ++i) 
        {
            m_frame_data.c_data.row(i) = cr;
        }

        IGLMeshSPtr tmp = IGLMesh::Create(m_frame_data.v_data[0], 
                                               m_frame_data.f_data, 
                                               m_frame_data.n_data[0], 
                                               cr);
        tmp->Update(true);        
        update_camera(tmp->WorldBoundBox);
       
        m_timeline->SetFrameRange(m_frame_data.v_data.size()-1);
    }

    void App::load_bunny()
    {
        RA_LOG_INFO("Load Bunny");
        auto obj_list_file = std::string("/home/rinat/Google Drive/StopMotionProject/BlenderOpenMovies/Production Files Archive Rinat/production/obj_export/02_rabbit/objlist2.txt");
        load_mesh_sequence(obj_list_file, false); //use tiny obj loader
        m_state = App::State::LoadedModel;
        render();
    }

    //load mexican blobby guy
    void App::load_blobby() 
    {   
        RA_LOG_INFO("LOAD_BLOBBY");
        auto obj_list_file = std::string("/home/rinat/Google Drive/StopMotionProject/Claymation/data/hello/FramesOBJ/objlist2.txt");
        load_mesh_sequence(obj_list_file);
        m_state = App::State::LoadedModel;
        render();
    }

    //load oldman (will you go to lunch)
    void App::load_oldman() 
    {   
        RA_LOG_INFO("LOAD OLDMAN");
        auto obj_list_file = std::string("/home/rinat/Google Drive/StopMotionProject/Claymation/data/oldman/gotolunch/FramesOBJ/FullFace/objlist2.txt");
        load_mesh_sequence(obj_list_file);
        m_state = App::State::LoadedModel;
        render();
    }

    void App::mouse_down(Window& window, int button, int modifier) 
    {   
        RA_LOG_INFO("mouse down %i", button);

        if (m_state != App::State::LoadedModel) return;

        mouse_is_down = true;
        m_modifier = modifier;
        m_mouse_btn_clicked = button;
        if (m_modifier == TYRO_MOD_CONTROL) return; //rotating
        if (m_mouse_btn_clicked == 2) return; //translating

        int fid;
        Eigen::Vector3f bc;
        // Cast a ray in the view direction starting from the mouse position
        double x = current_mouse_x;
        double y = m_camera->GetViewport()[3] - current_mouse_y;
        Eigen::Vector2f mouse_pos(x,y);
        Wm5::HMatrix modelViewMatrix = m_camera->GetViewMatrix() * render_data.org_mesh->WorldTransform.Matrix();
        Wm5::HMatrix projectMatrix = m_camera->GetProjectionMatrix();
        Eigen::Matrix4f e1 = Eigen::Map<Eigen::Matrix4f>(modelViewMatrix.mEntry);
        Eigen::Matrix4f e2 = Eigen::Map<Eigen::Matrix4f>(projectMatrix.mEntry);
        Eigen::Vector4f e3 = Eigen::Vector4f(m_camera->GetViewport()[0],
                                             m_camera->GetViewport()[1],
                                             m_camera->GetViewport()[2],
                                             m_camera->GetViewport()[3]);
        
        if (igl::unproject_onto_mesh(mouse_pos, 
                                     e1.transpose(),
                                     e2.transpose(),
                                     e3,
                                     m_frame_data.v_data[m_frame],
                                     m_frame_data.f_data,
                                     fid,
                                     bc))
        {
            if (m_sel_mode == App::SelectionMode::Vertex) 
            {
                long c;
                bc.maxCoeff(&c);
                int vid = m_frame_data.f_data(fid, c);
                auto it = std::find(vid_list.begin(), vid_list.end(), vid);
                if (it == vid_list.end()) 
                {
                    Eigen::RowVector3d new_c = m_frame_data.v_data[m_frame].row(vid);
                    RA_LOG_INFO("Picked face_id %i vertex_id %i", fid, vid);
                    ES2SphereSPtr object = ES2Sphere::Create(10, 10, 0.1);
                    Wm5::Transform tr;
                    tr.SetTranslate(APoint(new_c(0), new_c(1), new_c(2)));
                    object->LocalTransform = tr * object->LocalTransform;
                    object->Update(true);
                    ball_list.push_back(object);
                    vid_list.push_back(vid);
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
            else if (m_sel_mode == App::SelectionMode::Faces) 
            {
                auto it = std::find(fid_list.begin(), fid_list.end(), fid);
                if (it == fid_list.end()) 
                {
                    RA_LOG_INFO("Picked face_id %i", fid);
                    fid_list.push_back(fid);

                    Eigen::Vector3d clr(0,0,0.5);
                    Eigen::Vector3i vtx_idx = m_frame_data.f_data.row(fid);
                    m_frame_data.c_data.row(vtx_idx(0)) = clr;
                    m_frame_data.c_data.row(vtx_idx(1)) = clr;
                    m_frame_data.c_data.row(vtx_idx(2)) = clr;
                }  
                else
                {   
                    RA_LOG_INFO("remove face %i", fid);
                    auto index = std::distance(fid_list.begin(), it);
                    fid_list.erase(fid_list.begin() + index);
                    Eigen::Vector3d clr(0.5,0.5,0.5);
                    Eigen::Vector3i vtx_idx = m_frame_data.f_data.row(fid);
                    m_frame_data.c_data.row(vtx_idx(0)) = clr;
                    m_frame_data.c_data.row(vtx_idx(1)) = clr;
                    m_frame_data.c_data.row(vtx_idx(2)) = clr;
                }
                render();
            }                  
        };        
    }

    void App::mouse_up(Window& window, int button, int modifier) 
    {   
        if (m_state != App::State::LoadedModel) return;
        RA_LOG_INFO("MOUSE_UP");
        if (mouse_is_down&& m_modifier == TYRO_MOD_CONTROL) 
        {   
            gesture_state = 2;
            m_camera->HandleOneFingerPanGesture(gesture_state, Vector2i(current_mouse_x, current_mouse_y));
            render();
        }
        else if (mouse_is_down && m_mouse_btn_clicked == 2) 
        {
            gesture_state = 2;
            m_camera->HandleTwoFingerPanGesture(gesture_state, Vector2i(current_mouse_x, -current_mouse_y));
            render();
        }
        mouse_is_down = false;
        gesture_state = 0;
    }
    
    void App::mouse_move(Window& window, int mouse_x, int mouse_y) 
    {   
        RA_LOG_INFO("mouse move state %i", m_state);
        if (m_state != App::State::LoadedModel) return;
        
        RA_LOG_INFO("mouse move");
        current_mouse_x = mouse_x;
        current_mouse_y = mouse_y;

        if (mouse_is_down && m_modifier == TYRO_MOD_CONTROL) 
        {   
            m_camera->HandleOneFingerPanGesture(gesture_state, Vector2i(mouse_x, mouse_y));
            gesture_state = 1;
            render();
        } 
        else if (mouse_is_down && m_mouse_btn_clicked == 2) 
        {
            m_camera->HandleTwoFingerPanGesture(gesture_state, Vector2i(mouse_x, -mouse_y));
            gesture_state = 1;
            render();
        }
    }
    
    void App::window_resize(Window& window, unsigned int w, unsigned int h)
    {
        //RA_LOG_INFO("window resized")
        //float  aspect = m_gl_rend->GetViewWidth()/ (float) m_gl_rend->GetViewHeight();
        //Vector4i viewport(0, 0, m_gl_rend->GetViewWidth(), m_gl_rend->GetViewHeight());
        //m_camera->SetAspect(aspect);
        //m_camera->SetViewport(viewport);
    }

    void App::key_pressed(Window& window, unsigned int key, int modifiers) 
    {   
        RA_LOG_INFO("Key pressed %c", key);
        
        if (key == '`') 
        {   
            RA_LOG_INFO("Pressed %c", key);
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
        RA_LOG_INFO("Key down %i", key)
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