#include "TyroApp.h"

#include "RAEnginePrerequisites.h"
#include "RAES2TextOverlay.h"
#include "RAFont.h"
#include "RAVisibleSet.h"
#include "RAES2StandardMesh.h"
#include "RAAxisAlignedBBox.h"

#include <functional>

#include "Wm5APoint.h"
#include "Wm5Vector2.h"
#include "Wm5Vector4.h"

#include <igl/extract_manifold_patches.h>
#include <igl/unproject_onto_mesh.h>
#include <igl/readOBJ.h>
#include <igl/per_vertex_normals.h>
#include <igl/writeSTL.h>
#include <igl/readSTL.h>
#include <igl/writeOBJ.h>
#include <igl/remove_duplicate_vertices.h>
#include <igl/oriented_facets.h>
#include <igl/is_edge_manifold.h>
#include <igl/edge_flaps.h>
#include <igl/remove_unreferenced.h>
#include <igl/slice.h>
#include <igl/project.h>
#include <igl/min_quad_with_fixed.h>
#include <igl/upsample.h>
#include <igl/adjacency_list.h>

#include <filesystem/path.h>

#include "TyroIGLMesh.h"
#include "TyroFileUtils.h"

#include "load_mesh_sequence.h"
#include "compute_deformation.h"
#include "stop_motion.h"
#include "mesh_split.h"
#include "segmentation.h"



using namespace std;

using Eigen::VectorXi;
using Eigen::VectorXd;

using Eigen::MatrixXi;
using Eigen::MatrixXd;

using Eigen::Vector3i;
using Eigen::Vector3f;
using Eigen::RowVector3d;

using Wm5::APoint;

//converte random soup of unique edges into set of directed edges that form a loop
/*
void tyro::get_edge_loop(const Eigen::MatrixXi& SUE,
                         Eigen::MatrixXi& eid_list, // edges from vid_list
                         Eigen::VectorXi& EI, // indicies into directed edges matrix
                         Eigen::VectorXi& uEI, // indicies into undirected edges matrix
                         Eigen::VectorXi& DMAP
                         ) 
{

}
*/
void tyro::copy_animation(const tyro::App::MAnimation& source, 
                          tyro::App::MAnimation& dest, 
                          bool topology, 
                          bool face_color, 
                          bool edge_color) 
{
    if (topology) 
    {
        dest.f_data = source.f_data;
        dest.e_data = source.e_data;
        dest.ue_data = source.ue_data;
        dest.EMAP = source.EMAP;
    }

    if (face_color)
        dest.fc_data = source.fc_data;
    
    if (edge_color)
        dest.ec_data = source.ec_data;
}

void tyro::color_black_matrix(int rows, Eigen::MatrixXd& uC)
{
    uC.resize(rows, 3);
    uC.setZero();
}

void tyro::color_matrix(int rows, const Eigen::Vector3d& cv, Eigen::MatrixXd& uC)
{
    uC.resize(rows, 3);
    for (int e = 0; e<uC.rows(); ++e) 
    {
        uC.row(e) = cv;
    }
}

void tyro::convert_vertex_to_edge_selection(const std::vector<int>& vid_list,
                                            const Eigen::MatrixXi& E, //all directed edges
                                            const Eigen::MatrixXi& uE, //all unique edges
                                            const Eigen::VectorXi& EMAP, // map from directed to unique edge index 
                                            Eigen::MatrixXi& eid_list, // edges from vid_list
                                            Eigen::VectorXi& EI, // indicies into directed edges matrix
                                            Eigen::VectorXi& uEI, // indicies into undirected edges matrix
                                            Eigen::VectorXi& DMAP) // checks which directions where switched HACKY
{
    //create a closed edge loop from vertex selection
    int num_edges = vid_list.size(); //assumes its a closed loop
    eid_list.resize(num_edges, 2);
    for (int i = 0; i < num_edges; ++i) 
    {   
        if (i == num_edges - 1) //last edge
            eid_list.row(i) = Eigen::Vector2i(vid_list[i], vid_list[0]);
        else
            eid_list.row(i) = Eigen::Vector2i(vid_list[i], vid_list[i+1]);
    }

    EI.resize(num_edges);
    uEI.resize(num_edges);
    DMAP.resize(num_edges);
    DMAP.setZero();
    int num_found = 0;
    for (int i = 0; i < eid_list.rows(); ++i) 
    {
        VectorXi e1 = eid_list.row(i);
        for (int j = 0; j < E.rows(); ++j) 
        {
            VectorXi e2 = E.row(j);
            if (e1(0) == e2(0) && e1(1) == e2(1)) 
            {
                EI(i) = j; //directed edge index
                uEI(i) = EMAP(j); //unique edge index
                Eigen::Vector2i evec = uE.row(uEI(i));
               
                //if direction is switched after a mapping directed to undirected
                if (!(evec(0)== e1(0) && evec(1) == e1(1))) 
                {
                    DMAP(i) = 1;
                }
                num_found++;
                break;
            }
        }
    }

    assert(num_found == num_edges);
}


namespace tyro
{   
    namespace
    {   
        void console_laplacian_smooth_along_edges(App* app, const std::vector<std::string>& args) 
        {
              using Eigen::Vector3d;
            //Taubian smoothing
            if (app->vid_list.size() > 0 ) 
            {   
                for (int iter = 0; iter < 1; ++iter) 
                {   
                    const auto& FDV = app->m_frame_data.avg_v_data;
                    MatrixXd Voriginal = app->m_frame_data.avg_v_data.eval();

                    for (int i = 0; i < app->vid_list.size(); ++i) 
                    {   
                        Vector3d Lv;
                        Lv.setZero();
                        
                        int left = i-1; 
                        int right = i+1; 
                        
                        //check if v is boundary, then dont smooth
                        if (i==0) 
                        {
                            left = app->vid_list.size()-1;
                            right = 1;  
                        }
                        else if (i == app->vid_list.size()-1) 
                        {
                            left = i-1;
                            right = 0;  
                        }
                        
                        Vector3d v = FDV.row(app->vid_list[i]);
                        Vector3d v1 = FDV.row(app->vid_list[left]);
                        Vector3d v2 = FDV.row(app->vid_list[right]);  

                        double w1 = 0.5;//1.0/(1000*(v1 - v).squaredNorm());
                        double w2 = 0.5;//1.0/(1000*(v2 - v).squaredNorm());
                        
                        Lv += (w1)*v1;
                        Lv += (w2)*v2;
                        
                        //Lv += app->m_frame_data.v_data[app->m_frame].row(vid_n);
                        Lv = (1.0/(w1+w2)) * Lv - v;
                        //std::cout<< Lv;
                        Voriginal.row(app->vid_list[i]) +=  0.5*Lv;
                        
                    }
                    app->m_frame_data.avg_v_data = Voriginal.eval();
                }

                
                app->render();
                glfwPostEmptyEvent;
            }
            
        }
        

        void console_taubin_smooth_along_edges(App* app, const std::vector<std::string>& args) 
        {   
            using Eigen::Vector3d;
            //Taubian smoothing
            if (app->vid_list.size() > 0) 
            {   

                for (int iter = 0; iter < 6; ++iter) 
                {   
                    const auto& FDV = app->m_frame_data.avg_v_data;
                    MatrixXd Voriginal = app->m_frame_data.avg_v_data.eval();

                    for (int i = 0; i < app->vid_list.size(); ++i) 
                    {   
                        Vector3d Lv;
                        Lv.setZero();
                        
                        int left = i-1; 
                        int right = i+1; 
                        
                        //check if v is boundary, then dont smooth
                        if (i==0) 
                        {
                            left = app->vid_list.size()-1;
                            right = 1;  
                        }
                        else if (i == app->vid_list.size()-1) 
                        {
                            left = i-1;
                            right = 0;  
                        }
                        
                        Vector3d v = FDV.row(app->vid_list[i]);
                        Vector3d v1 = FDV.row(app->vid_list[left]);
                        Vector3d v2 = FDV.row(app->vid_list[right]);  

                        double w1 = 0.5;//1.0/(1000*(v1 - v).squaredNorm());
                        double w2 = 0.5;//1.0/(1000*(v2 - v).squaredNorm());
                        
                        Lv += (w1)*v1;
                        Lv += (w2)*v2;
                        
                        //Lv += app->m_frame_data.v_data[app->m_frame].row(vid_n);
                        Lv = (1.0/(w1+w2)) * Lv - v;
                        //std::cout<< Lv;
                        if (iter%2 == 0)
                            Voriginal.row(app->vid_list[i]) +=  0.5*Lv;
                        else
                            Voriginal.row(app->vid_list[i]) += -0.55*Lv;
                        
                    }
                    app->m_frame_data.avg_v_data = Voriginal.eval();
                }

                
                app->render();
                glfwPostEmptyEvent;
            }
            
        }
        
        void console_save_serialised_data(App* app, const std::vector<std::string>& args) 
        {   
            if (args.size() != 2) 
                return; 

            auto type = args[0];
            auto filename = args[1];

            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path(filename);
            std::ofstream os(p.str(), std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            
            if (type == "frames") 
            {
                archive(app->m_frame_data);
            } 
            else if (type == "deform") 
            {
                archive(app->m_frame_deformed_data);
            }
            else if (type == "split") 
            {
                archive(app->m_pieces);
            }
            else if (type == "stop") 
            {
                archive(app->m_stop_motion);
            }
            else if (type == "stop_low") 
            {
                archive(app->m_stop_motion[1]);
            }
            else if (type == "stop_up") 
            {
                archive(app->m_stop_motion[0]);
            }
        }

        void console_load_serialised_data(App* app, const std::vector<std::string>& args) 
        {   
            if (args.size() < 2) 
                return; 
            
            auto type = args[0];
            auto filename = args[1];

            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path(filename);

            std::ifstream in = std::ifstream(p.str(), std::ios::binary);
            cereal::BinaryInputArchive archive(in);
            
            if (type == "frames") 
            {
                archive(app->m_frame_data);

                app->m_timeline->SetFrameRange(app->m_frame_data.v_data.size()-1);

                //Compute radius of the bounding box of the model
                AxisAlignedBBox bbox;
                MatrixXd VT = app->m_frame_data.v_data[0].transpose();
                bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
                app->m_model_offset = bbox.GetRadius(); 

                app->m_update_camera = true;
                app->m_state = App::State::LoadedModel;
            } 
            else if (type == "deform") 
            {
                archive(app->m_frame_deformed_data);
                app->m_computed_deformation = true;
                app->m_update_camera = true;
            }
            else if (type == "split") 
            {
                archive(app->m_pieces);
                app->m_computed_parts = true;            
                app->m_update_camera = true;
            }
            else if (type == "stop") 
            {
                archive(app->m_stop_motion);
                for (int i = 0; i < app->m_stop_motion.size(); ++i)
                    if (app->m_stop_motion[i].anim.v_data.size() > 0)
                        app->m_stop_motion[i].computed = true;
                
                app->m_computed_stop_motion = true;
                app->m_update_camera = true;

            }
            else if (type == "stop_low") 
            {   
                if (app->m_stop_motion.empty())
                    app->m_stop_motion.resize(2);
                
                archive(app->m_stop_motion[1]);
                app->m_stop_motion[1].computed = true;
                
                app->m_computed_stop_motion = true;
                app->m_update_camera = true;
            }
            else if (type == "stop_up") 
            {
                if (app->m_stop_motion.empty())
                    app->m_stop_motion.resize(2);
                
                archive(app->m_stop_motion[0]);
                app->m_stop_motion[0].computed = true;
                
                app->m_computed_stop_motion = true;
                app->m_update_camera = true;
            }
            app->render();
            glfwPostEmptyEvent();
        }

        void console_clear_selection(App* app, const std::vector<std::string>& args) 
        {
            app->vid_list.clear();
            app->ball_list.clear();
            
            for (int fid = 0; fid < app->m_frame_data.f_data.rows(); ++fid) 
            {
                app->setFaceColor(fid, false);
            }
            app->fid_list.clear();        
            app->fid_list2.clear();
            app->fid_list3.clear();
            app->render();
        }

        void console_clear_vertex_selection(App* app, const std::vector<std::string>& args) 
        {
            app->vid_list.clear();
            app->ball_list.clear();
            app->render();
        }

        void console_clear_face_selection(App* app, const std::vector<std::string>& args) 
        {
            for (int fid = 0; fid < app->m_frame_data.f_data.rows(); ++fid) 
            {
                app->setFaceColor(fid, false);
            }
            app->fid_list.clear();        
            app->fid_list2.clear();
            app->fid_list3.clear();
            app->render();
        }

        void console_set_vertex_weight(App* app, const std::vector<std::string>& args) 
        {
            if (args.size()>0) 
            {   
                float w = std::stof(args[0]);
                for (auto vid : app->vid_list) 
                {
                    app->m_weights.VW(vid) = w;
                }
            }
        }

        void console_set_face_weight(App* app, const std::vector<std::string>& args) 
        {
            if (args.size()>0) 
            {   
                float w = std::stof(args[0]);
                for (auto fid : app->fid_list) 
                {
                    app->m_weights.FW(fid) = w;
                }
            }
        }

        void console_draw_vertex_weight_map(App* app, const std::vector<std::string>& args) 
        {   
            double maxC = app->m_weights.VW.maxCoeff();
            double minC = app->m_weights.VW.minCoeff(); //should be 1
            
            for (int vid = 0; vid < app->m_weights.VW.size(); ++vid) 
            {   
                double w = app->m_weights.VW(vid); 
                if (w > 1) 
                {   
                    double red = w / (maxC - minC); 
                    app->addSphere(vid, Wm5::Vector4f(red, 0, 0, 1));
                }
            }
        }

        void console_draw_face_weight_map(App* app, const std::vector<std::string>& args) 
        {   
            double maxC = app->m_weights.FW.maxCoeff();
            double minC = app->m_weights.FW.minCoeff(); //should be 1
            
            for (int fid = 0; fid < app->m_weights.FW.size(); ++fid) 
            {   
                double w = app->m_weights.FW(fid); 
                //if (w > 1) 
                //{   
                    double green = w / (maxC - minC); 
                    Eigen::Vector3d clr(0, green, 0);
                    app->setFaceColor(fid, clr);
                //}
            }
        } 

        void console_segmentation(App* app, const std::vector<std::string>& args) 
        {  
            if (!(app->fid_list.size() > 0 && 
                app->fid_list2.size() > 0 &&
                app->fid_list3.size() > 0 &&
                args.size() == 1)) 
            { 
                return;   
            }   

            double smooth_weight = (double)std::stoi(args[0]);
            
            const auto& FD = app->m_frame_data;

            VectorXi L;
            VectorXi flist1 = Eigen::Map<VectorXi>(app->fid_list.data(), 
                                                   app->fid_list.size());
            MatrixXi F;
            igl::slice(FD.f_data, flist1, 1, F); 

            VectorXi seeds2, seeds3; //foreground and background         
            seeds2.resize(app->fid_list2.size());
            seeds3.resize(app->fid_list3.size());
            
            for (int i = 0; i < app->fid_list2.size(); ++i)
            {
                for (int j = 0; j < app->fid_list.size(); ++j) 
                {
                    if (app->fid_list2[i] == app->fid_list[j]) 
                    {
                        seeds2(i) = j;
                    }
                }
            }

            for (int i = 0; i < app->fid_list3.size(); ++i)
            {
                for (int j = 0; j < app->fid_list.size(); ++j) 
                {
                    if (app->fid_list3[i] == app->fid_list[j]) 
                    {
                        seeds3(i) = j;
                    }
                }
            }

            assert(seeds2.maxCoeff() < F.rows());
            assert(seeds3.maxCoeff() < F.rows());

            std::vector<MatrixXd> v_data;
            MatrixXi NF, I;
            for (int i = 0; i < FD.v_data.size(); ++i) 
            {   
                MatrixXd NV;
                igl::remove_unreferenced(FD.v_data[i], F, NV, NF, I);
                v_data.push_back(NV);
            }

            MatrixXd Vavg;
            igl::remove_unreferenced(FD.avg_v_data, F, Vavg, NF, I);
                    
            tyro::segmentation(v_data, 
                               NF,
                               Vavg,
                               seeds2,
                               seeds3,
                               smooth_weight,
                               L);
            
            for (int i = 0; i < L.size(); ++i) 
            {
                if (L(i) == 1) 
                {
                    app->setFaceColor(flist1[i], Eigen::Vector3d(0,0.5,0));
                }
                else 
                {
                    app->setFaceColor(flist1[i], Eigen::Vector3d(0.5,0,0));                
                }
            }

            //TODO:find boundary
            /*
            const auto & cE = FD.ue_data;
            const auto & cEMAP = FD.EMAP;
            MatrixXi EF, EI;

            igl::edge_flaps(FD.f_data, 
                            cE, 
                            cEMAP, 
                            EF, 
                            EI);
            
            for (int i = 0; i < FD.ue_data.rows(); ++i) 
            {               
                int f1 = EF(i, 0);
                int f2 = EF(i, 1);
                
                auto it1 = std::find(app->fid_list.begin(), app->fid_list.end(), f1);
                auto it2 = std::find(app->fid_list.begin(), app->fid_list.end(), f2);
                
                if (it1 != app->fid_list.end() && it2 != app->fid_list.end()) 
                {   
                    int idx1 = it1 - app->fid_list.begin();
                    int idx2 = it2 - app->fid_list.begin();
                    if (L(idx1) != L(idx2)) 
                    {
                        app->eid_list.push_back(i);                        
                        app->m_frame_data.ec_data.row(i) = Eigen::Vector3d(0,0.0,0.8);

                        Eigen::Vector2i evec = FD.ue_data.row(i);
                       //if direction is switched after a mapping directed to undirected
                        if (!(evec(0)== e1(0) && evec(1) == e1(1))) 
                        {
                            DMAP(i) = 1;
                        }
                    }
                }
            }    */ 
        }
        
        void console_upsample(App* app, const std::vector<std::string> & args) 
        {
            auto& FD = app->m_frame_data;
            Eigen::SparseMatrix<double> S;
            Eigen::MatrixXi newF;
            igl::upsample(FD.v_data[0].rows(), FD.f_data, S, newF);
            FD.f_data = newF;

            for (int i =0; i < FD.v_data.size(); ++i) 
            {
                FD.v_data[i] = S * FD.v_data[i];
                
                Eigen::MatrixXd N;
                int num_face = FD.f_data.rows();
                igl::per_vertex_normals(FD.v_data[i], FD.f_data, N); 
                FD.n_data[i] = N;

                std::vector<std::vector<int> > uE2E;
                igl::unique_edge_map(FD.f_data,
                                     FD.e_data,
                                     FD.ue_data,
                                     FD.EMAP,
                                     uE2E);
                Eigen::Vector3d face_color(0.5,0.5,0.5);
                tyro::color_matrix(FD.f_data.rows(), face_color, FD.fc_data);
                tyro::color_black_matrix(FD.ue_data.rows(), FD.ec_data);
            }

            app->compute_average();                       
        }

        void console_show_wireframe(App* app, const std::vector<std::string> & args) 
        {
            if (args.size() == 1) 
            {
                int show = std::stoi(args[0]);
                app->m_show_wireframe = show;
                app->render();                
            }
        }

        void console_deselect_faces(App* app, const std::vector<std::string> & args) 
        {
            for (auto fid : app->fid_list) 
            {
                app->setFaceColor(fid, false);
            }
            app->fid_list.clear();
            app->render();
            glfwPostEmptyEvent(); 
        }

        void console_deselect_verticies(App* app, const std::vector<std::string> & args) 
        {
            app->removeSpheres(app->vid_list);
            app->vid_list.clear();
            app->render();
            glfwPostEmptyEvent();
        }

        void console_show_edge_selection(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Converting vertex to edge selection");
            
            MatrixXi eid_list;
            VectorXi EI, uEI, DMAP;
            convert_vertex_to_edge_selection(app->vid_list, 
                                             app->m_frame_data.e_data, 
                                             app->m_frame_data.ue_data, 
                                             app->m_frame_data.EMAP,
                                             eid_list, 
                                             EI, 
                                             uEI, 
                                             DMAP);
        
            for (int i = 0; i < uEI.size(); ++i) 
            {   
                app->m_frame_data.ec_data.row(uEI(i)) = Eigen::Vector3d(0,0.8,0);
            }
            
            //debug_show_faces_near_edge_selection(uEI, DMAP);       
                    
            app->render();
            glfwPostEmptyEvent();
            
            return;
        }

        void console_split_mesh(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Splitting mesh");
            
            if (app->vid_list.size() == 0 || app->m_frame_deformed_data.v_data.size() == 0) 
            {   
                RA_LOG_INFO("Not enough information");
                return;
            }

            if (!igl::is_edge_manifold(app->m_frame_deformed_data.f_data)) 
            {   
                Eigen::MatrixXi P;
                igl::extract_manifold_patches(app->m_frame_deformed_data.f_data, P);
                //int a = P.minCoeff();
                //int b = P.maxCoeff();
                using Eigen::Vector3d;
                int a = 0, b = 0, c = 0;

                for (int i = 0; i < P.rows(); ++i) 
                {   
                    Vector3d clr;
                    if (P(i,0)==0) {
                        clr = Vector3d(1,0,0);
                        a++;}
                    
                    if (P(i,0)==1){ 
                        clr = Vector3d(0,1,0);
                        b++;}

                    if (P(i,0)==2) { 
                        clr = Vector3d(0,0,1);
                        c++;}

                    app->setFaceColor(i, clr);
                }
                RA_LOG_ERROR("Mesh is not edge manifold");
                return;
            }

            MatrixXi eid_list;
            VectorXi EI, uEI, DMAP;
            convert_vertex_to_edge_selection(app->vid_list, 
                                             app->m_frame_deformed_data.e_data, 
                                             app->m_frame_deformed_data.ue_data, 
                                             app->m_frame_deformed_data.EMAP,
                                             eid_list, 
                                             EI, 
                                             uEI, 
                                             DMAP);
        

            MatrixXi F1, F2;
            tyro::mesh_split(app->m_frame_deformed_data.f_data,
                             uEI,
                             DMAP, 
                             F1, 
                             F2);
            
            app->m_pieces.resize(2);
            auto& A1 = app->m_pieces[0];
            auto& A2 = app->m_pieces[1];
            A1.v_data.resize(app->m_frame_deformed_data.v_data.size());
            A2.v_data.resize(app->m_frame_deformed_data.v_data.size());
            A1.n_data.resize(app->m_frame_deformed_data.v_data.size());
            A2.n_data.resize(app->m_frame_deformed_data.v_data.size());
            
            A1.sequenceIdx = app->m_frame_data.sequenceIdx;
            A2.sequenceIdx = app->m_frame_data.sequenceIdx;
            
            for (int i = 0; i < app->m_frame_data.v_data.size(); ++i) 
            {                   
                MatrixXi I1, I2;    
                igl::remove_unreferenced(app->m_frame_deformed_data.v_data[i], 
                                         F1, 
                                         A1.v_data[i], 
                                         A1.f_data, 
                                         I1);

                igl::per_vertex_normals(A1.v_data[i], A1.f_data, A1.n_data[i]);
                std::vector<std::vector<int> > uE2E1;
                igl::unique_edge_map(A1.f_data,A1.e_data,A1.ue_data,A1.EMAP,uE2E1);
                tyro::color_matrix(A1.f_data.rows(), Eigen::Vector3d(0.2,0.2,0.2), A1.fc_data);
                tyro::color_black_matrix(A1.e_data.rows(), A1.ec_data);

                igl::remove_unreferenced(app->m_frame_deformed_data.v_data[i], 
                                         F2, 
                                         A2.v_data[i], 
                                         A2.f_data, 
                                         I2);
                
                igl::per_vertex_normals(A2.v_data[i], A2.f_data, A2.n_data[i]);
                std::vector<std::vector<int> > uE2E2;
                igl::unique_edge_map(A2.f_data,A2.e_data,A2.ue_data,A2.EMAP,uE2E2);
                tyro::color_matrix(A2.f_data.rows(), Eigen::Vector3d(0.6,0.6,0.6), A2.fc_data);
                tyro::color_black_matrix(A2.e_data.rows(), A2.ec_data);
            }

                
            app->m_computed_parts = true;            
            app->m_update_camera = true;

            return;
        }

        void console_frame(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Jump to a new frame");

            if (args.size() == 1)
            {   int frame = std::stoi(args[0]);
                app->m_timeline->SetFrame(frame);
                return;
            }
        }

        void console_stop_motion(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("running stop motion");

            if (args.size() == 4)
            {   
                int num_labels = std::stoi(args[0]);
                double smooth_weight = std::stof(args[1]);
                int part_id = std::stoi(args[2]);
                std::string initmethod = args[3]; 
                
                if (app->m_stop_motion.empty())
                //app->m_stop_motion.clear();
                    app->m_stop_motion.resize(app->m_pieces.size());

                int start, end;
                if (part_id == -1) 
                {
                    start = 0;
                    end = app->m_stop_motion.size();
                }
                else
                {   
                    //just do one part
                    start = part_id;
                    end = part_id+1;
                }
                
                for (int i = start; i < end; ++i) 
                {   
                    double result_energy;
                    auto& sm = app->m_stop_motion[i]; 
                    auto& piece = app->m_pieces[i];
                    bool kmeans = false;
                    if (initmethod == "kmeans") kmeans = true; 
                    tyro::stop_motion_vertex_distance(num_labels, 
                                                      smooth_weight,
                                                      kmeans,
                                                      piece.v_data,
                                                      piece.sequenceIdx,
                                                      piece.f_data,
                                                      sm.D, //dictionary
                                                      sm.L, //labels,  
                                                      result_energy);
                    
                    sm.anim.f_data = piece.f_data;
                    sm.anim.e_data = piece.e_data;
                    sm.anim.ue_data = piece.ue_data;
                    sm.anim.EMAP = piece.EMAP;
                    sm.anim.ec_data = piece.ec_data;
                    sm.anim.fc_data = piece.fc_data;
                    //precompute normals
                    std::vector<MatrixXd> normals;
                    normals.resize(sm.D.size());
                    for (int j = 0; j < sm.D.size(); ++j) 
                    {   
                        igl::per_vertex_normals(sm.D[j], sm.anim.f_data, normals[j]);
                    }

                    for (int j = 0; j < sm.L.size(); ++j) 
                    {
                        int l_idx = sm.L(j);
                        sm.anim.v_data.push_back(sm.D[l_idx]);
                        sm.anim.n_data.push_back(normals[l_idx]);
                    }
                    sm.computed = true;                   
                }

                app->m_computed_stop_motion = true;
                app->m_update_camera = true;

                app->render();
                glfwPostEmptyEvent();
            }
        }

        void console_align_all_models(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("aligning models");
            app->align_all_models();
            return;
        }

        void console_save_mesh_sequence_with_selected_faces(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Saving mesh sequence with selected faces");
            
            if (args.size() == 2 && app->fid_list.size() > 0) 
            {   
                auto& type = args[0];
                auto wheretosave = filesystem::path(args[1]);
                
                MatrixXi newF;
                VectorXi slice_list = Eigen::Map<VectorXi>(app->fid_list.data(), app->fid_list.size());
                igl::slice(app->m_frame_data.f_data, slice_list, 1, newF);
                int start_frame = 0;

                //check that the slice is manifold
                if (!igl::is_edge_manifold(newF)) 
                {
                    Eigen::MatrixXi P;
                    igl::extract_manifold_patches(newF, P);
                    std::vector<int> Pindx;
                    for (int fid=0; fid < P.rows(); ++fid) 
                    {
                        if (P(fid,0) == 0) Pindx.push_back(fid); //TODO HACK assume path 0 is the largest one
                    }
                    MatrixXi manifoldF;
                    slice_list = Eigen::Map<VectorXi>(Pindx.data(), Pindx.size());
                    igl::slice(newF, slice_list, 1, manifoldF);
                    newF = manifoldF;
                }

                if (type == "obj") 
                {
                    for (int file = 0; file < app->m_frame_data.sequenceIdx.size(); ++file) 
                    {   
                        auto folder = filesystem::path(app->FOLDERS[file])/wheretosave;

                        if (!folder.exists()) 
                        {
                            filesystem::create_directory(folder);
                        }
                        
                        auto objlist_path = folder/filesystem::path("objlist.txt");
                        int num_frames = app->m_frame_data.sequenceIdx[file];
                        
                        ofstream objlist_file;
                        objlist_file.open (objlist_path.str());
                    
                        for (int frame = start_frame; frame < start_frame + num_frames; ++frame) 
                        {   
                            assert(frame < app->m_frame_data.v_data.size());
                            MatrixXd temp_V;
                            MatrixXi temp_F;
                            VectorXi I;
                            igl::remove_unreferenced(app->m_frame_data.v_data[frame], newF, temp_V, temp_F, I);
                            auto file_name = filesystem::path(tyro::pad_zeros(frame) + std::string(".obj"));
                            auto file_path = folder/file_name; 
                            igl::writeOBJ(file_path.str(), temp_V, temp_F);
                            objlist_file << file_name << "\n";
                        }
                        
                        start_frame += num_frames;
                        
                        objlist_file.close();             
                    } 
                }
                else if (type == "binary")
                {     
                    App::MAnimation to_save;
                    to_save.v_data.resize(app->m_frame_data.v_data.size());
                    to_save.n_data.resize(app->m_frame_data.v_data.size());
                    for (int frame = 0; frame < app->m_frame_data.v_data.size(); ++frame) 
                    {   
                        MatrixXi temp_F;
                        VectorXi I;
                        igl::remove_unreferenced(app->m_frame_data.v_data[frame], newF, to_save.v_data[frame], temp_F, I);
                        igl::per_vertex_normals(to_save.v_data[frame], temp_F, to_save.n_data[frame]);
                        if (frame == 0) 
                        {
                            to_save.f_data =temp_F;
                        }
                    }  
                
                    std::vector<std::vector<int> > uE2E;
                    igl::unique_edge_map(to_save.f_data, 
                                         to_save.e_data,
                                         to_save.ue_data,
                                         to_save.EMAP,
                                         uE2E);
                    
                    //@TODO need this to update camera
                    Eigen::Vector3d face_color(0.5,0.5,0.5);
                    tyro::color_matrix(to_save.f_data.rows(), face_color, to_save.fc_data);
                    tyro::color_black_matrix(to_save.ue_data.rows(), to_save.ec_data);

                    to_save.sequenceIdx = app->m_frame_data.sequenceIdx;

                    auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
                    auto p = f/filesystem::path(wheretosave);
                    std::ofstream os(p.str(), std::ios::binary);
                    cereal::BinaryOutputArchive archive(os);
                    archive(to_save);
                }
            }
    
            return;

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

        void console_set_sel_primitive(App* app, const std::vector<std::string> & args)
        {
            RA_LOG_INFO("Set selection type");

            if (args.size() == 1)
            {      
                if (args[0] == "vertex") 
                    app->set_sel_primitive(App::SelectionPrimitive::Vertex);
                else if (args[0] == "faces")
                    app->set_sel_primitive(App::SelectionPrimitive::Faces);
                else if (args[0] == "edges")
                    app->set_sel_primitive(App::SelectionPrimitive::Edges);
                return;
            }
        }

        void console_set_sel_method(App* app, const std::vector<std::string> & args)
        {
            RA_LOG_INFO("Set selection type");

            if (args.size() == 1)
            {      
                if (args[0] == "oneclick") 
                    app->set_sel_method(App::SelectionMethod::OneClick);
                else if (args[0] == "square")
                    app->set_sel_method(App::SelectionMethod::Square);
                
                return;
            }
        }

        void console_load_oldman(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Loading oldman obj sequence");
            app->load_oldman();
            return;
        }
        
        void console_load_monka(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("load monka");
            bool serialized = true;
            if (args.size() == 1) 
                serialized = std::stoi(args[0]);

            //Picked face_id 26093 vertex_id 13681 coord 11.950990 2.934150 16.501955
            //face_id 40217 vertex_id 10585 coord -1.194749 -0.823966 1.658947
            //Picked face_id 31779 vertex_id 12996 coord 0.012255 1.557947 0.744930
            int offset_vid = 12996; // 1222;
            auto offset = Eigen::Vector3d(0.012255, 1.557947, 0.744930) ; //Eigen::Vector3d(0.613322, 2.613381, 2.238946);
        
            app->FOLDERS = 
            {   
                            
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/02/"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/03/"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/06/"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/08/"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/09/"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/10/"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/11/"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/12/"),
                //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/01/13/"),
                
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/03/01/"),
                
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/05/03/"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/05/04/"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/05/06/"),
         
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/data/production/obj/06/01/")
            };

            if (serialized) 
            {
                auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
                auto p = f/filesystem::path("monka_manifold");
                std::ifstream in = std::ifstream(p.str(), std::ios::binary);
                cereal::BinaryInputArchive archive_i(in);
                archive_i(app->m_frame_data);
            }
            else
            {
                std::vector<std::string> obj_paths;
                for (int i = 0; i < app->FOLDERS.size(); ++i) 
                {
                    int num_files_read;

                    //Add smth
                    auto folder = app->FOLDERS[i];
                    folder += std::string("face/");

                    RA_LOG_INFO("loading folder %s", folder.data());
                    tyro::obj_file_path_list(folder, "objlist.txt", obj_paths, num_files_read);
                    RA_LOG_INFO("frames read %i", num_files_read);
                    app->m_frame_data.sequenceIdx.push_back(num_files_read);
                    app->load_mesh_sequence(obj_paths, true); //use IGL obj loader
                }
            }

            
            if (!igl::is_edge_manifold(app->m_frame_data.f_data)) 
            {   
                RA_LOG_ERROR_ASSERT("not manifold");
                return;
            }
            app->m_timeline->SetFrameRange(app->m_frame_data.v_data.size()-1);

            //app->align_all_models(offset_vid, offset);
                
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->m_frame_data.v_data[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            app->compute_average();

            app->m_weights.VW.resize(app->m_frame_data.v_data[0].rows());
            app->m_weights.VW.setOnes();

            app->m_weights.FW.resize(app->m_frame_data.f_data.rows());
            app->m_weights.FW.setOnes();
            
            app->compute_average();

            app->render();
        }

        void console_load_bunny(App* app, const std::vector<std::string> & args) 
        {   
            RA_LOG_INFO("Loading bunny obj sequence");
            
            if (args.size() == 1) 
            {
                int a = std::stoi(args[0]);
                app->load_bunny(a);
            }
            else 
            {
                app->load_bunny();
            }
            return;
        }

        void console_load_blobby(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Loading blobby obj sequence");
            app->load_blobby();
        }

        void console_compute_average(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("compute_average");
            app->compute_average();
        }

        void console_compute_deformation(App* app, const std::vector<std::string> & args)
        {
            RA_LOG_INFO("computing deformation");

            if (app->vid_list.empty() || app->fid_list.empty() || !app->m_computed_avg) 
            { 
                RA_LOG_WARN("Need vertex/face selection and average mesh to compute deformation")
                return;
            }
            //app->m_frame_deformed_data.v_data.clear();
            bool result = tyro::compute_deformation(app->vid_list, 
                                                    app->fid_list,
                                                    app->m_frame_data.v_data,
                                                    app->m_frame_data.f_data,
                                                    app->m_frame_data.avg_v_data,
                                                    app->m_frame_deformed_data.v_data);
            assert(result);
            tyro::copy_animation(app->m_frame_data, app->m_frame_deformed_data, true, true, true);
            app->m_computed_deformation = true;
            app->m_update_camera = true;
            //app->render();
        }

          void console_invert_face_selection(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("invert face selection");
            app->invert_face_selection();
            return;
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
    m_sel_primitive(App::SelectionPrimitive::Faces),
    m_sel_method(App::SelectionMethod::OneClick),
    m_computed_stop_motion(false),
    m_update_camera(false),
    m_frame_overlay(nullptr),
    m_computed_parts(false),
    m_show_wireframe(true),
    add_seg_faces(false)
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
        m_gl_rend->SetClearColor(Wm5::Vector4f(0.0, 153.0/255.0, 153.0/255.0, 1.0));
 
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
            glfwPostEmptyEvent();
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

        register_console_function("load_blobby", console_load_blobby, "");
        register_console_function("load_oldman", console_load_oldman, "");
        register_console_function("load_bunny", console_load_bunny, "");
        register_console_function("compute_average", console_compute_average, "");
        register_console_function("compute_deformation", console_compute_deformation, "");
        register_console_function("save_selected_faces", console_save_selected_faces, "");
        register_console_function("load_selected_faces", console_load_selected_faces, "");
        register_console_function("save_selected_verticies", console_save_selected_verticies, "");
        register_console_function("load_selected_verticies", console_load_selected_verticies, "");
        register_console_function("invert_face_selection", console_invert_face_selection, "");
        register_console_function("set_sel_primitive", console_set_sel_primitive, "");
        register_console_function("set_sel_method", console_set_sel_method, "");
        register_console_function("save_mesh_sequence_with_selected_faces", console_save_mesh_sequence_with_selected_faces, "");
        register_console_function("align_all_models", console_align_all_models, "");
        register_console_function("stop_motion", console_stop_motion, "");
        register_console_function("frame", console_frame, "");
        register_console_function("split_mesh", console_split_mesh, "");
        register_console_function("show_edge_selection", console_show_edge_selection, "");
        register_console_function("deselect_faces", console_deselect_faces, "");
        register_console_function("deselect_verticies", console_deselect_verticies, "");
        register_console_function("show_wireframe", console_show_wireframe, "");
        register_console_function("segmentation", console_segmentation, "");
        register_console_function("clear_selection", console_clear_selection, "");
        register_console_function("clear_vertex_selection", console_clear_vertex_selection, "");
        register_console_function("clear_face_selection", console_clear_face_selection, "");

        register_console_function("save_serialised_data", console_save_serialised_data, "");
        register_console_function("load_serialised_data", console_load_serialised_data, "");
        register_console_function("load_monka", console_load_monka, "");
        
        register_console_function("set_vertex_weight", console_set_vertex_weight, "");
        register_console_function("set_face_weight", console_set_face_weight, "");
        
        register_console_function("draw_vertex_weight_map", console_draw_vertex_weight_map, "");
        register_console_function("draw_face_weight_map", console_draw_face_weight_map, "");
        register_console_function("upsample", console_upsample, "");
        register_console_function("taubin_smooth_along_edges", console_taubin_smooth_along_edges, "");
        register_console_function("laplacian_smooth_along_edges", console_laplacian_smooth_along_edges, "");

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
        FontManager* fManager = FontManager::GetSingleton();
        float scale = 1;
        float ppi = 144;
        fManager->Setup(ppi, scale);

        ES2FontSPtr font = FontManager::GetSingleton()->GetSystemFontOfSize12();
        std::string strrr("Frame 0/9000");
        m_frame_overlay = ES2TextOverlay::Create(strrr, 
                                                 Wm5::Vector2f(0, 0), 
                                                 font, 
                                                 Wm5::Vector4f(0,0,0,1), 
                                                 viewport);
        m_frame_overlay->SetTranslate(Wm5::Vector2i(-viewport[2]/2 + 50,-viewport[3]/2 + 50));
        
        while (!m_tyro_window->ShouldClose())
        {   
            if (m_need_rendering) 
           {
                //RA_LOG_INFO("RENDER BEGIN");

                if (m_state == App::State::Launched) 
                {
                    m_gl_rend->ClearScreen();
                }
                else if (m_state == App::State::LoadedModel) 
                {   
                    //create renderable for mesh
                    VisibleSet vis_set;

                    //ORIGNAL MODEL
                    render_data.org_mesh = IGLMesh::Create(m_frame_data.v_data[m_frame], 
                                                           m_frame_data.f_data, 
                                                           m_frame_data.n_data[m_frame],
                                                           m_frame_data.fc_data);
                    render_data.org_mesh->Update(true);
                    vis_set.Insert(render_data.org_mesh.get());
                    
                    if (m_show_wireframe)
                    {
                        render_data.org_mesh_wire = IGLMeshWireframe::Create(m_frame_data.v_data[m_frame], 
                                                                            m_frame_data.ue_data,
                                                                            m_frame_data.ec_data);
                        render_data.org_mesh_wire->Update(true);
                        vis_set.Insert(render_data.org_mesh_wire.get());
                    }
                    
                    if (m_computed_avg) 
                    {
                        auto mesh = IGLMesh::Create(m_frame_data.avg_v_data, 
                                                    m_frame_data.f_data, 
                                                    m_frame_data.n_data[0],
                                                    Eigen::Vector3d(0.5, 0, 0));
                        Wm5::Transform tr;
                        tr.SetTranslate(Wm5::APoint(-2*m_model_offset, 0, 0));
                        mesh->LocalTransform = tr * mesh->LocalTransform;
                        mesh->Update(true);

                        render_data.avg_mesh = mesh;
                        vis_set.Insert(mesh.get());

                       //create renderable for mesh wireframe
                        if (m_show_wireframe)
                        {
                            auto wire = IGLMeshWireframe::Create(m_frame_data.avg_v_data, 
                                                                 m_frame_data.ue_data,
                                                                 m_frame_data.ec_data);
                            wire->LocalTransform = tr * wire->LocalTransform;
                            wire->Update(true);
                            render_data.avg_mesh_wire = wire;
                            vis_set.Insert(render_data.avg_mesh_wire.get());
                        }
                    
                    }

                    //DEFORMED MODEL
                    if (m_computed_deformation)
                    {                        
                        auto mesh = IGLMesh::Create(m_frame_deformed_data.v_data[m_frame], 
                                                    m_frame_deformed_data.f_data, 
                                                    m_frame_data.n_data[m_frame],
                                                    Eigen::Vector3d(0.5,0.5,0.5));
                        Wm5::Transform tr;
                        tr.SetTranslate(Wm5::APoint(-1*m_model_offset, 0, 0));
                        mesh->LocalTransform = tr * mesh->LocalTransform;
                        mesh->Update(true);
                        render_data.dfm_mesh = mesh;
                        vis_set.Insert(mesh.get());

                        //create renderable for mesh wireframe
                         if (m_show_wireframe)
                        {
                            auto wire = IGLMeshWireframe::Create(m_frame_deformed_data.v_data[m_frame], 
                                                                 m_frame_deformed_data.ue_data,
                                                                 m_frame_data.ec_data);
                            wire->LocalTransform = tr * wire->LocalTransform;
                            wire->Update(true);
                            render_data.dfm_mesh_wire = wire;
                            vis_set.Insert(render_data.dfm_mesh_wire.get());
                        }
                    }

                    //Split Mesh
                    if (m_computed_parts) 
                    {
                        //Rendering stuff
                        render_data.part_meshes.clear();
                        render_data.part_meshes_wire.clear();

                        for (int i = 0; i < m_pieces.size(); ++i) 
                        {                             
                            auto mesh1 = IGLMesh::Create(m_pieces[i].v_data[m_frame], 
                                                         m_pieces[i].f_data, 
                                                         m_pieces[i].n_data[m_frame],
                                                         m_pieces[i].fc_data);
                            Wm5::Transform tr;
                            tr.SetTranslate(Wm5::APoint(m_model_offset, 0, 0));
                            mesh1->LocalTransform = tr * mesh1->LocalTransform;
                            mesh1->Update(true);
                            render_data.part_meshes.push_back(mesh1);
                            vis_set.Insert(mesh1.get());

                            //create renderable for mesh wireframe
                            if (m_show_wireframe)
                            {
                                auto wire1 = IGLMeshWireframe::Create(m_pieces[i].v_data[m_frame], 
                                                                    m_pieces[i].ue_data,
                                                                    m_pieces[i].ec_data);
                                wire1->LocalTransform = tr * wire1->LocalTransform;                                                        
                                wire1->Update(true);
                                render_data.part_meshes_wire.push_back(wire1);
                                vis_set.Insert(wire1.get());
                            }
                        }
                    }

                    //Stop motion
                    if (m_computed_stop_motion) 
                    {   
                        render_data.stop_motion_meshes.clear();
                        render_data.stop_motion_meshes_wire.clear();

                        for (int i = 0; i < m_stop_motion.size(); ++i) 
                        {   
                            auto& sm = m_stop_motion[i];
                            
                            if (sm.computed == false) 
                                continue;

                            auto mesh = IGLMesh::Create(sm.anim.v_data[m_frame], 
                                                        sm.anim.f_data, 
                                                        sm.anim.n_data[m_frame],
                                                        sm.anim.fc_data);
                            Wm5::Transform tr;
                            tr.SetTranslate(Wm5::APoint(2*m_model_offset, 0, 0));
                            mesh->LocalTransform = tr * mesh->LocalTransform;
                            mesh->Update(true);
                            render_data.stop_motion_meshes.push_back(mesh);
                            vis_set.Insert(mesh.get());

                            //create renderable for mesh wireframe
                            if (m_show_wireframe)
                            {
                                auto wire  = IGLMeshWireframe::Create(sm.anim.v_data[m_frame], 
                                                                    sm.anim.ue_data,
                                                                    sm.anim.ec_data);
                                wire->LocalTransform = tr * wire->LocalTransform;                                                        
                                wire->Update(true);
                                render_data.stop_motion_meshes_wire.push_back(wire);
                                vis_set.Insert(wire.get());
                            }
                        }
                    }

                    for (auto object_sptr : ball_list) 
                    {
                        vis_set.Insert(object_sptr.get());
                    }
                    
                    std::string fstr = std::string("Frame ") + std::to_string(m_frame) + std::string("/") + std::to_string(m_frame_data.v_data.size());
                    m_frame_overlay->SetText(fstr);
                    vis_set.Insert(m_frame_overlay.get());

                    if (m_update_camera) 
                    {
                        update_camera();
                        m_update_camera = false;
                    }
                 
                    m_gl_rend->RenderVisibleSet(&vis_set, m_camera);         
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

    void App::render() 
    {   
        //RA_LOG_INFO("NEED RENDERING");
        m_need_rendering = true;
    }

    void App::invert_face_selection() 
    {
        std::vector<int> newlist;
        for (int fid = 0; fid < m_frame_data.f_data.rows(); ++fid) 
        {
            auto it2 = std::find(fid_list.begin(), fid_list.end(), fid);
            if (it2 == fid_list.end()) 
            { 
                newlist.push_back(fid);
            } 
        }

        for (auto fid : fid_list)
        {
            setFaceColor(fid, false);
        }
        fid_list.clear();

        for (auto fid : newlist) 
        {
            fid_list.push_back(fid);
            setFaceColor(fid, true);

        }
        render();
        glfwPostEmptyEvent();
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

        m_computed_avg = true;
        m_update_camera = true;
    }

    void App::update_camera() 
    {
        //setup camera
        AxisAlignedBBox WorldBoundBox = render_data.org_mesh->WorldBoundBox;

        if (m_computed_avg) 
        {
            WorldBoundBox.Merge(render_data.avg_mesh->WorldBoundBox);
        }

        if (m_computed_deformation) 
        {
            WorldBoundBox.Merge(render_data.dfm_mesh->WorldBoundBox);
        }

        if (m_computed_parts) 
        {
            for (auto& mesh : render_data.part_meshes)
                WorldBoundBox.Merge(mesh->WorldBoundBox);
        }

        if (m_computed_stop_motion) 
        {   
            for (auto& mesh : render_data.stop_motion_meshes)
                WorldBoundBox.Merge(mesh->WorldBoundBox);
        }

        Wm5::APoint world_center = WorldBoundBox.GetCenter();
        float radius = std::abs(WorldBoundBox.GetRadius()*2.5);
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        float aspect = (float)v_width/v_height;
        
        if (m_camera)
            delete m_camera;
        
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
    }

    void App::addSphere(int vid, Wm5::Vector4f color) 
    {   
        Eigen::RowVector3d new_c = m_frame_data.v_data[m_frame].row(vid);
        ES2SphereSPtr object = ES2Sphere::Create(10, 10, 0.007);
        Wm5::Transform tr;
        tr.SetTranslate(Wm5::APoint(new_c(0), new_c(1), new_c(2)));
        object->LocalTransform = tr * object->LocalTransform;
        object->Update(true);
        object->SetColor(color);
        ball_list.push_back(object);
    }     

    void App::removeSpheres(std::vector<int> vids) 
    {   
        ball_list.clear();
    }
    
     void App::setFaceColor(int fid, const Eigen::Vector3d& clr) 
    {
        m_frame_data.fc_data.row(fid) = clr;
    }

    void App::setFaceColor(int fid, bool selected) 
    {
        Eigen::Vector3d clr;
        if (selected) 
            clr = Eigen::Vector3d(0, 0, 0.5);
        else
            clr = Eigen::Vector3d(0.5,0.5,0.5);
        
        m_frame_data.fc_data.row(fid) = clr;
    }
   
    void App::load_mesh_sequence(const std::vector<std::string>& obj_list, bool use_igl_loader) 
    {   
        RA_LOG_INFO("LOAD MESH SEQUENCE")
        
        tyro::load_mesh_sequence(obj_list, 
                                 m_frame_data.v_data, 
                                 m_frame_data.n_data, 
                                 m_frame_data.f_data,
                                 m_frame_data.e_data,
                                 m_frame_data.ue_data,
                                 m_frame_data.EMAP,
                                 use_igl_loader);
        
        //@TODO need this to update camera
        Eigen::Vector3d face_color(0.5,0.5,0.5);
        tyro::color_matrix(m_frame_data.f_data.rows(), face_color, m_frame_data.fc_data);
        tyro::color_black_matrix(m_frame_data.ue_data.rows(), m_frame_data.ec_data);
    }

    void App::load_bunny(bool serialized)
    {
        RA_LOG_INFO("load bunny serialized %i",serialized);
        int offset_vid = 1030; // 1222;
        auto offset = Eigen::Vector3d(0.268563, 3.142050, 2.504273) ; //Eigen::Vector3d(0.613322, 2.613381, 2.238946);
       
        FOLDERS = 
        {   
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj_export/just_face")
                    
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/02/01/"),
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/02/02/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/02/03/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/02/04/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/02/05/"),
            
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/03/01/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/03/02/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/03/03/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/03/05/"),
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/03/06/"),
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/04/01/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/04/02/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/04/03/"),

            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/05/01/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/05/03/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/05/05/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/05/07/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/05/09/"),
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/05/11/"),
            
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/07/01/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/07/02/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/07/05/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/07/06/"),


            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/01/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/02B/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/03/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/05/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/07/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/07B/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/09/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/10/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/11/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/13/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/14/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/16/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/18/"),
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/08/19/"),
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/09/05/"),

            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/11/10/"),

            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/12/09/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/12/21/"),

            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/13/03/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/13/04/")
            
        };

        if (serialized) 
        {   
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path("bunny_frames_upsampled");
            std::ifstream in = std::ifstream(p.str(), std::ios::binary);
            cereal::BinaryInputArchive archive_i(in);
            archive_i(m_frame_data);
        }
        else
        {   
            std::vector<std::string> obj_paths;
            for (auto folder : FOLDERS) 
            {
                int num_files_read;

                //Add smth
                //folder += std::string("no_mouth/");

                RA_LOG_INFO("loading folder %s", folder.data());
                tyro::obj_file_path_list(folder, "objlist2.txt", obj_paths, num_files_read);
                RA_LOG_INFO("frames read %i", num_files_read);
                m_frame_data.sequenceIdx.push_back(num_files_read);
            }
            load_mesh_sequence(obj_paths, true); //use IGL obj loader
        }
        m_timeline->SetFrameRange(m_frame_data.v_data.size()-1);

        //align_all_models(offset_vid, offset);
               
        //Compute radius of the bounding box of the model
        AxisAlignedBBox bbox;
        MatrixXd VT = m_frame_data.v_data[0].transpose();
        bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
        m_model_offset = bbox.GetRadius(); 

        m_update_camera = true;
        m_state = App::State::LoadedModel;
        
        m_weights.VW.resize(m_frame_data.v_data[0].rows());
        m_weights.VW.setOnes();

        m_weights.FW.resize(m_frame_data.f_data.rows());
        m_weights.FW.setOnes();

        compute_average();
        render();
    }

    //load mexican blobby guy
    void App::load_blobby() 
    {   
        RA_LOG_INFO("load blobby");
        auto obj_list_file = std::string("/home/rinat/GDrive/StopMotionProject/Claymation/data/hello/FramesOBJ/objlist2.txt");
        //load_mesh_sequence(obj_list_file);
        m_update_camera = true;
        
        m_state = App::State::LoadedModel;
        render();
    }

    //load oldman (will you go to lunch)
    void App::load_oldman() 
    {   
        RA_LOG_INFO("load oldman");
        auto obj_list_file = std::string("/home/rinat/GDrive/StopMotionProject/Claymation/data/oldman/gotolunch/FramesOBJ/FullFace/objlist2.txt");
        //load_mesh_sequence(obj_list_file);
        m_update_camera = true;

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
            //printf("%i ", fid);
            
            auto it = std::find(fid_list.begin(), fid_list.end(), fid);
            if (it == fid_list.end()) 
            {
                fid_list.push_back(fid);
                setFaceColor(fid, true);
            }              
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
            //printf("%i ", vid);
            vid_list.push_back(vid);
            addSphere(vid);
        }
        render();
        glfwPostEmptyEvent();        
        //getchar();
    }
    
    void App::set_sel_primitive(App::SelectionPrimitive sel_state) 
    {
        m_sel_primitive = sel_state;
    }

    void App::set_sel_method(App::SelectionMethod sel_state) 
    {
        m_sel_method = sel_state;
    }

    void App::align_all_models(int vid, Eigen::Vector3d ref_vec)
    {
        for (int frame = 0; frame < m_frame_data.v_data.size(); ++frame) 
        {
            Eigen::Vector3d new_vec = m_frame_data.v_data[frame].row(vid);
            Eigen::Vector3d d = ref_vec - new_vec;
            RowVector3d diff(d(0), d(1), d(2)); 
            m_frame_data.v_data[frame].rowwise() += diff;
        }
        render();
    }
        

    void App::align_all_models() 
    {
        //last selected vid 
        if (vid_list.size()==0) return;
        int vid = vid_list.back();
        
        Eigen::Vector3d ref_vec;
        for (int frame = 0; frame < m_frame_data.v_data.size(); ++frame) 
        {
            if (frame ==0 ) 
            {
                ref_vec = m_frame_data.v_data[frame].row(vid);
            }
            else 
            {
                Eigen::Vector3d new_vec = m_frame_data.v_data[frame].row(vid);
                Eigen::Vector3d d = ref_vec - new_vec;
                Eigen::RowVector3d diff(d(0), d(1), d(2)); 
                m_frame_data.v_data[frame].rowwise() += diff;
            }
        }

        render();
        glfwPostEmptyEvent();
    }

    void App::selectVertex(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier) 
    {
        int fid;
        Eigen::Vector3f bc;
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
            if (m_sel_primitive == App::SelectionPrimitive::Vertex) 
            {
                long c;
                bc.maxCoeff(&c);
                int vid = m_frame_data.f_data(fid, c);
                auto it = std::find(vid_list.begin(), vid_list.end(), vid);
                if (it == vid_list.end()) 
                {       
                Eigen::Vector3d vec = m_frame_data.v_data[m_frame].row(vid);
                    RA_LOG_INFO("Picked face_id %i vertex_id %i coord %f %f %f", fid, vid, vec(0), vec(1), vec(2));
                    addSphere(vid);
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
            else if (m_sel_primitive == App::SelectionPrimitive::Faces) 
            {   
                if (mouse_button == 0) 
                {
                    auto it = std::find(fid_list.begin(), fid_list.end(), fid);
                    if (it == fid_list.end()) 
                    {
                        fid_list.push_back(fid);
                        setFaceColor(fid, true); 
                    }  
                    else
                    {   
                        auto index = std::distance(fid_list.begin(), it);
                        fid_list.erase(fid_list.begin() + index);
                        setFaceColor(fid, false);
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
                        setFaceColor(fid, Eigen::Vector3d(147/255.0, 112/255.0, 219/255.0)); 
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
        }   
    }
    
    
    void App::debug_show_faces_near_edge_selection(const Eigen::VectorXi& uEI, const Eigen::VectorXi& DMAP) 
    {   
        const auto & cE = m_frame_data.ue_data;
        const auto & cEMAP = m_frame_data.EMAP;
        MatrixXi EF, EI;

        igl::edge_flaps(m_frame_data.f_data, 
                        cE, 
                        cEMAP, 
                        EF, 
                        EI);
        
        for (int i = 0; i < uEI.size(); ++i) 
        {               
            int f1 = EF(uEI(i), 0);
            int f2 = EF(uEI(i), 1);

            if (DMAP(i) == 0)
                setFaceColor(f1, true);
            else 
                setFaceColor(f2, true);
        }        
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
            m_square_sel_start_x = current_mouse_x;
            m_square_sel_start_y = current_mouse_y;
            return; //selection
        }
        if (m_mouse_btn_clicked == 2) return; //translating
        
        // Cast a ray in the view direction starting from the mouse position
        double x = current_mouse_x;
        double y = m_camera->GetViewport()[3] - current_mouse_y;
        Eigen::Vector2f mouse_pos(x,y);
        selectVertex(mouse_pos, button, modifier);     
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
        else if (mouse_is_down && m_modifier == TYRO_MOD_SHIFT && m_sel_method == App::SelectionMethod::Square) 
        {
            gesture_state = 2;
            //double x = current_mouse_x;
            //double y = m_camera->GetViewport()[3] - current_mouse_y;
            //Eigen::Vector2f mouse_pos(x,y);
            
            int x0 = m_square_sel_start_x;
            int y0 = m_camera->GetViewport()[3] - m_square_sel_start_y;
            int x1 = current_mouse_x;
            int y1 = m_camera->GetViewport()[3] - current_mouse_y;

            Wm5::HMatrix modelViewMatrix = m_camera->GetViewMatrix() * render_data.org_mesh->WorldTransform.Matrix();
            Wm5::HMatrix projectMatrix = m_camera->GetProjectionMatrix();
            Eigen::Matrix4f e1 = Eigen::Map<Eigen::Matrix4f>(modelViewMatrix.mEntry);
            Eigen::Matrix4f e2 = Eigen::Map<Eigen::Matrix4f>(projectMatrix.mEntry);
            Eigen::Vector4f e3 = Eigen::Vector4f(m_camera->GetViewport()[0],
                                                m_camera->GetViewport()[1],
                                                m_camera->GetViewport()[2],
                                                m_camera->GetViewport()[3]);


            MatrixXd P;
            igl::project(m_frame_data.v_data[m_frame],
                         e1.transpose(), 
                         e2.transpose(),
                         e3,
                         P);

            
            std::vector<int> vid_selected;
            for (int vid = 0; vid < P.rows(); ++vid) 
            {
                int px = P.row(vid)(0);
                int py = P.row(vid)(1);

                int minx = std::min(x0, x1);
                int maxx = std::max(x0, x1);
                int miny = std::min(y0, y1);
                int maxy = std::max(y0, y1);
                if (px <=maxx && px >= minx && py <= maxy && py >= miny) 
                {
                    vid_selected.push_back(vid);
                }
            }
            if (m_sel_primitive == App::SelectionPrimitive::Vertex) 
            {   
                for (auto vid : vid_selected) 
                {
                    addSphere(vid);
                    vid_list.push_back(vid);
                }
            }
            else if (m_sel_primitive == App::SelectionPrimitive::Faces) 
            {   
                //std::vector<int> fid_selected;
                for (int fid = 0; fid < m_frame_data.f_data.rows(); ++fid) 
                {
                    Eigen::Vector3i vids = m_frame_data.f_data.row(fid);

                    auto it = std::find(vid_selected.begin(), vid_selected.end(), vids(0));
                    if (it != vid_selected.end())  
                    {
                        fid_list.push_back(fid); 
                        continue;
                    } 
                    
                    it = std::find(vid_selected.begin(), vid_selected.end(), vids(1));
                    if (it != vid_selected.end()) 
                    { 
                        fid_list.push_back(fid);
                        continue;
                    }

                    it = std::find(vid_selected.begin(), vid_selected.end(), vids(2));
                    if (it != vid_selected.end()) 
                    {
                        fid_list.push_back(fid); 
                        continue;
                    }
                }

                for (auto fid : fid_list) 
                {
                    setFaceColor(fid, true);
                }
            }

            
        
            //m_camera->HandleTwoFingerPanGesture(gesture_state, Vector2i(current_mouse_x, -current_mouse_y));
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

        if (mouse_is_down && m_modifier == TYRO_MOD_SHIFT && m_sel_method == App::SelectionMethod::OneClick) 
        {
            // Cast a ray in the view direction starting from the mouse position
            double x = current_mouse_x;
            double y = m_camera->GetViewport()[3] - current_mouse_y;
            Eigen::Vector2f mouse_pos(x,y);
            selectVertex(mouse_pos, m_mouse_btn_clicked, m_modifier);
        }
        else if (mouse_is_down && m_modifier == TYRO_MOD_SHIFT && m_sel_method == App::SelectionMethod::Square) 
        {
            //update
        }
        else if (mouse_is_down && m_modifier == TYRO_MOD_CONTROL) 
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