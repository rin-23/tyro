#include "face_warp.h"
#include "RAEnginePrerequisites.h"
// #include "RAES2TextOverlay.h"
// #include "RAFont.h"
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
#include <igl/readDMAT.h>

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
    // m_frame_overlay(nullptr),
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
                     Eigen::VectorXi& FtoT,      // Face to tet ID match
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
            FtoT.resize(F.rows());
                
            for (unsigned i=0; i<numtets;++i)
            {
                V.row(i*4+0) = TV.row(TT(i,0));
                V.row(i*4+1) = TV.row(TT(i,1));
                V.row(i*4+2) = TV.row(TT(i,2));
                V.row(i*4+3) = TV.row(TT(i,3));

                F.row(i*4+0) << (i*4)+1, (i*4)+2, (i*4)+3; 
                F.row(i*4+1) << (i*4)+3, (i*4)+2, (i*4)+0;
                F.row(i*4+2) << (i*4)+0, (i*4)+1, (i*4)+3;
                F.row(i*4+3) << (i*4)+0, (i*4)+2, (i*4)+1;

                // bottom trianlge with 3 on the top
                //       2
                //      / \  
                //     /   \
                //    /     \
                //   /       \
                //  0---------1

                FtoT(i*4+0) = i;
                FtoT(i*4+1) = i;
                FtoT(i*4+2) = i;
                FtoT(i*4+3) = i;
                
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

                // F.row(i*4+0) << (i*4)+0, (i*4)+1, (i*4)+3;
                // F.row(i*4+1) << (i*4)+0, (i*4)+2, (i*4)+1;
                // F.row(i*4+2) << (i*4)+3, (i*4)+2, (i*4)+0;
                // F.row(i*4+3) << (i*4)+1, (i*4)+2, (i*4)+3;

                

            }  
        }
        else 
        {   
            assert(false);
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
        m_gl_rend->SetClearColor(   Wm5::Vector4f(173.0/255, 216/255.0, 230/255.0, 1));

        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(Wm5::APoint(0,0,0), 1.0, 1.0, 2, viewport, true);
        
        //set up window callbacks
        //@TODO use std::bind instead
        m_tyro_window->callback_mouse_down = [&](Window& window, int button, int modifier)->bool 
        {
            return this->mouse_down(window, button, modifier);
        };

        m_tyro_window->callback_mouse_up = [&](Window& window, int button, int modifier)->bool 
        {
            return this->mouse_up(window, button, modifier);
        };

        m_tyro_window->callback_mouse_move = [&](Window& window, int mouse_x, int mouse_y)->bool 
        {
            return this->mouse_move(window, mouse_x, mouse_y);
        };

        m_tyro_window->callback_window_resize = [&](Window& window, unsigned int w, unsigned int h)->bool 
        {
            return this->window_resize(window, w, h);
        };

        m_tyro_window->callback_key_pressed = [&](Window& window, unsigned int key, int modifiers)->bool 
        {
            return this->key_pressed(window, key, modifiers);
        };

        m_tyro_window->callback_key_down = [&](Window& window, unsigned int key, int modifiers)->bool 
        {
            return this->key_down(window, key, modifiers);
        };
        
        m_tyro_window->callback_mouse_scroll = [&](Window& window, float ydelta)->bool 
        {
            return this->mouse_scroll(window, ydelta);
        };

        m_state = App::State::Launched;
        m_need_rendering = true;

        // load obj file
        //Eigen::MatrixXd V,N; // Vertex data. 3*num_vert by num_frames. 
        Eigen::MatrixXd UEC_T, UEC_S;
        Eigen::MatrixXi E_T, E_S, UE_T, UE_S; // Face data. 
        Eigen::VectorXi EMAP_S, EMAP_T; // Map directed edges to unique edges. 
        std::vector<std::vector<int> > uE2E_S, uE2E_T;

       
        Eigen::MatrixXd V_temp;
        Eigen::MatrixXd C_temp;
        Eigen::MatrixXi F_temp;
        Eigen::MatrixXi T_temp, T_temp2;
        Eigen::MatrixXd TC, CC; 
        Eigen::MatrixXi TI; 
        Eigen::MatrixXi TBF;

        // int maxbufsize;
        // glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxbufsize);
        // std::cout<< " GL_MAX_TEXTURE_BUFFER_SIZE " << maxbufsize <<std::endl;

        /*
         * LOAD TET MESH
         */


        // std::string tetmeshpath= "/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/dino/boneskin_coarse3.mesh";
        // std::string tetmeshpath= "/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/cube.mesh";
        std::string tetmeshpath="/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/cylinder3Dmusle.mesh";
        igl::readMESH(tetmeshpath, V_temp, T_temp, F_temp);
        // T_temp.resize(1,4);
        // T_temp.row(0) = T_temp2.row(4);

        /*
         *  COMPUTE BOUNDARY OF THE FACE
         */ 
        // igl::boundary_facets(T_one_tet, TBF); // boundary faces

        int texture_units;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
        std::cout  << "texture_units  " << texture_units << std::endl;

        /*
         * LOAD DIFFUSED VALUES
         */

        // Eigen::MatrixXd Z;
        // igl::deserialize(Z, "Z", "/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/dino/Z");
        // assert(Z.rows()>0);
        // Eigen::VectorXd MV, MV2;
        // Eigen::VectorXi MC;
        // Eigen::VectorXi MT;
        // MV = Z.col(1);  
        // MV.resize(V_temp.rows());
        // MV.setZero();

        Eigen::MatrixXd Z;
        // std::string readDMATpath="/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/cube.dmat";
        std::string readDMATpath="/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/cylinder3D.dmat";
        igl::readDMAT(readDMATpath, Z);
        // igl::deserialize(Z, "Z", "/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/dino/Z");
        assert(Z.rows()>0);
        Eigen::VectorXd MV, MV2;
        Eigen::VectorXi MC;
        Eigen::VectorXi MT;
        MV = Z.col(0);      

        
        

        /*
         * OVERWRITE Generate one tet 
         */ 
        #if 1
        T_temp.resize(1,4);
        V_temp.resize(4,3);
        Z.resize(4,1);
        
        T_temp <<  0, 1, 2, 3;// 0,7, 5, 4; //, 4, 5, 6, 7; //, 8, 9, 10, 11;
        // V_temp << -0.17114,  0.3437, -5.2931,
        //         -0.0024069, 0.33338, -5.3974,
        //           0.027116, 0.51782, -5.2421,
        //            0.13845, 0.33106, -5.2695;
        
        Z << 1, 0.8, 0.5, 0; //, 1, 0, 0, 0;//, 0.5, 0.6, 0.8, 0.9;
        MV = Z.col(0);  


        // T_temp << 0 ,1, 5,7;
                //   0 ,7, 5,4,
                //   0 ,1, 7,3,
                //   0 ,3, 7,2,
                //   0 ,6, 7,4,
                //   0 ,2, 7,6;

        // V_temp << 0, 0, 0,
        //         0, 1, 0,        
        //         1, 0, 0,
        //         1, 1, 0,
        //         0, 0, 1,
        //         0, 1, 1,
        //         1, 0, 1,
        //         1, 1, 1;
        // 0,     0,     0,
        //            0,     1,     0,
        //            0,     1,     1,
        //            1,     1,     1; 

        V_temp << 0, 0, 0, 
                  1, 0, 0, 
                  0.5, 1, 0, 
                  0.5, 0.5, 1;

        V_temp = 100*V_temp;
               
                  
                //   1, 1, 1, 
                //   2, 1, 1, 
                //   1.5, 2, 1, 
                //   1.5, 1.5, 2;
        // // V_temp << 1, 1, 1, 
        // //           2, 1, 1, 
        // //           1.5, 2, 1, 
        // //           1.5, 1.5, 2,
        // //           4, 4, 4, 
        // //           5, 4, 4, 
        // //           4.5, 5, 4, 
        // //           4.5, 4.5, 5;
                  
        #endif

        Eigen::VectorXi FtoT;
        triangulate_tets(T_temp, V_temp, TC, TI, MV, GEOMETRY.VT, GEOMETRY.FT, CC, MV2, FtoT, true);
        // igl::per_vertex_normals(GEOMETRY.VT,GEOMETRY.FT,GEOMETRY.NT);
        igl::per_face_normals(GEOMETRY.VT, GEOMETRY.FT, GEOMETRY.NT);
        
        // std::cerr<<igl::matlab_format(GEOMETRY.VT,"GEOMETRY.VT")<<std::endl;
        // std::cerr<<igl::matlab_format(GEOMETRY.FT,"GEOMETRY.FT")<<std::endl;
        // std::cerr<<igl::matlab_format(GEOMETRY.NT,"GEOMETRY.NT")<<std::endl;

        #if 0
        std::cerr<<igl::matlab_format(Z,"Z")<<std::endl;
        #endif   

        // std::string scan_obj = "/Users/rinat/Workspace/MuscleGeometrySrc/src/muscle_meshes/cylinder.obj";
        assert(GEOMETRY.VT.rows() == MV2.rows());

        // Eigen::MatrixXd N_temp;
        // igl::per_vertex_normals(V_temp,GEOMETRY.FT,N_temp);
        // RENDER.template_mesh = IGLMesh::Create(GEOMETRY.VT, GEOMETRY.FT, N_temp, Eigen::Vector3d(1,0,0));
        RENDER.template_mesh = MuscleMesh::Create(GEOMETRY.VT, GEOMETRY.FT, GEOMETRY.NT, MV, V_temp, T_temp, FtoT);

        igl::unique_edge_map(GEOMETRY.FT,E_T,UE_T,EMAP_T,uE2E_T);

        color_matrix(UE_T.rows(), Eigen::Vector3d(0.2,0.2,0.2), UEC_T);
        RENDER.template_mesh_wire = IGLMeshWireframe::Create(GEOMETRY.VT, UE_T, UEC_T);
        RENDER.template_mesh_wire->Visible = true;
        
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
        VisibleSet vis_set;

        RENDER.template_mesh->Update(true);
        if (RENDER.template_mesh->Visible) 
            vis_set.Insert(RENDER.template_mesh.get());
        
        RENDER.template_mesh_wire->Update(true);
        if (RENDER.template_mesh_wire->Visible)
            vis_set.Insert(RENDER.template_mesh_wire.get());

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
        
        m_camera = new iOSCamera(world_center, radius, aspect, 1, viewport, true);
    }

    bool App::mouse_down(Window& window, int button, int modifier) 
    {   
        //RA_LOG_INFO("mouse down %i", button);

        if (m_state != App::State::LoadedModel) return false;

        mouse_is_down = true;
        m_modifier = modifier;
        m_mouse_btn_clicked = button;

        if (m_modifier == TYRO_MOD_CONTROL) return false; //rotating
        if (button == 0 && m_modifier == TYRO_MOD_SHIFT) 
        {   
            //m_square_sel_start_x = current_mouse_x;
            //m_square_sel_start_y = current_mouse_y;
            return false; //selection
        }
        if (m_mouse_btn_clicked == 2) return false; //translating
        
        // Cast a ray in the view direction starting from the mouse position
        double x = current_mouse_x;
        double y = m_camera->GetViewport()[3] - current_mouse_y;
        //Eigen::Vector2f mouse_pos(x,y);
        //selectVertex(mouse_pos, button, modifier);     
        return true;
    }

    bool App::mouse_up(Window& window, int button, int modifier) 
    {   
        if (m_state != App::State::LoadedModel) return false;
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

        return true;
    }

    bool App::mouse_move(Window& window, int mouse_x, int mouse_y) 
    {   
        
        if (m_state != App::State::LoadedModel) return false; 
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

        return true;
        
    }

    bool App::mouse_scroll(Window& window, float ydelta) 
    {
        //RA_LOG_INFO("mouse scroll delta %f", ydelta);
        if (m_state != App::State::LoadedModel) return false;
        
        m_camera->HandlePinchGesture(gesture_state, Wm5::Vector2i(current_mouse_x, current_mouse_y), ydelta);
        render();
        return true;
    } 

    bool App::window_resize(Window& window, unsigned int w, unsigned int h)
    {
        //RA_LOG_INFO("window resized")
        //float  aspect = m_gl_rend->GetViewWidth()/ (float) m_gl_rend->GetViewHeight();
        //Vector4i viewport(0, 0, m_gl_rend->GetViewWidth(), m_gl_rend->GetViewHeight());
        //m_camera->SetAspect(aspect);
        //m_camera->SetViewport(viewport);
        
        render();   
        return true;
    }


    bool App::key_pressed(Window& window, unsigned int key, int modifiers) 
    {   
        RA_LOG_INFO("Key pressed %c", key);
        
        if (key == '`') 
        {   
            //RA_LOG_INFO("Pressed %c", key);
            RENDER.template_mesh_wire->Visible = !RENDER.template_mesh_wire->Visible;
            // show_console = !show_console;
            render();
            return false;
        }

        if (show_console) 
        {  
           m_console.keyboard(key);
           render();
           return false;
        }
        else 
        {
            
        }

        return true;
    }
    
    bool App::key_down(Window& window, unsigned int key, int modifiers) 
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

        return true;
    }

    void App::register_console_function(const std::string& name,
                                        const std::function<void(App*, const std::vector<std::string>&)>& con_fun,
                                        const std::string& help_txt)
    {
        std::function<void (const std::vector<std::string>&)> f = bind(con_fun, this, std::placeholders::_1);
        m_console.reg_cmdN(name, f, help_txt);
    }
}