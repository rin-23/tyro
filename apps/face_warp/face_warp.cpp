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
#include <igl/writeOBJ.h>
#include <igl/per_vertex_normals.h>
#include <igl/unique_edge_map.h>
#include <igl/unproject_onto_mesh.h>
#include "RAES2StandardMesh.h"
#include <igl/project.h>
#include <igl/eigs.h>
#include <igl/cotmatrix.h>
#include <igl/massmatrix.h>
#include <igl/read_triangle_mesh.h>
#include <igl/adjacency_matrix.h>
#include <igl/matlab_format.h>
#include <igl/bfs.h>
#include <igl/serialize.h>

#include <ctime>
#include "muslemesh.h"
#include <igl/readMESH.h>

using namespace std;

using Wm5::APoint;
using namespace Eigen;

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
    m_frame_overlay(nullptr),
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
    
    // split tets in TT into triangles so you can visualize them
    // output triangular mesh with colors per vertex or per face
    void 
    triangulate_tets(const Eigen::MatrixXi& TT,  // tets #T by 4
                     const Eigen::MatrixXd& TV,  // tet verticies #TV by 3
                     const Eigen::MatrixXd& TC,  // (optional) per vertex or per-tet colors #T|#TV by 3
                     const Eigen::MatrixXi& TI,  // (optional) only use subset of TT. If 1 te
                     const Eigen::VectorXd& TD,  // #TV by 3 diffusion values per vertex
                     Eigen::MatrixXd& V,         // resulting verticies #V by 3
                     Eigen::MatrixXi& F,         // resulting triangular faces #F by 3
                     Eigen::MatrixXd& C,         // (optional if TC is not empty) resulting per-vertex or per face colors #F|#V by 3
                     Eigen::VectorXd& D,         // #V by 3 diffusion values per vertex
                     bool perfacecolor)          // (ignored if TC is empty) if true then TC and C will contain per face colors, otherwise per-vertex
    {
        using namespace std;
        using namespace Eigen;
    
        if (TI.size()==0) 
        {
            int numtets = TT.rows();
            V.resize(numtets*4,3);
            D.resize(V.rows());
            // if (perfacecolor)
            if (TC.size() > 0)
                C.resize(numtets*4,3);
            // else
                // C.resize(numtets*4,3);
            
            F.resize(numtets*4,3);
                
            for (unsigned i=0; i<numtets;++i)
            {
                V.row(i*4+0) = TV.row(TT(i,0));
                V.row(i*4+1) = TV.row(TT(i,1));
                V.row(i*4+2) = TV.row(TT(i,2));
                V.row(i*4+3) = TV.row(TT(i,3));

                D.row(i*4+0) = TD.row(TT(i,0));
                D.row(i*4+1) = TD.row(TT(i,1));
                D.row(i*4+2) = TD.row(TT(i,2));
                D.row(i*4+3) = TD.row(TT(i,3));

                if (TC.size() > 0 && i < TC.rows()) 
                {
                    if (perfacecolor) 
                    {
                        C.row(i*4+0) = TC.row(i);
                        C.row(i*4+1) = TC.row(i);
                        C.row(i*4+2) = TC.row(i);
                        C.row(i*4+3) = TC.row(i);        
                    }
                    else 
                    {
                        C.row(i*4+0) = TC.row(TT(i,0));
                        C.row(i*4+1) = TC.row(TT(i,1));
                        C.row(i*4+2) = TC.row(TT(i,2));
                        C.row(i*4+3) = TC.row(TT(i,3));
                    }
                }

                F.row(i*4+0) << (i*4)+0, (i*4)+1, (i*4)+3;
                F.row(i*4+1) << (i*4)+0, (i*4)+2, (i*4)+1;
                F.row(i*4+2) << (i*4)+3, (i*4)+2, (i*4)+0;
                F.row(i*4+3) << (i*4)+1, (i*4)+2, (i*4)+3;
            }  
        }
        else 
        {
            int numtets = TI.rows();
            V.resize(numtets*4,3);
            // if (perfacecolor)
            if (TC.size() > 0)
                C.resize(numtets*4,3);
            // else
                // C.resize(numtets*4,3);
            
            F.resize(numtets*4,3);
                
            for (unsigned i=0; i<numtets;++i)
            {   
                auto rowidx = TI(i);
                auto v = TT.row(rowidx);
                V.row(i*4+0) = TV.row(v(0));
                V.row(i*4+1) = TV.row(v(1));
                V.row(i*4+2) = TV.row(v(2));
                V.row(i*4+3) = TV.row(v(3));

                D.row(i*4+0) = TD.row(TT(i,0));
                D.row(i*4+1) = TD.row(TT(i,1));
                D.row(i*4+2) = TD.row(TT(i,2));
                D.row(i*4+3) = TD.row(TT(i,3));


                if (TC.size() > 0 && rowidx < TC.rows()) 
                {
                    if (perfacecolor) 
                    {
                        C.row(i*4+0) = TC.row(rowidx);
                        C.row(i*4+1) = TC.row(rowidx);
                        C.row(i*4+2) = TC.row(rowidx);
                        C.row(i*4+3) = TC.row(rowidx);        
                    }
                    else 
                    {
                        C.row(i*4+0) = TC.row(v(0));
                        C.row(i*4+1) = TC.row(v(1));
                        C.row(i*4+2) = TC.row(v(2));
                        C.row(i*4+3) = TC.row(v(3));
                    }
                }

                F.row(i*4+0) << (i*4)+0, (i*4)+1, (i*4)+3;
                F.row(i*4+1) << (i*4)+0, (i*4)+2, (i*4)+1;
                F.row(i*4+2) << (i*4)+3, (i*4)+2, (i*4)+0;
                F.row(i*4+3) << (i*4)+1, (i*4)+2, (i*4)+3;
            }  
        }
    }


    void
    max_curve_values(const Eigen::MatrixXi& TT, // tets
                     const Eigen::MatrixXd& BC, // diffused vector values for each vertex
                     Eigen::VectorXd& MV,       // the diffusion value of the muscle a vertex belongs to
                     Eigen::VectorXi& MC,       // index of the curve that the vertex belongs to
                     Eigen::VectorXi& MT)       // index of the curve that the tet belongs to
    { 
        #define DEBUG_max_curve_values 0
        
        MT.resize(TT.rows());
        MC.resize(BC.rows());              
        MV.resize(BC.rows());              

        for (int i=0;i<TT.rows();++i)
        {   
            int maxidx = -1;
            double maxval = -1;
            for (int j=0; j < TT.cols(); j++) 
            {
                int vidx = TT(i,j);
                VectorXd::Index cmaxcol;
                double cmaxval = BC.row(vidx).maxCoeff(&cmaxcol);
                if (cmaxval > maxval) 
                {
                    maxval=cmaxval;
                    maxidx=cmaxcol;
                }
            }

            assert(maxidx>=0);
            MT(i) = maxidx;
            
            
            // MatrixXd::Index maxcol;
            // BC.row(i).maxCoeff(&maxcol);
            // MC(i) = maxcol;
        }

        for (int i=0;i<BC.rows();++i)
        {
            MatrixXd::Index maxcol;
            double maxval = BC.row(i).maxCoeff(&maxcol);
            MC(i) = maxcol;
            MV(i) = maxval;
        }

        #if DEBUG_max_curve_values
        std::cerr<<igl::matlab_format(MC,"MC")<<std::endl;
        #endif          
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


        // std::string strrr("LOLL");// = std::to_string(cur_idx);//("Framasdasdasdsaddasde 0/9000");
        // m_frame_overlay = ES2TextOverlay::Create(strrr, 
        //                                         Wm5::Vector2f(0, 0), 
        //                                         font, 
        //                                         Wm5::Vector4f(1,0,0,1), 
        //                                         m_camera->GetViewport());
        // m_frame_overlay->SetText(strrr);
        


        // load obj file
        //Eigen::MatrixXd V,N; // Vertex data. 3*num_vert by num_frames. 
        Eigen::MatrixXd UEC_T, UEC_S;
        Eigen::MatrixXi E_T, E_S, UE_T, UE_S; // Face data. 
        Eigen::VectorXi EMAP_S, EMAP_T; // Map directed edges to unique edges. 
        std::vector<std::vector<int> > uE2E_S, uE2E_T;

        //std::string template_obj = "/Users/rinat/Workspace/TemplateDeform/data/sphere_temp.obj";
        //std::string template_obj = "/Users/rinat/Workspace/R3DS_Wrap_3.3.17_Linux/Models/HeadScans/Alex.obj";
        //std::string template_obj = "/Users/rinat/Workspace/R3DS_Wrap_3.3.17_Linux/Models/Blendshapes/Basemesh_igl.obj";

        // GEOMETRY.VT, GEOMETRY.NT, GEOMETRY.FT
        Eigen::MatrixXd V_temp;
        Eigen::MatrixXd C_temp;
        Eigen::MatrixXi F_temp;
        Eigen::MatrixXi T_temp;
        Eigen::MatrixXd TC, CC; 
        Eigen::MatrixXi TI; 
        Eigen::MatrixXi TBF;

        /*
         * LOAD TET MESH
         */
        std::string tetmeshpath= "/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/dino/boneskin_coarse3.mesh";
        igl::readMESH(tetmeshpath, V_temp, T_temp, F_temp);

        /*
         *  COMPUTE BOUNDARY OF THE FACE
         */
        // igl::boundary_facets(T_temp, TBF); // boundary faces

        /*
         * LOAD DIFFUSED VALUES
         */
        int max3dsize;
        glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max3dsize);
        Eigen::MatrixXd Z;
        igl::deserialize(Z, "Z", "/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/dino/Z");
        assert(Z.rows()>0);
        Eigen::VectorXd MV, MV2;
        Eigen::VectorXi MC;
        Eigen::VectorXi MT;

        MV = Z.col(1);        
        triangulate_tets(T_temp, V_temp, TC, TI, MV, GEOMETRY.VT, GEOMETRY.FT, CC, MV2, true);
        // igl::per_vertex_normals(GEOMETRY.VT,GEOMETRY.FT,GEOMETRY.NT);
        igl::per_face_normals(GEOMETRY.VT, GEOMETRY.FT, GEOMETRY.NT);

        
        #if 0
        std::cerr<<igl::matlab_format(Z,"Z")<<std::endl;
        #endif   

        // std::string template_obj = "/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/cylinder.obj"; //Basemeshes/WrapHand.obj";
        //igl::readOBJ(template_obj, GEOMETRY.VT,GEOMETRY.FT);
        // tyro::load_mesh(template_obj, GEOMETRY.VT, GEOMETRY.NT, GEOMETRY.FT);
        //igl::writeOBJ("/Users/rinat/Workspace/R3DS_Wrap_3.3.17_Linux/Models/Basemeshes/WrapHand_igl.obj", GEOMETRY.VT,GEOMETRY.FT);
        // igl::per_vertex_normals(GEOMETRY.VT,GEOMETRY.FT,GEOMETRY.NT);
        igl::unique_edge_map(GEOMETRY.FT,E_T,UE_T,EMAP_T,uE2E_T);
        
        std::string scan_obj = "/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/cylinder.obj";
        //std::string scan_obj = "/Users/rinat/Workspace/R3DS_Wrap_3.3.17_Linux/Models/HeadScans/Alex.obj";
        //std::string scan_obj = "/Users/rinat/Workspace/R3DS_Wrap_3.3.17_Linux/Models/Blendshapes/LipsDisgust_igl.obj";
        //std::string scan_obj = "/Users/rinat/Workspace/TemplateDeform/data/sphere_scan.obj";
        //tyro::load_mesh(obj_list, V, N, F, E, UE, EMAP);
        // igl::readOBJ(scan_obj, GEOMETRY.VS,GEOMETRY.FS);
        //tyro::load_mesh(scan_obj, GEOMETRY.VS, GEOMETRY.NS, GEOMETRY.FS);
        //igl::writeOBJ("/Users/rinat/Workspace/R3DS_Wrap_3.3.17_Linux/Models/HandScans/Relaxed_igl.obj", GEOMETRY.VS,GEOMETRY.FS);
        // igl::per_vertex_normals(GEOMETRY.VS,GEOMETRY.FS,GEOMETRY.NS);
        // igl::unique_edge_map(GEOMETRY.FS,E_S,UE_S,EMAP_S,uE2E_S);

        // RENDER.scan = IGLMesh::Create(GEOMETRY.VS, GEOMETRY.FS, GEOMETRY.NS, Eigen::Vector3d(0,0,1));
        // RENDER.template_mesh = IGLMesh::Create(GEOMETRY.VT, GEOMETRY.FT, GEOMETRY.NT, Eigen::Vector3d(0,0,0.7));
        assert(GEOMETRY.VT.rows() == MV2.rows());
        RENDER.template_mesh = MuscleMesh::Create(GEOMETRY.VT, GEOMETRY.FT, GEOMETRY.NT, MV2);

        color_matrix(UE_T.rows(), Eigen::Vector3d(0.5,0.5,0.5), UEC_T);
        color_matrix(UE_S.rows(), Eigen::Vector3d(0.5,0.5,0.5), UEC_S);

        // RENDER.scan_wire = IGLMeshWireframe::Create(GEOMETRY.VS, UE_S, UEC_S);
        RENDER.template_mesh_wire = IGLMeshWireframe::Create(GEOMETRY.VT, UE_T, UEC_T);
        RENDER.template_mesh_wire->Visible = true;
        
        // RENDER.scan->Visible = false;
        // RENDER.scan_wire->Visible = false;
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

    void App::calculateEigs() 
    {
        Eigen::MatrixXd V,U;
        Eigen::MatrixXi F;
        using namespace Eigen;
        using namespace std;
        using namespace igl;
        
        VectorXd D;
        if(!read_triangle_mesh("/Users/rinat/Workspace/TemplateDeform/data/WrapHand_igl2.obj",V,F))
        {
            cout<<"failed to load mesh"<<endl;
            return;
        }

        Eigen::SparseMatrix<double> A;
        igl::adjacency_matrix(F,A);
        std::vector<int> a1,a2;
        
        // clock_t begin = clock();

        // for (int i=0; i < V.rows(); ++i) 
        // {
        //     igl::bfs(A,i,a1,a2);    
        // }

        // clock_t end = clock();
        // double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
        // cout<<"Time "<< elapsed_secs << endl;
        
        clock_t begin2 = clock();
        SparseMatrix<double> L,M;
        cotmatrix(V,F,L);   
        L = (-L).eval();
        massmatrix(V,F,MASSMATRIX_TYPE_DEFAULT,M);
        const size_t k = 20;
        cout<<"Eigs started."<<endl;
        if(!eigs(L,M,k+1,EIGS_TYPE_SM,U,D))
        {
            cout<<"Eigs failed."<<endl;
        }
        else 
        {
            cout<<D<<endl;
            cout<<"Eigs done"<<endl;
        }
        clock_t end2 = clock();
        double elapsed_secs2 = double(end2 - begin2) / CLOCKS_PER_SEC;
        cout<<"Time "<< elapsed_secs2 << endl;
    }
    
    void App::DrawMeshes() 
    {
        //RA_LOG_INFO("RENDER BEGIN");

        //create renderable for mesh
        VisibleSet vis_set;
   
        //std::string fstr = std::string("Frame ") + std::to_string(m_frame) + std::string("/") + std::to_string(ANIM.VD.size());
        //fstr =fstr+ std::string(" Scene ") + std::to_string(scene_id);
        //m_frame_overlay->SetText(fstr);
        
        // RENDER.scan->Update(true);
        // if (RENDER.scan->Visible)
        //     vis_set.Insert(RENDER.scan.get());

        RENDER.template_mesh->Update(true);
        if (RENDER.template_mesh->Visible) 
            vis_set.Insert(RENDER.template_mesh.get());
        
        // RENDER.scan_wire->Update(true);
        // if (RENDER.scan_wire->Visible)
        //     vis_set.Insert(RENDER.scan_wire.get());

        RENDER.template_mesh_wire->Update(true);
        if (RENDER.template_mesh_wire->Visible)
            vis_set.Insert(RENDER.template_mesh_wire.get());

        for (auto object_sptr : ball_list_template) 
        {   
            vis_set.Insert(object_sptr.get());
        }
        
        for (auto object_sptr : ball_list_scan) 
        {   
            vis_set.Insert(object_sptr.get());
        }

        for (auto object_sptr : vid_numbers_template) 
        {      
           // object_sptr->UpdateUniformsWithCamera(m_camera);
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
        AxisAlignedBBox WorldBoundBox = RENDER.template_mesh->WorldBoundBox;
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
        RA_LOG_INFO("Key pressed %c", key);
        
        if (key == '`') 
        {   
            //RA_LOG_INFO("Pressed %c", key);
            RENDER.template_mesh_wire->Visible = !RENDER.template_mesh_wire->Visible;
            // show_console = !show_console;
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
        RA_LOG_INFO("Key down %i", key)
        // handle not text keys   

        if (key == TYRO_KEY_SPACE)
        {
            RA_LOG_INFO("save selected verticies");
            auto path = std::string("/Users/rinat/Workspace/TemplateDeform/data/template_points_facemesh.txt");

            std::ofstream outFile(path);
            for (const auto &vid : vid_list_template)
            { 
                outFile << vid << "\n";
            }
        }
        if (key == TYRO_KEY_ENTER)
        {
            RA_LOG_INFO("save selected verticies");
            auto path = std::string("/Users/rinat/Workspace/TemplateDeform/data/scan_points_facemesh.txt");

            std::ofstream outFile(path);
            for (const auto &vid : vid_list_template)
            { 
                outFile << vid << "\n";
            }
        }
        else if (key == TYRO_KEY_LEFT){ 
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

    bool App::testScanClicked(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier, int& fid, Eigen::Vector3f& bc) 
    {
        Wm5::HMatrix modelViewMatrix = m_camera->GetViewMatrix(); // * RENDER.scan->WorldTransform.Matrix();
        Wm5::HMatrix projectMatrix = m_camera->GetProjectionMatrix();
        Eigen::Matrix4f e1 = Eigen::Map<Eigen::Matrix4f>(modelViewMatrix.mEntry);
        Eigen::Matrix4f e2 = Eigen::Map<Eigen::Matrix4f>(projectMatrix.mEntry);
        Eigen::Vector4f e3 = Eigen::Vector4f(m_camera->GetViewport()[0],
                                             m_camera->GetViewport()[1],
                                             m_camera->GetViewport()[2],
                                             m_camera->GetViewport()[3]);

        RA_LOG_INFO("here2");
        RA_LOG_INFO("%f %f ",mouse_pos[0],mouse_pos[1]);
        // int fid;
        // Eigen::Vector3f bc;
        Eigen::Vector2f mouse_pos2(current_mouse_x, current_mouse_y);
        //RA_LOG_INFO("%f %f ",mouse_pos2[0],mouse_pos2[1]);
        bool hit = igl::unproject_onto_mesh(mouse_pos, 
                                     e1.transpose(),
                                     e2.transpose(),
                                     e3,
                                     GEOMETRY.VS,
                                     GEOMETRY.FS,
                                     fid,
                                     bc);
        return hit;
    }

    bool App::testTemplateClicked(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier, int& fid, Eigen::Vector3f& bc) 
    {
        Wm5::HMatrix modelViewMatrix = m_camera->GetViewMatrix() * RENDER.template_mesh->WorldTransform.Matrix();
        Wm5::HMatrix projectMatrix = m_camera->GetProjectionMatrix();
        Eigen::Matrix4f e1 = Eigen::Map<Eigen::Matrix4f>(modelViewMatrix.mEntry);
        Eigen::Matrix4f e2 = Eigen::Map<Eigen::Matrix4f>(projectMatrix.mEntry);
        Eigen::Vector4f e3 = Eigen::Vector4f(m_camera->GetViewport()[0],
                                             m_camera->GetViewport()[1],
                                             m_camera->GetViewport()[2],
                                             m_camera->GetViewport()[3]);

        RA_LOG_INFO("here3");
        RA_LOG_INFO("%f %f ", mouse_pos[0], mouse_pos[1]);
        // int fid;
        // Eigen::Vector3f bc;
        Eigen::Vector2f mouse_pos2(current_mouse_x, current_mouse_y);
        //RA_LOG_INFO("%f %f ",mouse_pos2[0],mouse_pos2[1]);
        bool hit = igl::unproject_onto_mesh(mouse_pos, 
                                     e1.transpose(),
                                     e2.transpose(),
                                     e3,
                                     GEOMETRY.VT,
                                     GEOMETRY.FT,
                                     fid,
                                     bc);
        return hit;
    }

    void App::selectVertex(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier) 
    {   
        RA_LOG_INFO("here1");

        int fid;
        Eigen::Vector3f bc;
        bool hitScan = false;
        // if (RENDER.scan->Visible == true) 
        //     hitScan = testScanClicked(mouse_pos, mouse_button, modifier, fid, bc);
        
        if (hitScan) 
        {
            RA_LOG_INFO("here3");

            if (true) 
            {   
                RA_LOG_INFO("here4");
                long c;
                bc.maxCoeff(&c);
                int vid = GEOMETRY.FS(fid, c);
                auto it = std::find(vid_list_scan.begin(), vid_list_scan.end(), vid);
                if (it == vid_list_scan.end()) 
                {       
                    Eigen::Vector3d vec = GEOMETRY.VS.row(vid);
                    RA_LOG_INFO("Picked face_id %i vertex_id %i coord %f %f %f", fid, vid, vec(0), vec(1), vec(2));
                    // addSphere(vid, GEOMETRY.VS, Wm5::Vector4f(0,1,0,1), RENDER.scan->WorldTransform, true);
                    add_vertex(vid,true);
                }  
                else
                {   
                    RA_LOG_INFO("remove vertex %i %i", fid, vid);
                    auto index = std::distance(vid_list_scan.begin(), it);
                    ball_list_scan.erase(ball_list_scan.begin() + index);
                    vid_list_scan.erase(vid_list_scan.begin() + index);
                }
                render(); 
            } 
        } 
        else 
        {   
            
            bool hitTemplate = false;
            if (RENDER.template_mesh->Visible == true) 
                hitTemplate = testTemplateClicked(mouse_pos, mouse_button, modifier, fid, bc);
            
            if (hitTemplate) 
            {
                RA_LOG_INFO("here3");

                if (true) 
                {   
                    RA_LOG_INFO("here4");
                    long c;
                    bc.maxCoeff(&c);
                    int vid = GEOMETRY.FT(fid, c);
                    auto it = std::find(vid_list_template.begin(), vid_list_template.end(), vid);
                    if (it == vid_list_template.end()) 
                    {       
                        Eigen::Vector3d vec = GEOMETRY.VT.row(vid);
                        RA_LOG_INFO("Picked face_id %i vertex_id %i coord %f %f %f", fid, vid, vec(0), vec(1), vec(2));
                        addSphere(vid, GEOMETRY.VT, Wm5::Vector4f(0,1,0,1), RENDER.template_mesh->WorldTransform, false);
                        add_vertex(vid,false);
                    }  
                    else
                    {   
                        RA_LOG_INFO("remove vertex %i %i", fid, vid);
                        auto index = std::distance(vid_list_template.begin(), it);
                        ball_list_template.erase(ball_list_template.begin() + index);
                        vid_list_template.erase(vid_list_template.begin() + index);
                    }
                    render(); 
                } 
            }
        }
           
    }

    void App::add_vertex(int vid, bool isScan)
    {   
        if (isScan) 
        {
            auto it = std::find(vid_list_scan.begin(), vid_list_scan.end(), vid);
            if (it == vid_list_scan.end())
            {
                vid_list_scan.push_back(vid);
            }
        } 
        else 
        {
            auto it = std::find(vid_list_template.begin(), vid_list_template.end(), vid);
            if (it == vid_list_template.end())
            {
                vid_list_template.push_back(vid);
            }
        }       
    }

    void App::addSphere(int vid, const Eigen::MatrixXd& V, Wm5::Vector4f color, Wm5::Transform world, bool isScan) 
    {   
        Eigen::RowVector3d new_c = V.row(vid);
        float m_ball_size = 0.12;
        ES2SphereSPtr object = ES2Sphere::Create(10, 10, m_ball_size);
        Wm5::Transform tr;
        tr.SetTranslate(Wm5::APoint(new_c(0), new_c(1), new_c(2)));
        object->LocalTransform = world * tr * object->LocalTransform;
        object->Update(true);
        object->SetColor(color);
        
        if (isScan)
            ball_list_scan.push_back(object);
        else 
        {
            ball_list_template.push_back(object); 
            addPointText(vid, V, color, isScan);
        }
    }     

    void App::addPointText(int vid, const Eigen::MatrixXd& V, Wm5::Vector4f color, bool isScan) 
    {   
        ES2FontSPtr font = FontManager::GetSingleton()->GetSystemFontOfSize12();
        int cur_idx = vid_list_template.size();
        std::string strrr = std::to_string(cur_idx);//("Framasdasdasdsaddasde 0/9000");
        ES2TextOverlaySPtr text_num = ES2TextOverlay::Create(strrr, 
                                                            Wm5::Vector2f(0, 0), 
                                                            font, 
                                                            Wm5::Vector4f(1,0,0,1), 
                                                            m_camera->GetViewport());


        // Wm5::HMatrix modelViewMatrix = m_camera->GetViewMatrix() * RENDER.template_mesh->WorldTransform.Matrix(); //TODO need on for scan
        // Wm5::HMatrix projectMatrix = m_camera->GetProjectionMatrix();
        // Eigen::Matrix4f e1 = Eigen::Map<Eigen::Matrix4f>(modelViewMatrix.mEntry);
        // Eigen::Matrix4f e2 = Eigen::Map<Eigen::Matrix4f>(projectMatrix.mEntry);
        // Eigen::Vector4f e3 = Eigen::Vector4f(m_camera->GetViewport()[0],
        //                                      m_camera->GetViewport()[1],
        //                                      m_camera->GetViewport()[2],
        //                                      m_camera->GetViewport()[3]);
        
        //Eigen::Vector3f objj(V(vid,0),V(vid,1),V(vid,2));
        Wm5::Vector3f wm5objj(V(vid,0),V(vid,1),V(vid,2));
        // Eigen::Vector3f projectedPoint = igl::project(objj, e1, e2, e3);
        // Wm5::Vector3f vv = m_camera->WorldToScreen(Wm5::Vector3f(V(vid,0),V(vid,1),V(vid,2)));
        // RA_LOG_INFO("Add point %i text %f %f %f",vid, projectedPoint[0],projectedPoint[1], projectedPoint[2]);
        // RA_LOG_INFO("Add point %i text %f %f %f",vid, vv[0],vv[1], vv[2]);
        // // vv[0] = (vv[0] - e3[2]/2)/(e3[2]/2);
        // vv[1] = (vv[1] - e3[3]/2)/(e3[3]/2);
        //text_num->SetTranslate(Wm5::Vector2f(vv[0], vv[1]));
        text_num->SetWorldPos(wm5objj);
        text_num->SetText(strrr);
        vid_numbers_template.push_back(text_num);
    }
    //todo: update rotation of points
    //void App::updatePointText()
}