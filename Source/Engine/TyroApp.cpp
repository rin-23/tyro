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
    m_old_frame(0)
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

        //setup window
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
        register_console_function("compute_average", console_compute_average, "");
        register_console_function("compute_deformation", console_compute_deformation, "");
       
        // Loop until the user closes the window
        while (!m_tyro_window->ShouldClose())
        {   
            if (m_frame > m_old_frame) 
            {
                RA_LOG_INFO("RENDER BEGIN");
                // Render here 
                
                //create renderable for mesh
                object_list.clear(); //remove all previous data

                //create renderable for mesh
                igl_mesh = IGLMesh::Create(m_frame_data.v_data[m_frame], 
                                        m_frame_data.f_data, 
                                        m_frame_data.n_data[m_frame]);
                igl_mesh->Update(true);
                object_list.push_back(igl_mesh);

                //create renderable for mesh wireframe
                igl_mesh_wire = IGLMeshWireframe::Create(m_frame_data.v_data[m_frame], m_frame_data.f_data);
                igl_mesh_wire->Update(true);
                object_list.push_back(igl_mesh_wire);

                VisibleSet vis_set;
                for (auto object_sptr : object_list) {
                    vis_set.Insert(object_sptr.get());
                }

                for (auto object_sptr : ball_list) {
                    vis_set.Insert(object_sptr.get());
                }
            
                m_gl_rend->RenderVisibleSet(&vis_set, m_camera);
               
                
                RA_LOG_INFO("RENDER END");
                int tmp = m_frame;
                m_old_frame = tmp;
            }
            // Draw console
            if (show_console) 
            {
                glUseProgram(0);
                m_console.display(2);
            }

            // Poll for and process events
            m_tyro_window->GetGLContext()->swapBuffers();
            m_tyro_window->ProcessUserEvents();
        }

	    return 0;
    }

    void App::compute_deformation()
    {
        assert(vid_list.size() > 0);

        Eigen::MatrixXd V = m_frame_data.v_data[0];
        Eigen::MatrixXi F = m_frame_data.f_data;
        Eigen::VectorXi b;
        b.resize(vid_list.size());
        int b_index = 0;
        for (int vid : vid_list){
            b(b_index++) = vid;
            RA_LOG_INFO("vide %i", vid)
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

        //
        Eigen::MatrixXd D;
        Eigen::MatrixXd bc;
        bc.resize(vid_list.size(), 3);
        b_index = 0;
        for (int vid : vid_list)
        {
            bc.row(b_index++) = m_frame_data.avg_v_data.row(vid) - m_frame_data.v_data[0].row(vid);
        }

        Eigen::MatrixXd B = Eigen::MatrixXd::Zero(data.n, bc.cols());
	    Eigen::VectorXd Beq;
        //Beq.resize(bc.)
	    result = igl::min_quad_with_fixed_solve(data, B, bc, Beq, D);
        assert(result);

        Eigen::MatrixXd V_prime = V + D;
        IGLMeshSPtr prime_igl_mesh = IGLMesh::Create(V_prime, 
                                                     m_frame_data.f_data, 
                                                     m_frame_data.n_data[0]);
        prime_igl_mesh->Update(true);
        //object_list.push_back(prime_igl_mesh);

        //create renderable for mesh wireframe
        IGLMeshWireframeSPtr prime_igl_mesh_wire = IGLMeshWireframe::Create(V_prime, m_frame_data.f_data);
        prime_igl_mesh_wire->SetColor(Wm5::Vector4f(0,1,0,1));
        prime_igl_mesh_wire->Update(true);
        object_list.push_back(prime_igl_mesh_wire);
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
        

        IGLMeshSPtr avg_igl_mesh = IGLMesh::Create(m_frame_data.avg_v_data, 
                                                   m_frame_data.f_data, 
                                                   m_frame_data.n_data[0]);
        avg_igl_mesh->SetColor(Wm5::Vector4f(1,0,0,1));
        avg_igl_mesh->Update(true);
        //object_list.push_back(avg_igl_mesh);

        IGLMeshWireframeSPtr avg_igl_mesh_wire = IGLMeshWireframe::Create(m_frame_data.avg_v_data, m_frame_data.f_data);
        avg_igl_mesh_wire->SetColor(Wm5::Vector4f(1,0,0,1));
        avg_igl_mesh_wire->Update(true);
        object_list.push_back(avg_igl_mesh_wire);
    }

    void App::update_camera(const Spatial& spatial) 
    {
                //setup camera
        APoint world_center = spatial.WorldBoundBox.GetCenter();
        float radius = std::abs(spatial.WorldBoundBox.GetRadius()*2);
        float aspect = 1.0;
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Vector4i viewport(0, 0, v_width, v_height);
        
        if (m_camera)
            delete m_camera;
        
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
    }

    void App::load_mesh_sequence(const std::string& obj_list_file) 
    {   
        object_list.clear(); //remove all previous data
        
        stop::load_mesh_sequence(obj_list_file, 
                                 m_frame_data.v_data, 
                                 m_frame_data.n_data, 
                                 m_frame_data.f_data);
        //create renderable for mesh
        igl_mesh = IGLMesh::Create(m_frame_data.v_data[0], 
                                   m_frame_data.f_data, 
                                   m_frame_data.n_data[0]);
        igl_mesh->Update(true);
        object_list.push_back(igl_mesh);

        //create renderable for mesh wireframe
        igl_mesh_wire = IGLMeshWireframe::Create(m_frame_data.v_data[0], m_frame_data.f_data);
        igl_mesh_wire->Update(true);
        object_list.push_back(igl_mesh_wire);

        update_camera(*(igl_mesh.get()));
        m_timeline->SetFrameRange(m_frame_data.v_data.size());
    }

    //load mexican blobby guy
    void App::load_blobby() 
    {   
        auto obj_list_file = std::string("/home/rinat/Google Drive/StopMotionProject/Claymation/data/hello/FramesOBJ/objlist.txt");
        load_mesh_sequence(obj_list_file);
    }

    //load oldman (will you go to lunch)
    void App::load_oldman() 
    {   
        auto obj_list_file = std::string("/home/rinat/Google Drive/StopMotionProject/Claymation/data/oldman/gotolunch/FramesOBJ/FullFace/objlist.txt");
        load_mesh_sequence(obj_list_file);
    }

    void App::mouse_down(Window& window, int button, int modifier) 
    {   
        if (object_list.size() == 0) return;

        //RA_LOG_INFO("mouse down");
        mouse_is_down = true;
        m_modifier = modifier;

        if (m_modifier == TYRO_MOD_CONTROL) return; //rotating

        int fid;
        Eigen::Vector3f bc;
        // Cast a ray in the view direction starting from the mouse position
        double x = current_mouse_x;
        double y = m_camera->GetViewport()[3] - current_mouse_y;
        Eigen::Vector2f mouse_pos(x,y);
        Wm5::HMatrix modelViewMatrix = m_camera->GetViewMatrix() * igl_mesh->WorldTransform.Matrix();
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
                                     igl_mesh->V,
                                     igl_mesh->F,
                                     fid,
                                     bc))
        {
            
            long c;
            bc.maxCoeff(&c);
            int vid = igl_mesh->F(fid, c);
            auto it = std::find(vid_list.begin(), vid_list.end(), vid);
            if (it == vid_list.end()) 
            {
                Eigen::RowVector3d new_c = igl_mesh->V.row(vid);
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
                auto index = std::distance(vid_list.begin(), it);
                ball_list.erase(ball_list.begin() + index);
                vid_list.erase(vid_list.begin() + index);
            }                    
        };
    }

    void App::mouse_up(Window& window, int button, int modifier) 
    {   
        if (object_list.size() == 0) return;
        RA_LOG_INFO("modifier %i", modifier);
        if (mouse_is_down) 
        {   
            gesture_state = 2;
            //m_camera->HandleOneFingerPanGesture(gesture_state, Vector2i(mouse_x, mouse_y));
        }
        mouse_is_down = false;
        gesture_state = 0;
    }
    
    void App::mouse_move(Window& window, int mouse_x, int mouse_y) 
    {   
        if (object_list.size() == 0) return;

        
       // RA_LOG_INFO("mouse move");
        current_mouse_x = mouse_x;
        current_mouse_y = mouse_y;
        if (mouse_is_down && m_modifier == TYRO_MOD_CONTROL) 
        {   
            m_camera->HandleOneFingerPanGesture(gesture_state, Vector2i(mouse_x, mouse_y));
            gesture_state = 1;
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
        //RA_LOG_INFO("Key pressed %c", key);
        
        if (key == '`') 
        {
            show_console = !show_console;
            return;
        }

        if (show_console) 
        {
           m_console.keyboard(key);
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
            {}
            else if (key == '[')
            {}
        }
    }
    
    void App::key_down(Window& window, unsigned int key, int modifiers) 
   {   
        //RA_LOG_INFO("Key down %i", key)
        // handle not text keys   

        if (key == TYRO_KEY_LEFT) 
            m_console.key_left();
        else if (key == TYRO_KEY_RIGHT) 
            m_console.key_right();
         else if (key == TYRO_KEY_ENTER)
            m_console.key_enter();
        else if (key == TYRO_KEY_BACKSPACE)
            m_console.key_backspace();
        else if (key == TYRO_KEY_UP)
            m_console.key_up();
        else if (key == TYRO_KEY_DOWN)
            m_console.key_down();
        else if (key == TYRO_KEY_TAB)
            m_console.key_tab();
            
    }

     void App::register_console_function(const std::string& name,
                                         const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                         const std::string& help_txt)
    {
        std::function<void (const std::vector<std::string>&)> f = bind(con_fun, this, std::placeholders::_1);
        m_console.reg_cmdN(name, f, help_txt);
    }
}