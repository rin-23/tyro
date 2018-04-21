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
#include <igl/writeSTL.h>
#include <igl/readSTL.h>
#include <igl/writeOBJ.h>
#include <igl/remove_duplicate_vertices.h>
#include "TyroFileUtils.h"
#include <igl/slice.h>
#include <igl/unique.h>

using namespace Wm5;
using namespace std;

namespace tyro
{   
    namespace
    {   
        void console_save_mesh_sequence_with_selected_faces(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Saving mesh sequence with selected faces");

            if (args.size() == 2)
            {   
                app->save_mesh_sequence_with_selected_faces(args[0], args[1]);
                return;
            }
        }

        void console_load_selected_faces(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Loading selected faces from tmp folder");

            if (args.size() == 1)
            {   
                app->load_selected_faces(args[0]);
                return;
            }
        }

        void console_save_selected_faces(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Saving selected faces to tmp folder");

            if (args.size() == 1)
            {   
                app->save_selected_faces(args[0]);
                return;
            }
        }

        void console_save_selected_verticies(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Saving selected verticies to tmp folder");

            if (args.size() == 1)
            {   
                app->save_selected_verticies(args[0]);
                return;
            }
        }

        void console_load_selected_verticies(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Loading selected verticies from tmp folder");

            if (args.size() == 1)
            {   
                app->load_selected_verticies(args[0]);
                return;
            }
        }

        void console_set_selection_type(App* app, const std::vector<std::string> & args)
        {
            RA_LOG_INFO("Set selection type");

            if (args.size() == 1)
            {      
                if (args[0] == "vertex") 
                    app->set_selection_type(App::SelectionMode::Vertex);
                else if (args[0] == "faces")
                    app->set_selection_type(App::SelectionMode::Faces);
                else if (args[0] == "edges")
                    app->set_selection_type(App::SelectionMode::Edges);
                return;
            }
        }

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

         void console_compute_deformation2(App* app, const std::vector<std::string> & args)
        {
            RA_LOG_INFO("deform2");

            if (args.size() == 0)
            {   
                app->compute_deformation2();
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
        
        m_tyro_window->callback_mouse_scroll = [&](Window& window, float ydelta)->bool 
        {
            this->mouse_scroll(window, ydelta);
        };


        register_console_function("load_blobby", console_load_blobby, "");
        register_console_function("load_oldman", console_load_oldman, "");
        register_console_function("load_bunny", console_load_bunny, "");
        register_console_function("compute_average", console_compute_average, "");
        register_console_function("compute_deformation", console_compute_deformation, "");
        register_console_function("compute_deformation2", console_compute_deformation2, "");
        register_console_function("save_selected_faces", console_save_selected_faces, "");
        register_console_function("load_selected_faces", console_load_selected_faces, "");
        register_console_function("save_selected_verticies", console_save_selected_verticies, "");
        register_console_function("load_selected_verticies", console_load_selected_verticies, "");
        register_console_function("set_selection_type", console_set_selection_type, "");
        register_console_function("save_mesh_sequence_with_selected_faces", console_save_mesh_sequence_with_selected_faces, "");

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

    void App::compute_deformation2()
    {
        if (fid_list.size() == 0) return;
        
        m_frame_deformed_data.v_data.reserve(m_frame_data.v_data.size());
        m_frame_deformed_data.f_data = m_frame_data.f_data;

        //list of vids that can be deformed
        std::vector<int> vid_deform, vid_deform_dup; 
        for (auto fid : fid_list) 
        {
            int vid0 = m_frame_data.f_data.row(fid)(0);
            int vid1 = m_frame_data.f_data.row(fid)(1);
            int vid2 = m_frame_data.f_data.row(fid)(2);
            
            //check that we dont include ones that are part of the seam 
            auto it2 = std::find(vid_list.begin(), vid_list.end(), vid0);
            if (it2 == vid_list.end()) vid_deform_dup.push_back(vid0);
            
            it2 = std::find(vid_list.begin(), vid_list.end(), vid1);
            if (it2 == vid_list.end()) vid_deform_dup.push_back(vid1);

            it2 = std::find(vid_list.begin(), vid_list.end(), vid2);
            if (it2 == vid_list.end()) vid_deform_dup.push_back(vid2);            
        }
        igl::unique(vid_deform_dup, vid_deform); //remove duplicates
        
        //list of vids that can not be deformed minus seam vids
        std::vector<int> vid_not_deform; 
        for (int vid = 0; vid < m_frame_data.v_data[0].rows(); ++vid) 
        {
            auto it = std::find(vid_deform.begin(), vid_deform.end(), vid); 
            auto it2 = std::find(vid_list.begin(), vid_list.end(), vid);

            if (it == vid_deform.end() && it2 == vid_list.end())
            {   
                vid_not_deform.push_back(vid);
            }    
        }

        assert(vid_deform.size() + vid_not_deform.size() + vid_list.size() == m_frame_data.v_data[0].rows());

        int num_frames = m_frame_data.v_data.size();
        for (int i =0; i < num_frames; ++i) 
        {   
            RA_LOG_INFO("Compute deformation for mesh %i out of %i", i, num_frames)
            Eigen::MatrixXd V = m_frame_data.v_data[i];
            Eigen::MatrixXi F = m_frame_data.f_data;
            Eigen::VectorXi b;
            int num_fixed = vid_not_deform.size() + vid_list.size();
            int num_not_fixed = vid_deform.size();
            b.resize(num_fixed);
            
            int b_index = 0;            
            for (int vid : vid_list) b(b_index++) = vid;
            for (int vid : vid_not_deform) b(b_index++) = vid;
                       
            //do biharmonic stuff here.
            igl::min_quad_with_fixed_data<double> data;
            Eigen::SparseMatrix<double> L;            
            igl::cotmatrix(V, F, L); //try average

            Eigen::SparseMatrix<double> M;	
            igl::massmatrix(V, F, igl::MASSMATRIX_TYPE_DEFAULT, M);

            //Computer M inverse
            Eigen::SimplicialLDLT <Eigen::SparseMatrix<double>> solver;
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
            bc.resize(num_fixed, 3);
            bc.setZero();
            b_index = 0;
            for (int vid : vid_list)
            {
                bc.row(b_index++) = m_frame_data.avg_v_data.row(vid) - m_frame_data.v_data[i].row(vid);
            }
            //for (int vid : vid_not_deform)
           // {
            //    bc.row(b_index++) = V.row(vid);
           // }

            Eigen::MatrixXd B = Eigen::MatrixXd::Zero(data.n, bc.cols());
            Eigen::VectorXd Beq;
            
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
        float radius = std::abs(WorldBoundBox.GetRadius()*2.5);
        float aspect = 1.0;
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Vector4i viewport(0, 0, v_width, v_height);
        
        if (m_camera)
            delete m_camera;
        
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
    }

    void App::load_mesh_sequence(const std::vector<std::string>& obj_list, bool use_igl_loader) 
    {   
        RA_LOG_INFO("LOAD MESH SEQUENCE")
        
        for (const auto& obj_list_file : obj_list) 
        {
            stop::load_mesh_sequence(obj_list_file, 
                                     m_frame_data.v_data, 
                                     m_frame_data.n_data, 
                                     m_frame_data.f_data,
                                     use_igl_loader);
        }
        
        //@TODO need this to update camera
        Eigen::Vector3d cr(0.5,0.5,0.5);
        
        //m_frame_data.c_data.resize(m_frame_data.v_data[0].rows(), 
        //                           m_frame_data.v_data[0].cols());
        m_frame_data.c_data.resize(m_frame_data.f_data.rows(), 3);
        for (int i =0 ; i <  m_frame_data.f_data.rows(); ++i) 
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

        compute_average();
    }

    void App::load_bunny()
    {
        RA_LOG_INFO("Load Bunny");
        auto obj_list_file1 = std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/Production Files Archive Rinat/production/obj_export/02_rabbit/04/noears/objlist.txt");
        auto obj_list_file2 = std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/Production Files Archive Rinat/production/obj_export/02_rabbit/05/noears/objlist.txt");
        //auto obj_list_file3 = std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/Production Files Archive Rinat/production/obj_export/02_rabbit/03/objlist.txt");
        std::vector<std::string> obj_list = {obj_list_file1, obj_list_file2};
        load_mesh_sequence(obj_list, false); //use tiny obj loader
        
        
        //auto obj_list_file = std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/Production Files Archive Rinat/production/obj_export/02_rabbit/02/bunn_face/objlist.txt");
        //load_mesh_sequence(obj_list_file, true);

        m_state = App::State::LoadedModel;
        render();
    }

    //load mexican blobby guy
    void App::load_blobby() 
    {   
        RA_LOG_INFO("LOAD_BLOBBY");
        auto obj_list_file = std::string("/home/rinat/GDrive/StopMotionProject/Claymation/data/hello/FramesOBJ/objlist2.txt");
        //load_mesh_sequence(obj_list_file);
        m_state = App::State::LoadedModel;
        render();
    }

    //load oldman (will you go to lunch)
    void App::load_oldman() 
    {   
        RA_LOG_INFO("LOAD OLDMAN");
        auto obj_list_file = std::string("/home/rinat/GDrive/StopMotionProject/Claymation/data/oldman/gotolunch/FramesOBJ/FullFace/objlist2.txt");
        //load_mesh_sequence(obj_list_file);
        m_state = App::State::LoadedModel;
        render();
    }

    void App::save_selected_faces(const std::string& filename) 
    {
        RA_LOG_INFO("save selected faces");
        auto path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/") + filename;

        std::ofstream outFile(path);
        for (const auto &e : fid_list)
        { 
            outFile << e << "\n";
        }
    }

    void App::save_selected_verticies(const std::string& filename) 
    {
        RA_LOG_INFO("save selected verticies");
        auto path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/") + filename;

        std::ofstream outFile(path);
        for (const auto &e : vid_list)
        { 
            outFile << e << "\n";
        }
    }
    
    void App::load_selected_faces(const std::string& filename) 
    {
        RA_LOG_INFO("Load selected faces");
        auto path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/") + filename;

        fid_list.clear();
        std::fstream myfile(path, std::ios_base::in);
        int fid;
        while (myfile >> fid)
        {
            printf("%i ", fid);
            fid_list.push_back(fid);

            Eigen::Vector3d clr(0,0,0.5);
            m_frame_data.c_data.row(fid) = clr;
        }
        render();
        glfwPostEmptyEvent();        
        //getchar();
    }

    void App::load_selected_verticies(const std::string& filename) 
    {
        RA_LOG_INFO("Load selected verticies");
        auto path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/") + filename;

        vid_list.clear();
        std::fstream myfile(path, std::ios_base::in);
        int vid;
        while (myfile >> vid)
        {
            printf("%i ", vid);
            vid_list.push_back(vid);
            Eigen::RowVector3d new_c = m_frame_data.v_data[m_frame].row(vid);
            ES2SphereSPtr object = ES2Sphere::Create(10, 10, 0.001);
            Wm5::Transform tr;
            tr.SetTranslate(APoint(new_c(0), new_c(1), new_c(2)));
            object->LocalTransform = tr * object->LocalTransform;
            object->Update(true);
            ball_list.push_back(object);
        }
        render();
        glfwPostEmptyEvent();        
        //getchar();
    }
    
    void App::set_selection_type(SelectionMode sel_state) 
    {
        m_sel_mode = sel_state;
    }

    void App::save_mesh_sequence_with_selected_faces(const std::string& folder, const std::string& filename) 
    {   
        if (fid_list.size() > 0) 
        {   
            auto path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/") + folder + std::string("/") + filename;
            auto tmp_path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/") + folder + std::string("/") + "tmp";
            auto objlist_path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/") + folder + std::string("/") + std::string("objlist.txt");
            ofstream objlist_file;
            objlist_file.open (objlist_path);
            
            Eigen::MatrixXi SVI, SVJ, nodp_F;
            Eigen::MatrixXi newF;
            newF.resize(fid_list.size(), 3);
            int fidIdx = 0;
            for (auto fid : fid_list) newF.row(fidIdx++) = m_frame_data.f_data.row(fid);
            
            for (int frame = 0; frame < m_frame_data.v_data.size();++frame) 
            {   
                bool success = igl::writeSTL(tmp_path, m_frame_data.v_data[frame], newF, false);
                assert(success);
                Eigen::MatrixXd temp_V, temp_N;
                Eigen::MatrixXi temp_F;
                success = igl::readSTL(tmp_path, temp_V, temp_F, temp_N);
                assert(success);
                Eigen::MatrixXd nodp_V;
                
                if (frame == 0) 
                {
                    igl::remove_duplicate_vertices(temp_V, temp_F, 0, nodp_V, SVI, SVJ, nodp_F);
                }
                else 
                {
                    igl::slice(temp_V, SVI, 1, nodp_V); 
                }
                auto frame_path = path + tyro::pad_zeros(frame) + std::string(".obj"); 
                igl::writeOBJ(frame_path, nodp_V, nodp_F);
                objlist_file << frame_path << "\n";

            }
            objlist_file.close();             
        }
    }

    void App::mouse_down(Window& window, int button, int modifier) 
    {   
        RA_LOG_INFO("mouse down %i", button);

        if (m_state != App::State::LoadedModel) return;

        mouse_is_down = true;
        m_modifier = modifier;
        m_mouse_btn_clicked = button;

        if (m_modifier == TYRO_MOD_CONTROL) return; //rotating
        if (m_modifier == TYRO_MOD_SHIFT) return; //selection
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
                    ES2SphereSPtr object = ES2Sphere::Create(10, 10, 0.001);
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
                    m_frame_data.c_data.row(fid) = clr;
                }  
                else
                {   
                    RA_LOG_INFO("remove face %i", fid);
                    auto index = std::distance(fid_list.begin(), it);
                    fid_list.erase(fid_list.begin() + index);
                    Eigen::Vector3d clr(0.5,0.5,0.5);
                    //Eigen::Vector3i vtx_idx = m_frame_data.f_data.row(fid);
                    //m_frame_data.c_data.row(vtx_idx(0)) = clr;
                    //m_frame_data.c_data.row(vtx_idx(1)) = clr;
                    //m_frame_data.c_data.row(vtx_idx(2)) = clr;
                    m_frame_data.c_data.row(fid) = clr;
                }
                render();
            }                  
        };        
    }

    void App::mouse_up(Window& window, int button, int modifier) 
    {   
        if (m_state != App::State::LoadedModel) return;
        RA_LOG_INFO("MOUSE_UP");
        if (mouse_is_down && m_modifier == TYRO_MOD_CONTROL) 
        {   
            gesture_state = 2;
            m_camera->HandleOneFingerPanGesture(gesture_state, Vector2i(current_mouse_x, current_mouse_y));
            render();
        }
        else if (mouse_is_down && m_modifier == TYRO_MOD_SHIFT) 
        {
            gesture_state = 2;
            //m_camera->HandleTwoFingerPanGesture(gesture_state, Vector2i(current_mouse_x, -current_mouse_y));
            //render();
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
        
        current_mouse_x = mouse_x;
        current_mouse_y = mouse_y;

        if (mouse_is_down && m_modifier == TYRO_MOD_SHIFT) 
        {
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
                        m_frame_data.c_data.row(fid) = clr;
                    }  
                    else
                    {   
                        RA_LOG_INFO("remove face %i", fid);
                        //auto index = std::distance(fid_list.begin(), it);
                        //fid_list.erase(fid_list.begin() + index);
                        //Eigen::Vector3d clr(0.5,0.5,0.5);
                        //Eigen::Vector3i vtx_idx = m_frame_data.f_data.row(fid);
                        //m_frame_data.c_data.row(vtx_idx(0)) = clr;
                        //m_frame_data.c_data.row(vtx_idx(1)) = clr;
                        //m_frame_data.c_data.row(vtx_idx(2)) = clr;
                        //m_frame_data.c_data.row(fid) = clr;
                    }
                    render();
                }                  
            };        
        }
        else if (mouse_is_down && m_modifier == TYRO_MOD_CONTROL) 
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

    void App::mouse_scroll(Window& window, float ydelta) 
    {
        RA_LOG_INFO("mouse scroll delta %f", ydelta);
        if (m_state != App::State::LoadedModel) return;
        
        m_camera->HandlePinchGesture(gesture_state, Vector2i(current_mouse_x, current_mouse_y), ydelta);
        render();
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