#include "TyroApp.h"
#include <igl/facet_components.h>
#include "RAEnginePrerequisites.h"
#include "RAES2TextOverlay.h"
#include "RAFont.h"
#include "RAVisibleSet.h"
#include "RAES2StandardMesh.h"
#include "RAAxisAlignedBBox.h"
#include "ES2VideoTexture.h"
#include <stdio.h>

#include <functional>
#include <igl/jet.h>
#include <igl/avg_edge_length.h>
#include "Wm5APoint.h"
#include "Wm5Vector2.h"
#include "Wm5Vector4.h"
#include <igl/is_boundary_edge.h>
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
#include <igl/ambient_occlusion.h>
#include <igl/boundary_loop.h>
#include <igl/doublearea.h>
#include <filesystem/path.h>
#include <igl/per_vertex_attribute_smoothing.h>
#include <igl/edges_to_path.h>
#include <igl/on_boundary.h>
#include <igl/remove_duplicates.h>
#include <igl/collapse_small_triangles.h>

#include "TyroIGLMesh.h"
#include "TyroFileUtils.h"

#include "load_mesh_sequence.h"
#include "compute_deformation.h"
#include "stop_motion.h"
#include "mesh_split.h"
#include "segmentation.h"
#include "slice_isoline.h"
#include "data_smooth.h"
#include <random>

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
        dest.F = source.F;
        dest.E = source.E;
        dest.UE = source.UE;
        dest.EMAP = source.EMAP;
    }

    if (face_color)
        dest.FC = source.FC;
    
    if (edge_color)
        dest.UEC = source.UEC;
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
                                            const std::vector<int>& vid_second,
                                            const std::vector<int>& vid_third,
                                            const Eigen::MatrixXi& E, //all directed edges
                                            const Eigen::MatrixXi& uE, //all unique edges
                                            const Eigen::VectorXi& EMAP, // map from directed to unique edge index 
                                            bool isClosed,
                                            Eigen::MatrixXi& eid_list, // edges from vid_list
                                            Eigen::VectorXi& EI, // indicies into directed edges matrix
                                            Eigen::VectorXi& uEI, // indicies into undirected edges matrix
                                            Eigen::VectorXi& DMAP) // checks which directions where switched HACKY
{
    //create a closed edge loop from vertex selection
    int num_edges;
    if (isClosed) 
    {
        num_edges = vid_list.size() + vid_second.size()+ vid_third.size(); //assumes its a closed loop
        eid_list.resize(num_edges, 2);
        for (int i = 0; i < vid_list.size(); ++i) 
        {   
            if (i == vid_list.size() - 1) //last edge
                eid_list.row(i) = Eigen::Vector2i(vid_list[i], vid_list[0]);
            else
                eid_list.row(i) = Eigen::Vector2i(vid_list[i], vid_list[i+1]);
        }
        
        for (int i = 0; i < vid_second.size(); ++i) 
        {   
            if (i == vid_second.size() - 1) //last edge
                eid_list.row(i+vid_list.size()) = Eigen::Vector2i(vid_second[i], vid_second[0]);
            else
                eid_list.row(i+vid_list.size()) = Eigen::Vector2i(vid_second[i], vid_second[i+1]);
        }

        for (int i = 0; i < vid_third.size(); ++i) 
        {   
            if (i == vid_third.size() - 1) //last edge
                eid_list.row(i+vid_list.size() + vid_second.size()) = Eigen::Vector2i(vid_third[i], vid_third[0]);
            else
                eid_list.row(i+vid_list.size()+ vid_second.size()) = Eigen::Vector2i(vid_third[i], vid_third[i+1]);
        }
    }
    else 
    {   num_edges = vid_list.size() - 1;
        if (vid_second.size()>0)
            num_edges += vid_second.size() - 1; //assumes its a closed loop
        
        if (vid_third.size()>0)
            num_edges += vid_third.size() - 1; //assumes its a closed loop
        
        eid_list.resize(num_edges, 2);
        
        for (int i = 0; i < vid_list.size()-1; ++i) 
            eid_list.row(i) = Eigen::Vector2i(vid_list[i], vid_list[i+1]);

        if (vid_second.size() > 0) 
            for (int i = 0; i < vid_second.size() - 1; ++i) 
                eid_list.row(i + vid_list.size() - 1) = Eigen::Vector2i(vid_second[i], vid_second[i+1]);

        if (vid_third.size() > 0) 
            for (int i = 0; i < vid_third.size() - 1; ++i) 
                eid_list.row(i + vid_list.size() - 1 + vid_second.size() - 1) = Eigen::Vector2i(vid_third[i], vid_third[i+1]);
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
        void scale_one_exe(App::MAnimation& ANIM, const double& scale, const Eigen::RowVector3d& tr) 
        {   
         
            if (ANIM.VD.empty()) 
                return;
            for (int i = 0; i < ANIM.VD.size(); ++i) 
            {
                ANIM.VD[i] = scale * (ANIM.VD[i].rowwise() + tr);
            }
        }  

        void scale_one(App::MAnimation& ANIM, double& scale, Eigen::RowVector3d& tr) 
        {               
            if (ANIM.VD.empty()) 
                return;
            
            AxisAlignedBBox bbox;
            MatrixXd VT = ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            scale = 1/bbox.GetRadius(); 
            Wm5::Vector3f center = bbox.GetCenter();
            Wm5::Vector3f tocenter = Wm5::Vector3f::ZERO - center;

            tr = Eigen::RowVector3d(tocenter[0], tocenter[1], tocenter[2]);
            
            for (int i = 0; i < ANIM.VD.size(); ++i) 
            {                
                ANIM.VD[i] = scale * (ANIM.VD[i].rowwise() + tr);
            }
        }  

        void console_load_serialised_data(App* app, const std::vector<std::string>& args); 
        
        void console_collapse_small_triangles(App* app, const std::vector<std::string>&args) 
        {   
            Eigen::MatrixXi FF;
            if (app->isBLOBBY)
                igl::collapse_small_triangles(app->ANIM.AvgVD,app->ANIM.F, 0.0000001, FF);
            else
                igl::collapse_small_triangles(app->ANIM.AvgVD,app->ANIM.F, 0.0000001, FF);
            
            RA_LOG_INFO("After %i Before %i collapse", FF.rows(), app->ANIM.F.rows());
            
            MatrixXi I1; 
            for (int i=0;i<app->ANIM.VD.size();++i) 
            {   
                MatrixXd newVD;                
                igl::remove_unreferenced(app->ANIM.VD[i], 
                                         FF, 
                                         newVD, 
                                         app->ANIM.F, 
                                         I1);

                app->ANIM.VD[i] =  newVD;
                igl::per_vertex_normals(app->ANIM.VD[i], app->ANIM.F, app->ANIM.ND[i]);
            }
            
            std::vector<std::vector<int>> uE2E;
            igl::unique_edge_map(app->ANIM.F, app->ANIM.E, app->ANIM.UE, app->ANIM.EMAP, uE2E);

            tyro::color_matrix(app->ANIM.F.rows(), Eigen::Vector3d(0.5,0.5,0.5), app->ANIM.FC);
            tyro::color_matrix(app->ANIM.UE.rows(), Eigen::Vector3d(0.2,0.2,0.2), app->ANIM.UEC);
            
            app->ANIM.VW.resize(app->ANIM.VD[0].rows());
            app->ANIM.VW.setOnes();

            app->compute_average();

            app->RENDER.mesh = nullptr;
            app->RENDER.mesh_wire = nullptr;
            
            app->RENDER.avg = nullptr;
            app->RENDER.avg_wire = nullptr;
        }

        void console_visualize_seams(App* app, const std::vector<std::string>& args) 
        {   
            std::vector<std::vector<int>> L;
            igl::boundary_loop(app->ANIM.F,L);
        }

        void console_scale(App* app, const std::vector<std::string>& args) 
        {
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.AvgVD.transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            double scale = 2/bbox.GetRadius(); 
            RA_LOG_INFO("Scale %f", scale);
            for (int i = 0; i < app->ANIM.VD.size(); ++i) 
            {
                app->ANIM.VD[i] = scale * app->ANIM.VD[i];
            }
            app->compute_average();
            app->RENDER.mesh = nullptr;
            app->RENDER.mesh_wire = nullptr;
            app->m_update_camera = true;
            app->render();
        }

        void console_show_labels(App* app, const std::vector<std::string>& args) 
        {   
            if (args.size()!=1)  
            {
                RA_LOG_WARN("Need to specify type of the curve");
                return; 
            }
            MatrixXi eid_list;
            VectorXi EI, uEI, DMAP;
            bool isClosedSeam = std::stoi(args[0]);
            convert_vertex_to_edge_selection(app->vid_list,
            app->vid_list_second, app->vid_list_third,
                                             app->ANIM.E, 
                                             app->ANIM.UE, 
                                             app->ANIM.EMAP,
                                             isClosedSeam,
                                             eid_list, 
                                             EI, 
                                             uEI, 
                                             DMAP);

            VectorXi F1idx, F2idx;
            tyro::mesh_split(app->ANIM.F,
                             uEI,
                             DMAP, 
                             F1idx, 
                             F2idx);
            
            for (int i=0; i < F1idx.size(); ++i) 
            {
                app->setFaceColor(F1idx(i), Eigen::Vector3d(0.9,0.5,0.5));
            }
            app->render();
        }

        // plot colors of sum of differences between each frame and average
        void console_show_average_displacement(App* app, Eigen::SparseMatrix<double>& BC, Eigen::MatrixXi& newF) 
        {
            for (int i = 0; i < app->ANIM.VD.size(); ++i) 
            {
                
            }
        }

        void propogate_changes_to_frames(App* app, Eigen::SparseMatrix<double>& BC, Eigen::MatrixXi& newF) 
        {   
            //App::MAnimation newAnim; 
            //newAnim.VD.resize(app->ANIM.VD.size());
            
            app->ANIM.F = newF;
            for (int i=0;i<app->ANIM.VD.size();++i) 
            {   
                app->ANIM.VD[i] =  BC * app->ANIM.VD[i];
                igl::per_vertex_normals(app->ANIM.VD[i], app->ANIM.F, app->ANIM.ND[i]);
            }
            
            std::vector<std::vector<int>> uE2E;
            igl::unique_edge_map(app->ANIM.F, app->ANIM.E, app->ANIM.UE, app->ANIM.EMAP, uE2E);

            tyro::color_matrix(app->ANIM.F.rows(), Eigen::Vector3d(0.5,0.5,0.5), app->ANIM.FC);
            tyro::color_matrix(app->ANIM.UE.rows(), Eigen::Vector3d(0.2,0.2,0.2), app->ANIM.UEC);
            
            app->ANIM.VW.resize(app->ANIM.VD[0].rows());
            app->ANIM.VW.setOnes();

            app->compute_average();

            app->RENDER.mesh = nullptr;
            app->RENDER.mesh_wire = nullptr;
            
            app->RENDER.avg = nullptr;
            app->RENDER.avg_wire = nullptr;
            
        }

        void console_print_stats(App* app, const std::vector<std::string>& args) 
        {
            RA_LOG_INFO("ANIM F %i V %i", app->ANIM.VD[0].rows(), app->ANIM.F.rows());
            RA_LOG_INFO("PART 0 F %i V %i", app->PIECES[0].VD[0].rows(), app->PIECES[0].F.rows());
            RA_LOG_INFO("PART 1 F %i V %i", app->PIECES[1].VD[0].rows(), app->PIECES[1].F.rows());
            
        }

        void console_mesh_split_figure(App* app, const std::vector<std::string>& args) 
        {
            
            std::vector<int> F1idx_vec, F2idx_vec;
            for (int i=0;i<app->ANIM.F.rows(); ++i) 
            {   
                auto it = std::find(app->fid_list.begin(), app->fid_list.end(), i);
                if (it == app->fid_list.end()) 
                    F1idx_vec.push_back(i);
                else 
                    F2idx_vec.push_back(i);
            }
            
            VectorXi F1idx = Eigen::Map<VectorXi>(F1idx_vec.data(), F1idx_vec.size());
            VectorXi F2idx = Eigen::Map<VectorXi>(F2idx_vec.data(), F2idx_vec.size());
            if (app->PIECES_IDX.size() == 0)
                app->PIECES_IDX.resize(2);

            app->PIECES_IDX[0] = F1idx;
            app->PIECES_IDX[1] = F2idx;
            MatrixXi F1, F2;
            igl::slice(app->ANIM.F, F1idx, 1, F1); 
            igl::slice(app->ANIM.F, F2idx, 1, F2); 
            
            app->PIECES.resize(2);
            auto& A1 = app->PIECES[0];
            auto& A2 = app->PIECES[1];
            A1.VD.resize(app->ANIM.VD.size());
            A2.VD.resize(app->ANIM.VD.size());
            A1.ND.resize(app->ANIM.VD.size());
            A2.ND.resize(app->ANIM.VD.size());
            
            A1.SIdx = app->ANIM.SIdx;
            A2.SIdx = app->ANIM.SIdx;
            
            for (int i = 0; i < app->ANIM.VD.size(); ++i) 
            {                   
                MatrixXi I1, I2;    
                igl::remove_unreferenced(app->ANIM.VD[i], 
                                         F1, 
                                         A1.VD[i], 
                                         A1.F, 
                                         I1);

                igl::per_vertex_normals(A1.VD[i], A1.F, A1.ND[i]);
                std::vector<std::vector<int> > uE2E1;
                igl::unique_edge_map(A1.F,A1.E,A1.UE,A1.EMAP,uE2E1);
                tyro::color_matrix(A1.F.rows(), Eigen::Vector3d(0.2,0.2,0.2), A1.FC);
                tyro::color_black_matrix(A1.UE.rows(), A1.UEC);

                igl::remove_unreferenced(app->ANIM.VD[i], 
                                         F2, 
                                         A2.VD[i], 
                                         A2.F, 
                                         I2);
                
                igl::per_vertex_normals(A2.VD[i], A2.F, A2.ND[i]);
                std::vector<std::vector<int> > uE2E2;
                igl::unique_edge_map(A2.F, A2.E, A2.UE, A2.EMAP,uE2E2);
                tyro::color_matrix(A2.F.rows(), Eigen::Vector3d(0.6,0.6,0.6), A2.FC);
                tyro::color_black_matrix(A2.E.rows(), A2.UEC);
            }
                
            app->m_computed_parts = true;            
            app->m_update_camera = true;
        }

        void console_save_deform_average(App* app, const std::vector<std::string>& args)
        {   
            auto& DANIM = app->DANIM;

            if (DANIM.VD.size()==0)
                RA_LOG_ERROR_ASSERT("cant compute average");
            
            DANIM.AvgVD.resize(DANIM.VD[0].rows(),DANIM.VD[0].cols());
            DANIM.AvgVD.setZero();

            for (auto& mat : DANIM.VD) 
            {
                DANIM.AvgVD += mat;
            }
            DANIM.AvgVD = (1.0/DANIM.VD.size()) * DANIM.AvgVD;
            auto file = std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/print/avg.obj");
            igl::writeOBJ(file, DANIM.AvgVD, DANIM.F);
        }


        void console_figure_random(App* app, const std::vector<std::string>& args) 
        {
            using namespace Eigen;
            MatrixXd up1 = app->PIECES[1].VD[773].eval();
            MatrixXd up2 = app->PIECES[1].VD[1286].eval();

            app->PIECES[1].VD[773] = up2;
            app->PIECES[1].VD[1286] = up1;
        }

        void console_mesh_split_final(App* app, const std::vector<std::string>& args) 
        {   
            //console_mesh_split_figure(app, args);
            //return;

            std::vector<int> F1idx_vec, F2idx_vec;
            for (int i=0;i<app->ANIM.F.rows(); ++i) 
            {   
                auto it = std::find(app->fid_list.begin(), app->fid_list.end(), i);
                if (it == app->fid_list.end()) 
                    F1idx_vec.push_back(i);
                else 
                    F2idx_vec.push_back(i);
            }
            
            VectorXi F1idx = Eigen::Map<VectorXi>(F1idx_vec.data(), F1idx_vec.size());
            VectorXi F2idx = Eigen::Map<VectorXi>(F2idx_vec.data(), F2idx_vec.size());
            if (app->PIECES_IDX.size() == 0)
                app->PIECES_IDX.resize(2);

            app->PIECES_IDX[0] = F1idx;
            app->PIECES_IDX[1] = F2idx;
            MatrixXi F1, F2;
            igl::slice(app->DANIM.F, F1idx, 1, F1); 
            igl::slice(app->DANIM.F, F2idx, 1, F2); 
            
            app->PIECES.resize(2);
            auto& A1 = app->PIECES[0];
            auto& A2 = app->PIECES[1];
            A1.VD.resize(app->DANIM.VD.size());
            A2.VD.resize(app->DANIM.VD.size());
            A1.ND.resize(app->DANIM.VD.size());
            A2.ND.resize(app->DANIM.VD.size());
            
            A1.SIdx = app->ANIM.SIdx;
            A2.SIdx = app->ANIM.SIdx;
            
            for (int i = 0; i < app->ANIM.VD.size(); ++i) 
            {                   
                MatrixXi I1, I2;    
                igl::remove_unreferenced(app->DANIM.VD[i], 
                                         F1, 
                                         A1.VD[i], 
                                         A1.F, 
                                         I1);

                igl::per_vertex_normals(A1.VD[i], A1.F, A1.ND[i]);
                std::vector<std::vector<int> > uE2E1;
                igl::unique_edge_map(A1.F,A1.E,A1.UE,A1.EMAP,uE2E1);
                tyro::color_matrix(A1.F.rows(), Eigen::Vector3d(0.2,0.2,0.2), A1.FC);
                tyro::color_black_matrix(A1.UE.rows(), A1.UEC);

                igl::remove_unreferenced(app->DANIM.VD[i], 
                                         F2, 
                                         A2.VD[i], 
                                         A2.F, 
                                         I2);
                
                igl::per_vertex_normals(A2.VD[i], A2.F, A2.ND[i]);
                std::vector<std::vector<int> > uE2E2;
                igl::unique_edge_map(A2.F, A2.E, A2.UE, A2.EMAP,uE2E2);
                tyro::color_matrix(A2.F.rows(), Eigen::Vector3d(0.6,0.6,0.6), A2.FC);
                tyro::color_black_matrix(A2.E.rows(), A2.UEC);
            }
                
            app->m_computed_parts = true;            
            app->m_update_camera = true;
        }

        void console_mesh_split_bndr(App* app, const std::vector<std::string>& args) 
        {
            assert(app->LABELS.size()>0);

            std::vector<int> F1idx_vec, F2idx_vec;
            for (int i=0;i<app->LABELS.size(); ++i) 
            {
                if (app->LABELS(i)== 1) 
                    F1idx_vec.push_back(i);
                else 
                    F2idx_vec.push_back(i);
            }

            
            VectorXi F1idx = Eigen::Map<VectorXi>(F1idx_vec.data(), F1idx_vec.size());
            VectorXi F2idx = Eigen::Map<VectorXi>(F2idx_vec.data(), F2idx_vec.size());
            if (app->PIECES_IDX.size() == 0)
                 app->PIECES_IDX.resize(2);

            app->PIECES_IDX[0] = F1idx;
            app->PIECES_IDX[1] = F2idx;
            MatrixXi F1, F2;
            igl::slice(app->DANIM.F, F1idx, 1, F1); 
            igl::slice(app->DANIM.F, F2idx, 1, F2); 
            
            app->PIECES.resize(2);
            auto& A1 = app->PIECES[0];
            auto& A2 = app->PIECES[1];
            A1.VD.resize(app->DANIM.VD.size());
            A2.VD.resize(app->DANIM.VD.size());
            A1.ND.resize(app->DANIM.VD.size());
            A2.ND.resize(app->DANIM.VD.size());
            
            A1.SIdx = app->ANIM.SIdx;
            A2.SIdx = app->ANIM.SIdx;
            
            for (int i = 0; i < app->ANIM.VD.size(); ++i) 
            {                   
                MatrixXi I1, I2;    
                igl::remove_unreferenced(app->DANIM.VD[i], 
                                         F1, 
                                         A1.VD[i], 
                                         A1.F, 
                                         I1);

                igl::per_vertex_normals(A1.VD[i], A1.F, A1.ND[i]);
                std::vector<std::vector<int> > uE2E1;
                igl::unique_edge_map(A1.F,A1.E,A1.UE,A1.EMAP,uE2E1);
                tyro::color_matrix(A1.F.rows(), Eigen::Vector3d(0.2,0.2,0.2), A1.FC);
                tyro::color_black_matrix(A1.UE.rows(), A1.UEC);

                igl::remove_unreferenced(app->DANIM.VD[i], 
                                         F2, 
                                         A2.VD[i], 
                                         A2.F, 
                                         I2);
                
                igl::per_vertex_normals(A2.VD[i], A2.F, A2.ND[i]);
                std::vector<std::vector<int> > uE2E2;
                igl::unique_edge_map(A2.F, A2.E, A2.UE, A2.EMAP,uE2E2);
                tyro::color_matrix(A2.F.rows(), Eigen::Vector3d(0.6,0.6,0.6), A2.FC);
                tyro::color_black_matrix(A2.E.rows(), A2.UEC);
            }
                
            app->m_computed_parts = true;            
            app->m_update_camera = true;
        }   
        
         void console_compute_data_vel_error(App* app, const std::vector<std::string>& args)
        {      
            using namespace Eigen;

            if (args.size() != 1) return;

            int p = std::stoi(args[0]);
            int num_frames = app->ANIM.VD.size();
            int num_vert = app->PIECES[p].VD[0].rows();
            //Velocity
            MatrixXd m_error_velocity;
            m_error_velocity.resize(num_frames, num_vert); 
                
            auto& sm = app->SMOTION[p]; 
        
            
            VectorXd zero;
            zero.resize(app->ANIM.VD[0].rows());
            zero.setZero();

            m_error_velocity.row(0) = zero;
            
            
            for (int frame = 1; frame < num_frames; ++frame) 
            {                  
                VectorXd lul = ((app->PIECES[p].VD[frame] - app->PIECES[p].VD[frame-1]) - 
                                (app->SMOTION[p].anim.VD[frame] - app->SMOTION[p].anim.VD[frame-1])).rowwise().squaredNorm();
                                int s = lul.size();  
                m_error_velocity.row(frame) = lul;                            
            }
        

            MatrixXd m_error;
            m_error.resize(num_frames, num_vert); 
            {
                using namespace Eigen;
                int num_parts = app->SMOTION.size();
                
                
                for (int frame=0; frame < num_frames; ++frame) 
                {                   
                    VectorXd lul = (app->PIECES[p].VD[frame] - app->SMOTION[p].anim.VD[frame]).rowwise().squaredNorm();
                    m_error.row(frame) = lul;                             
                
                }
            }

            VectorXd total_data_error = m_error.rowwise().sum();
            VectorXd total_vel_error = m_error_velocity.rowwise().sum();

            
            RA_LOG_INFO("save selected faces");
            auto path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/TOTAL_ERROR");

            std::ofstream outFile(path);
            for (int i=0; i <num_frames; ++i) 
            { 
                outFile << total_data_error(i) << "," << total_vel_error(i) << "\n";
            }
        }

        void console_compute_stop_frames(App* app, const std::vector<std::string>& args) 
        {
            app->PRINTEDStopAnimMovie();
        }

        void console_compute_alec(App* app, const std::vector<std::string>& args)
        {   
            //std::vector<int> steps = {2000, 2500, 3000}; //;, 3500, 4000, 4500, 5000,5500, 6000, 7500, 8000, 9500, 9997};
            std::vector<int> steps = {3500, 4000, 4500, 5000,5500, 6000, 7500, 8000, 9500, 9997};
            
            //std::vector<int> steps = {2000};
            //int part_id = 1;
            double min_nrg = 200;
            int start_labels = 185;
            int max_num_labels = 1000;
            int step = 5;
            
            for (const auto& s : steps) 
            {
                for (int num_labels = start_labels; num_labels < max_num_labels; num_labels += step) 
                {
                    double smooth_weight = 1.0;
                    double result_energy;
                    App::MStopMotion sm;  
                    auto& piece = app->ANIM; //app->PIECES[part_id];
                    bool kmeans = true;

                    auto first = piece.VD.begin() + 0;
                    auto last = piece.VD.begin() + s;
                    std::vector<Eigen::MatrixXd> NEW_VD(first, last);

                    if (piece.VW.size() == 0) 
                    {
                        piece.VW.resize(NEW_VD[0].rows());
                        piece.VW.setOnes();
                        RA_LOG_INFO("----WEIGHTS ARE EMPTY, SETTING TO ONE");
                    }

                    std::vector<int> sidx = {s};
                    Eigen::VectorXd VW = piece.VW.cwiseSqrt();
                    if (tyro::stop_motion_vertex_distance(num_labels, 
                                                            smooth_weight,
                                                            kmeans,
                                                            NEW_VD, 
                                                            VW, 
                                                            sidx,
                                                            piece.F,
                                                            sm.D, //dictionary
                                                            sm.L, //labels  
                                                            result_energy))
                    {

                        if (result_energy < min_nrg)  
                        {
                            RA_LOG_INFO("-----num_labels %i for %i frames to reach %i energy", num_labels, s, (int)min_nrg);
                            start_labels = num_labels;
                            break;
                        }
                    }
                }     
            }
        }

        void console_compute_errors(App* app, const std::vector<std::string>& args) 
        {      

            if (args.size() != 1) return;
            
            int part_id =  std::stoi(args[0]);
            std::vector<double> errors;
            for (int num_labels = 10; num_labels < 600; num_labels += 20) 
            {
                double smooth_weight = 0.0;

                
                double result_energy;
                App::MStopMotion sm;  
                auto& piece = app->PIECES[part_id];
                bool kmeans = true;

                if (piece.VW.size() == 0) 
                {
                    piece.VW.resize(app->PIECES[part_id].VD[0].rows());
                    piece.VW.setOnes();
                    RA_LOG_INFO("WEIGHTS ARE EMPTY, SETTING TO ZERO");
                }


                Eigen::VectorXd VW = piece.VW.cwiseSqrt();
                tyro::stop_motion_vertex_distance(num_labels, 
                                                smooth_weight,
                                                kmeans,
                                                piece.VD, 
                                                VW, 
                                                app->ANIM.SIdx,
                                                piece.F,
                                                sm.D, //dictionary
                                                sm.L, //labels  
                                                result_energy);

                errors.push_back(result_energy);
            }      

            for (auto k : errors)    
                RA_LOG_INFO("--ENERGY %f", k);
   
         }

        void console_seam_from_selection(App* app, const std::vector<std::string>& args) 
        {            
            MatrixXi EI, EF;
            igl::edge_flaps(app->ANIM.F, app->ANIM.E, app->ANIM.EMAP, EF, EI);

            std::set<int> V;
            for (int i=0; i < EF.rows(); ++i) 
            {
                int fid1 = EF(i,0), fid2 = EF(i,1);
                if (fid1 == -1 || fid2 == -1) continue; 

                auto it1 = std::find(app->fid_list.begin(), app->fid_list.end(), fid1);
                auto it2 = std::find(app->fid_list.begin(), app->fid_list.end(), fid2);
                
                if (  (it1 == app->fid_list.end() && it2 != app->fid_list.end()) 
                        || 
                      (it1 != app->fid_list.end() && it2 == app->fid_list.end()) 
                    )                     
                {
                    V.insert(app->ANIM.UE(i,0));
                    V.insert(app->ANIM.UE(i,1));
                }
            }

            app->vid_list.clear();
            app->vid_list.resize(V.size());
            std::copy(V.begin(), V.end(), app->vid_list.begin());
            app->ball_list.clear();

            for (auto vid : app->vid_list) 
            {
                app->addSphere(vid, app->ANIM.VD[app->m_frame]);
            }
            app->render();
        }
        
        void console_compute_vertex_bndr(App* app, const std::vector<std::string>& args) 
        {
            assert(app->LABELS.size()>0);

            MatrixXi EI, EF;
            igl::edge_flaps(app->ANIM.F, app->ANIM.E, app->ANIM.EMAP, EF, EI);

            //std::set<int> V;
            app->vid_list.clear();
            app->ball_list.clear();
            for (int i=0; i < EF.rows(); ++i) 
            {
                int fid1 = EF(i,0), fid2 = EF(i,1);
                if (fid1 == -1 || fid2 == -1) continue; 
                       
                if (app->LABELS(fid1) != app->LABELS(fid2)) 
                {
                    //V.insert(app->ANIM.UE(i,0));
                    //V.insert(app->ANIM.UE(i,1));
                    app->add_vertex(app->ANIM.UE(i,0));
                    app->add_vertex(app->ANIM.UE(i,1));
                }
            }

            //app->vid_list.clear();
            //app->vid_list.resize(V.size());
            //std::copy(V.begin(), V.end(), app->vid_list.begin());
            //app->ball_list.clear();

            for (auto vid : app->vid_list) 
            {
                app->addSphere(vid, app->ANIM.VD[app->m_frame]);
            }
            app->render();
        }
        

        void console_show_iso_2(App* app, const std::vector<std::string>& args) 
        {
            if (args.size()!=1)  
            {
                RA_LOG_WARN("Need to specify type of the curve");
                return; 
            }
            using namespace Eigen;
            VectorXd DA; //double area
            igl::doublearea(app->ANIM.AvgVD, app->ANIM.F, DA);

            int num_vert = app->ANIM.AvgVD.rows();
            int num_faces = app->ANIM.F.rows();
            std::vector<std::vector<int>> VF, VI;
            igl::vertex_triangle_adjacency(num_vert, app->ANIM.F, VF, VI);
            SparseMatrix<double> W;
            W.resize(num_vert, num_faces);
            for (int vid = 0; vid < VF.size(); ++vid) 
            {
                double area_total = 0;
                for (auto& fid : VF[vid]) 
                {   
                    area_total += DA[fid];
                }
                RA_LOG_INFO ("Area %f", area_total);
                assert(area_total > 0.000001);

                for (auto& fid : VF[vid]) 
                {
                    double area_w = DA[fid];                    
                    W.coeffRef(vid, fid) = area_w/area_total;
                }                 
            }

            app->LABELS.resize(app->ANIM.F.rows());
            app->LABELS.setOnes();
            for (int i =0; i < app->fid_list.size(); ++i)
            {      
                 app->LABELS(app->fid_list[i]) = 0;
            }
            //other label is zero 

            MatrixXd LV = W * app->LABELS;  
            MatrixXd sLV; //smoothed

            if(!igl::is_edge_manifold(app->ANIM.F)) 
            {   
                RA_LOG_ERROR_ASSERT("Not edge manifold");
                exit(0);
            }

            int smooth_iter = 1;
            for (int i = 0; i < smooth_iter; ++i) 
            {
                //igl::per_vertex_attribute_smoothing(LV, app->ANIM.F, sLV);
                tyro::smooth2(app->ANIM.AvgVD, app->ANIM.F, LV, 0.05, sLV);
                //tyro::smooth(app->ANIM.AvgVD, app->ANIM.F, LV, 0.001, sLV);
                
                LV = sLV.eval();
            }
            Eigen::VectorXd val(1);
            val(0) = 0.5;
            Eigen::MatrixXd U;
            Eigen::MatrixXi G;
            Eigen::VectorXi J;
            Eigen::SparseMatrix<double> BC;
            Eigen::VectorXd SU, L;
                        
            app->m_computed_iso_color = true;

            slice_isolines(app->ANIM.AvgVD, app->ANIM.F, LV.col(0), val, U, G, J, BC, SU, L);
            
            app->ISOCOLORS = SU;
            app->FCSPLIT.resize(G.rows(), 3);
            app->LABELS = L;
            for (int i = 0; i < L.size(); ++i) 
            {   
                if (L(i) == 0) 
                    app->FCSPLIT.row(i) = Vector3d(0,1,0);
                else
                    app->FCSPLIT.row(i) =  Vector3d(1,0,0);
            }

            app->VSPLIT = U;
            app->FSPLIT = G;
            igl::per_vertex_normals(U, G, app->NSPLIT);

            MatrixXi E;
            MatrixXi uE;
            VectorXi EMAP;
            std::vector<std::vector<int>> uE2E;
            igl::unique_edge_map(G, E, uE, EMAP, uE2E);
            
            app->UESPLIT = uE;
            app->UCSPLIT.resize(uE.rows(), 3);
            app->UCSPLIT.setConstant(0.5);

            propogate_changes_to_frames(app, BC, G);
            app->ANIM.FC = app->FCSPLIT;
            RA_LOG_INFO("done");
        }

        void console_show_iso(App* app, const std::vector<std::string>& args) 
        {
            if (args.size()!=1)  
            {
                RA_LOG_WARN("Need to specify type of the curve");
                return; 
            }
            using namespace Eigen;
            VectorXd DA; //double area
            igl::doublearea(app->ANIM.AvgVD, app->ANIM.F, DA);

            int num_vert = app->ANIM.AvgVD.rows();
            int num_faces = app->ANIM.F.rows();
            std::vector<std::vector<int>> VF, VI;
            igl::vertex_triangle_adjacency(num_vert, app->ANIM.F, VF, VI);
            SparseMatrix<double> W;
            W.resize(num_vert, num_faces);
            for (int vid = 0; vid < VF.size(); ++vid) 
            {
                double area_total = 0;
                for (auto& fid : VF[vid]) 
                {   
                    area_total += DA[fid];
                }
                RA_LOG_INFO ("Area %f", area_total);
                assert(area_total > 0.000001);

                for (auto& fid : VF[vid]) 
                {
                    double area_w = DA[fid];                    
                    W.coeffRef(vid, fid) = area_w/area_total;
                }                 
            }


            MatrixXd LV = W * app->LABELS;
            MatrixXd sLV; //smoothed

            if(!igl::is_edge_manifold(app->ANIM.F)) 
            {   
                RA_LOG_ERROR_ASSERT("Not edge manifold");
                exit(0);
            }

            int smooth_iter = 1;
            for (int i = 0; i < smooth_iter; ++i) 
            {
                //igl::per_vertex_attribute_smoothing(LV, app->ANIM.F, sLV);
                tyro::smooth2(app->ANIM.AvgVD, app->ANIM.F, LV, 0.008, sLV);
                //tyro::smooth(app->ANIM.AvgVD, app->ANIM.F, LV, 0.001, sLV);
                
                LV = sLV.eval();
            }
            Eigen::VectorXd val(1);
            val(0) = 0.5;
            Eigen::MatrixXd U;
            Eigen::MatrixXi G;
            Eigen::VectorXi J;
            Eigen::SparseMatrix<double> BC;
            Eigen::VectorXd SU, L;
            
            
            app->m_computed_iso_color = true;

            slice_isolines(app->ANIM.AvgVD, app->ANIM.F, LV.col(0), val, U, G, J, BC, SU, L);
            
            app->ISOCOLORS = SU;
            app->FCSPLIT.resize(G.rows(), 3);
            app->LABELS = L;
            for (int i = 0; i < L.size(); ++i) 
            {   
                if (L(i) == 0) 
                    app->FCSPLIT.row(i) = Vector3d(0,1,0);
                else
                    app->FCSPLIT.row(i) =  Vector3d(1,0,0);
            }

            app->VSPLIT = U;
            app->FSPLIT = G;
            igl::per_vertex_normals(U, G, app->NSPLIT);

            MatrixXi E;
            MatrixXi uE;
            VectorXi EMAP;
            std::vector<std::vector<int>> uE2E;
            igl::unique_edge_map(G, E, uE, EMAP, uE2E);
            
            app->UESPLIT = uE;
            app->UCSPLIT.resize(uE.rows(), 3);
            app->UCSPLIT.setConstant(0.5);

            propogate_changes_to_frames(app, BC, G);
            app->ANIM.FC = app->FCSPLIT;
            RA_LOG_INFO("done");
        }

         void console_load_bunny_stop2(App* app, const std::vector<std::string>& args) 
        {   
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            //auto p = f/filesystem::path("2018_bunny_3d_print_frames");
            auto p = f/filesystem::path("bunny_sub_2");
            //auto p = f/filesystem::path("bunny_temp_anim");
    
            std::ifstream in = std::ifstream(p.str(), std::ios::binary);
            cereal::BinaryInputArchive archive_i(in);
            archive_i(app->ANIM);
            tyro::color_matrix(app->ANIM.UE.rows(), Eigen::Vector3d(0,0,0), app->ANIM.UEC);

            app->ANIM.SIdx = {105, 450, 211}; 
            assert((app->ANIM.SIdx[0] + app->ANIM.SIdx[1] + app->ANIM.SIdx[2])  == app->ANIM.VD.size());

            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);
            bool shouldscale = true;

            double S;
            Eigen::RowVector3d tr;
            
            if (shouldscale)
                scale_one(app->ANIM, S, tr);
                
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = 0.8*2*bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            
            app->ANIM.VW.resize(app->ANIM.VD[0].rows());
            app->ANIM.VW.setOnes();

            app->compute_average();
            
            app->ShowBunnyMovieStop2();
           
            app->m_ball_size = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F)/5;

            std::vector<std::string> args1 = {"split", "bunny_split_2"};
            console_load_serialised_data(app, args1);
            if (shouldscale) 
            {
                if (!app->PIECES.empty()) 
                {
                    scale_one_exe(app->PIECES[0], S, tr);
                    scale_one_exe(app->PIECES[1], S, tr);
                }
            }


            std::vector<std::string> args4 = {"deform", "bunny_deform_2"};
            console_load_serialised_data(app, args4);
            if (shouldscale) 
            {
                scale_one_exe(app->DANIM, S, tr);
                app->DANIM.AvgVD = S * (app->DANIM.AvgVD.rowwise() + tr);
            }

            std::vector<std::string> args2 = {"stop_low", "bunny_stop_15_1_0"};
            //console_load_serialised_data(app, args2);
            
            std::vector<std::string> args3 = {"stop_up", "1A_bunny_20_1_1_kmeans"};
            //console_load_serialised_data(app, args3);
            
            if (shouldscale) 
            {
                if (!app->SMOTION.empty()) 
                {
                    scale_one_exe(app->SMOTION[0].anim, S, tr);
                    scale_one_exe(app->SMOTION[1].anim, S, tr);
                }
            }

            auto clr = Eigen::Vector3d(255/255.0, 148/255.0, 135/255.0);
            
            auto clr1 = Eigen::Vector3d(255/255.0, 255/255.0, 255/255.0);
            
            auto clr2 = Eigen::Vector3d(180/255.0, 100/255.0, 179/255.0);
            auto clr3 = Eigen::Vector3d(120/255.0, 151/255.0, 200/255.0);

            tyro::color_matrix(app->ANIM.F.rows(), clr, app->ANIM.FC);
            if (app->PIECES.size()>0)
            {
                tyro::color_matrix(app->PIECES[0].F.rows(), clr2, app->PIECES[0].FC);
                tyro::color_matrix(app->PIECES[1].F.rows(), clr3, app->PIECES[1].FC);
            }

            if (app->SMOTION.size()>0)
            {
                tyro::color_matrix(app->SMOTION[0].anim.F.rows(), clr2, app->SMOTION[0].anim.FC);
                tyro::color_matrix(app->SMOTION[1].anim.F.rows(), clr3, app->SMOTION[1].anim.FC);
            }
            app->render();
        }

        void console_load_jali(App* app, const std::vector<std::string>& args) 
        {
            RA_LOG_INFO("load jali");
            int offset_vid = 1030; // 1222;
            auto offset = Eigen::Vector3d(0.268563, 3.142050, 2.504273) ; //Eigen::Vector3d(0.613322, 2.613381, 2.238946);
        
            app->FOLDERS = {std::string("/home/rinat/GDrive/StopMotionProject/JALIOBJ")};

            bool serialized = true;

            if (serialized) 
            {   
                auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
                //auto p = f/filesystem::path("jali_frames_face");
                auto p = f/filesystem::path("jali_lower_face");
                
                std::ifstream in = std::ifstream(p.str(), std::ios::binary);
                cereal::BinaryInputArchive archive_i(in);
                archive_i(app->ANIM);
                tyro::color_matrix(app->ANIM.UE.rows(), Eigen::Vector3d(0,0,0), app->ANIM.UEC);
            }
            else
            {   
                std::vector<std::string> obj_paths;
                for (auto folder : app->FOLDERS) 
                {
                    int num_files_read;

                    RA_LOG_INFO("loading folder %s", folder.data());
                    tyro::obj_file_path_list(folder, "objlist.txt", obj_paths, num_files_read);
                    RA_LOG_INFO("frames read %i", num_files_read);
                    app->ANIM.SIdx.push_back(num_files_read);
                }
                app->load_mesh_sequence(obj_paths, true); //use IGL obj loader
            }
            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);

            //align_all_models(offset_vid, offset);
            if(!igl::is_edge_manifold(app->ANIM.F)) 
            {
                RA_LOG_ERROR("Mesh is not edge manifold");
                //exit(0);
            }
            double S;
            Eigen::RowVector3d tr;
            scale_one(app->ANIM, S, tr);
            
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = 2*bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            
            app->ANIM.VW.resize(app->ANIM.VD[0].rows());
            app->ANIM.VW.setOnes();

            app->compute_average();
            app->m_ball_size = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F)/5;

            //compute ambient_occlusion
        // Eigen::VectorXd AO;
        /*
            for (int i = 0; i < 0; ++i) 
            {               
                app->ANIM.AO.resize(ANIM.VD.size());
                igl::ambient_occlusion(ANIM.VD[i], 
                                    ANIM.F, 
                                    ANIM.VD[i], 
                                    ANIM.ND[i], 
                                    1000, 
                                    ANIM.AO[3114]);
                RA_LOG_INFO("Computed AO for frame %i", i);
                //RA_LOG_INFO("AO %f, %f",ANIM.AO[339].minCoeff(), ANIM.AO[339].maxCoeff());
            }
            */
            //std::vector<std::string> args2 = {"split", "2018_bunny_split_vertical"};

            //std::vector<std::string> args2 = {"split", "rabbit_hor_split"};
            std::vector<std::string> args1 = {"split", "bunny_split_fuck"};
            //console_load_serialised_data(app, args1);


            std::vector<std::string> args2 = { "deform", "bunny_deform_fuck"};
            //console_load_serialised_data(app, args2);
            

            std::vector<std::string> args3 = {"stop_low", "bunny_170_1_0_kmeans"};
            //console_load_serialised_data(this, args3);
            
            std::vector<std::string> args4 = {"stop_up", "bunny_80_1_1_"};
            //console_load_serialised_data(this, args4);
            

            //addAnim1.FC
            //app->ParseImages();
            
            app->render();
        }

        void console_load_bunny_stop(App* app, const std::vector<std::string>& args) 
        {   
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            //auto p = f/filesystem::path("2018_bunny_3d_print_frames");
            auto p = f/filesystem::path("bunny_sub_frames_fuck");
            //auto p = f/filesystem::path("bunny_temp_anim");
    
            std::ifstream in = std::ifstream(p.str(), std::ios::binary);
            cereal::BinaryInputArchive archive_i(in);
            archive_i(app->ANIM);
            tyro::color_matrix(app->ANIM.UE.rows(), Eigen::Vector3d(0,0,0), app->ANIM.UEC);

            app->ANIM.SIdx = {105, 450, 211}; 
            assert((app->ANIM.SIdx[0] + app->ANIM.SIdx[1] + app->ANIM.SIdx[2])  == app->ANIM.VD.size());

            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);
            bool shouldscale = true;

            double S;
            Eigen::RowVector3d tr;
            
            if (shouldscale)
                scale_one(app->ANIM, S, tr);
                
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = 0.8*2*bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            
            app->ANIM.VW.resize(app->ANIM.VD[0].rows());
            app->ANIM.VW.setOnes();

            app->compute_average();
            
            app->ShowBunnyMovieStop();
           
            app->m_ball_size = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F)/5;

            std::vector<std::string> args1 = {"split", "bunny_sub_split_fuck"};
            console_load_serialised_data(app, args1);
            if (shouldscale) 
            {
                if (!app->PIECES.empty()) 
                {
                    scale_one_exe(app->PIECES[0], S, tr);
                    scale_one_exe(app->PIECES[1], S, tr);
                }
            }


            std::vector<std::string> args4 = {"deform", "bunny_split_deform_fuck"};
            console_load_serialised_data(app, args4);
            if (shouldscale) 
            {
                scale_one_exe(app->DANIM, S, tr);
                app->DANIM.AvgVD = S * (app->DANIM.AvgVD.rowwise() + tr);
            }

            std::vector<std::string> args2 = {"stop_low", "bunny_stop_15_1_0"};
            //console_load_serialised_data(app, args2);
            
            std::vector<std::string> args3 = {"stop_up", "1A_bunny_20_1_1_kmeans"};
            //console_load_serialised_data(app, args3);
            
            if (shouldscale) 
            {
                if (!app->SMOTION.empty()) 
                {
                    scale_one_exe(app->SMOTION[0].anim, S, tr);
                    scale_one_exe(app->SMOTION[1].anim, S, tr);
                }
            }

            auto clr = Eigen::Vector3d(255/255.0, 148/255.0, 135/255.0);
            
            auto clr1 = Eigen::Vector3d(255/255.0, 255/255.0, 255/255.0);
            
            auto clr2 = Eigen::Vector3d(180/255.0, 100/255.0, 179/255.0);
            auto clr3 = Eigen::Vector3d(120/255.0, 151/255.0, 200/255.0);

            tyro::color_matrix(app->ANIM.F.rows(), clr, app->ANIM.FC);
            if (app->PIECES.size()>0)
            {
                tyro::color_matrix(app->PIECES[0].F.rows(), clr2, app->PIECES[0].FC);
                tyro::color_matrix(app->PIECES[1].F.rows(), clr3, app->PIECES[1].FC);
            }

            if (app->SMOTION.size()>0)
            {
                tyro::color_matrix(app->SMOTION[0].anim.F.rows(), clr2, app->SMOTION[0].anim.FC);
                tyro::color_matrix(app->SMOTION[1].anim.F.rows(), clr3, app->SMOTION[1].anim.FC);
            }
            app->render();
        }
        
        void console_write_labels(App* app, const std::vector<std::string>& args) 
        {  
            RA_LOG_INFO("save selected LABELS");
            auto path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/LABELS.txt");

            std::ofstream outFile(path);

            Eigen::VectorXi L;
            if (app->LABELS.size()>0) 
            {
                //L = app->LABELS;
            }
            else 
            {
                L.resize(app->ANIM.F.rows());
                for (int i=0; i <  app->PIECES_IDX[0].size(); ++i) 
                {   
                    int fid  = app->PIECES_IDX[0](i);
                    app->setFaceColor(fid, Eigen::Vector3d(0,1,0));
                    L(fid) = 1;
                }

                for (int i=0; i < app->PIECES_IDX[1].size(); ++i) 
                {   
                    int fid  = app->PIECES_IDX[1](i);
                    RA_LOG_INFO("fid %i %i", fid, app->ANIM.F.rows());
                    app->setFaceColor(fid, Eigen::Vector3d(1,0,0));
                    L(fid) = 0;
                }
            }


            for (int i = 0; i < L.size(); ++i) 
            {
                outFile << (int)L(i) << "\n";
            }

            outFile.close();
        }

        void console_save_few_deform_frames(App* app, const std::vector<std::string>& args ) 
        {
            igl::writeOBJ("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/print/f_01.obj", app->DANIM.VD[500],  app->DANIM.F);
            igl::writeOBJ("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/print/f_02.obj", app->DANIM.VD[1000], app->DANIM.F);
            igl::writeOBJ("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/print/f_03.obj", app->DANIM.VD[2000], app->DANIM.F);
            igl::writeOBJ("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/print/f_04.obj", app->DANIM.VD[3000], app->DANIM.F);
            igl::writeOBJ("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/print/f_05.obj", app->DANIM.VD[4000], app->DANIM.F);
            igl::writeOBJ("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/print/f_06.obj", app->DANIM.VD[5000], app->DANIM.F);
        }

        void console_save_first_frame(App* app, const std::vector<std::string>& args ) 
        {
            igl::writeOBJ("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/print/base.obj", app->ANIM.VD[0], app->ANIM.F);
        }

        void console_save_sub_video(App* app, const std::vector<std::string>& args) 
        {   
            if (args.size() != 1) return;
            std::vector<int> frame = {0,1041};    
            App::VideoCV video;

             //main animation
            int total_size = 0;
            for (int s  = 0; s < frame.size(); s+=2) 
            { 
                total_size += frame[s+1] - frame[s] + 1;
            } 

            video.F.resize(total_size);
            
            int cur_frame = 0;
            for (int s  = 0; s < frame.size(); s+=2) 
            {   
                int start = frame[s];
                int end = frame[s+1];
                for (int f = start; f <= end; ++f) 
                {
                    video.F[cur_frame] = app->m_video.F[f];
                    cur_frame += 1;
                }
            } 

            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path(args[0]);
            std::ofstream os(p.str(), std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(video);

        }
void copy_sub2(App::MAnimation& subanimaton, std::vector<int> frame, App::MAnimation& sourceAnim) 
        {
            using namespace Eigen;
            using namespace std;
            
            //main animation
            int total_size = 0;
            for (int s  = 0; s < frame.size(); s+=2) 
            { 
                total_size += frame[s+1] - frame[s] + 1;
            } 

            //main animation
            subanimaton.VD.resize(total_size);
            //subanimaton.ND.resize(total_size);

            int cur_frame = 0;
            for (int s  = 0; s < frame.size(); s+=2) 
            {   int start = frame[s];
                int end = frame[s+1];
                for (int f = start; f <= end; ++f) 
                {
                    subanimaton.VD[cur_frame] = sourceAnim.VD[f];
                    //subanimaton.ND[cur_frame] = sourceAnim.ND[f];
                    cur_frame += 1;
                }
            } 

            subanimaton.F = sourceAnim.F.eval();
            subanimaton.E = sourceAnim.E.eval();
            subanimaton.UE = sourceAnim.UE.eval();
            subanimaton.EMAP = sourceAnim.EMAP.eval();
            subanimaton.FC = sourceAnim.FC.eval();
            subanimaton.UEC = sourceAnim.UEC.eval();
            subanimaton.AvgVD = sourceAnim.AvgVD.eval();
            subanimaton.SIdx.push_back(sourceAnim.VD.size());
        }
        void copy_sub(App::MAnimation& subanimaton, std::vector<int> frame, App::MAnimation& sourceAnim) 
        {
            using namespace Eigen;
            using namespace std;
            
            //main animation
            int total_size = 0;
            for (int s  = 0; s < frame.size(); s+=2) 
            { 
                total_size += frame[s+1] - frame[s] + 1;
            } 

            //main animation
            subanimaton.VD.resize(total_size);
            subanimaton.ND.resize(total_size);

            int cur_frame = 0;
            for (int s  = 0; s < frame.size(); s+=2) 
            {   int start = frame[s];
                int end = frame[s+1];
                for (int f = start; f <= end; ++f) 
                {
                    subanimaton.VD[cur_frame] = sourceAnim.VD[f];
                    subanimaton.ND[cur_frame] = sourceAnim.ND[f];
                    cur_frame += 1;
                }
            } 

            subanimaton.F = sourceAnim.F.eval();
            subanimaton.E = sourceAnim.E.eval();
            subanimaton.UE = sourceAnim.UE.eval();
            subanimaton.EMAP = sourceAnim.EMAP.eval();
            subanimaton.FC = sourceAnim.FC.eval();
            subanimaton.UEC = sourceAnim.UEC.eval();
            subanimaton.AvgVD = sourceAnim.AvgVD.eval();
            subanimaton.SIdx.push_back(sourceAnim.VD.size());
        }
        
        void console_save_sub_split(App* app, const std::vector<std::string>& args) 
        {   
            if (args.size() != 1) return;

            //std::vector<int> frame = {851,1501};
            //std::vector<int> frame = {1042,1597,1738,1947};    
            std::vector<int> frame = {0,1041};    

            std::vector<App::MAnimation> SUBPIECES; // Break deformed mesh into pieces along seam(s).
            SUBPIECES.resize(app->PIECES.size());
           
            for (int i=0; i<SUBPIECES.size(); ++i) 
            {   
                copy_sub(SUBPIECES[i], frame, app->PIECES[i]);
            }
            
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path(args[0]);
            std::ofstream os(p.str(), std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(SUBPIECES);
            archive(app->PIECES_IDX);
            
        }

        void console_save_sub_animation(App* app, const std::vector<std::string>& args) 
        {   
            if (args.size() != 1) return;
    
            //std::vector<int> frame = {851,1501};    
            //std::vector<int> frame = {1042,1597,1738,1947};
            std::vector<int> frame = {0,1041};    
            
            //main animation
            App::MAnimation subanimaton;
            copy_sub(subanimaton, frame, app->ANIM);

            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path(args[0]);
            std::ofstream os(p.str(), std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(subanimaton);
        }
        
        void console_save_sub_deformation(App* app, const std::vector<std::string>& args) 
        {   
            if (args.size() != 1) return;
    
            std::vector<int> frame = {851,1501};    
            //std::vector<int> frame = {1042,1597,1738,1947};
            //std::vector<int> frame = {0,1041};    

            //main animation
            App::MAnimation subanimaton;
            copy_sub2(subanimaton, frame, app->DANIM);

            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path(args[0]);
            std::ofstream os(p.str(), std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(subanimaton);
        }

        void console_save_for_printing(App* app, const std::vector<std::string>& args) 
        {   
            if (args.size() != 1) 
                return;

            int part_id = std::stoi(args[0]);
            auto path_up = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/print/up");
            
            if (part_id == 0) 
                path_up = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/print/low");

            auto& part = app->SMOTION[part_id];

            //MatrixXd SV; 
            //MatrixXi SF;
            //VectorXi SVI, SVJ;
            //igl::remove_duplicate_vertices(part.D[0], part.anim.F, 0.00001, SV, SVI, SVJ, SF);

            //MatrixXi FF;
            //igl::collapse_small_triangles(SV, SF, 0.000001, FF);
            
            //MatrixXd newVD;        
            //MatrixXi newF;  
            //VectorXi I1;      
            //igl::remove_unreferenced(SV,FF,newVD,newF,I1);
                
            for (int i = 0; i < part.D.size(); ++i) 
            {   
                auto file_name = filesystem::path(tyro::pad_zeros(i) + std::string(".obj"));
                auto p = path_up/file_name;
 
                igl::writeOBJ(p.str(), part.D[i], part.anim.F);
            }

            RA_LOG_INFO("save selected faces");
            auto path = path_up/filesystem::path("labels.txt");

            std::ofstream outFile(path.str());
            for (int i=0; i < part.L.size(); ++i)
            { 
                outFile << part.L(i) << "\n";
            }            
        }

        void console_render_to_video(App* app, const std::vector<std::string>& args) 
        {   
            if (args.size() != 1) return;

            auto filename = args[0];
            int v_width, v_height;
            app->m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
            
            cv::Size S = cv::Size(v_width, v_height);

            cv::VideoWriter outputVideo;                                        // Open the output
            outputVideo.open(string("/home/rinat/GDrive/StopMotionProject/Results/") + filename + string(".avi"),
                             CV_FOURCC('M','J','P','G'), 
                             //CV_FOURCC('x', '2', '6', '4'),
                             24, 
                             S, 
                             true);
            
            if (!outputVideo.isOpened())
            {
                cout  << "Could not open the output video for write" << endl;
                return;
            }

            for (int frame = 0; frame < app->ANIM.VD.size(); ++frame) 
            {   
                app->m_frame = frame;
                app->DrawMeshes();

                // Poll for and process events
                app->m_tyro_window->GetGLContext()->swapBuffers();

                //make sure everything was drawn
                glFlush();
                glFinish();
                GL_CHECK_ERROR;
                
                u_int8_t* texture = (u_int8_t*) malloc(4*v_width *v_height);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glReadPixels(0, 0, v_width, v_height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
                GL_CHECK_ERROR;

                cv::Mat image1(v_height, v_width, CV_8UC4, texture);
                cv::Mat image2, image3;
                cv::cvtColor(image1, image2, CV_RGBA2BGR);
                cv::flip(image2, image3, 0);

                outputVideo.write(image3);
                free(texture);
                cout << "Saved frame " << frame  << endl;

            }
            
            cout << "Finished writing" << endl;
        }    

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
            
            auto fldr = filesystem::path("/home/rinat/GDrive/StopMotionProject/Results/images")/name;
            
            if (fldr.exists()) 
                fldr = fldr/filesystem::path("_1");

            cv::imwrite(fldr.str(), image3);
            free(texture);
        }

        void console_show_seam(App* app, const std::vector<std::string>& args) 
        {
            app->m_eid_list.clear();
            app->m_eid_list.resize(app->SMOTION.size());
            for (int i = 0; i < app->SMOTION.size(); ++i) 
            {   
                if (app->SMOTION[i].computed) 
                {   
                    const auto & cE = app->SMOTION[i].anim.UE;
                    const auto & cEMAP = app->SMOTION[i].anim.EMAP;
                    MatrixXi EF, EI;

                    igl::edge_flaps(app->SMOTION[i].anim.F, cE, cEMAP, EF, EI);
                    
                    for (int j = 0; j < EF.rows(); ++j) 
                    {
                        int f1 = EF(j,0);
                        int f2 = EF(j,1);

                        if (f1 == -1 || f2 == -1) 
                        {
                            app->m_eid_list[i].push_back(j);
                        }
                    }
                }
            }
        }

        void console_plot_total_error_data(App* app, const std::vector<std::string>& args) 
        {

        } 

        void console_plot_total_error_vel(App* app, const std::vector<std::string>& args) 
        {
            
        } 

        void console_plot_error_deform(App* app, const std::vector<std::string>& args)
        {   
            int num_vert = app->ANIM.AvgVD.rows();
            int num_frames = app->ANIM.VD.size();
            app->max_def_error = std::numeric_limits<float>::min();
            app->m_error_deform.resize(num_frames, num_vert);

            //do biharmonic stuff here.
            Eigen::SparseMatrix<double> C, M;            
            igl::cotmatrix(app->ANIM.AvgVD, app->ANIM.F, C); //try average
            igl::massmatrix(app->ANIM.AvgVD, app->ANIM.F, igl::MASSMATRIX_TYPE_DEFAULT, M);

            //Computer M inverse
            Eigen::SimplicialLDLT <Eigen::SparseMatrix<double>> solver;
            solver.compute(M);
            Eigen::SparseMatrix<double> I(M.rows(), M.cols());
            I.setIdentity();
            Eigen::SparseMatrix<double> M_inv = solver.solve(I);
            Eigen::SparseMatrix<double> L = M_inv*C;

            for (int frame=0; frame<num_frames; ++frame) 
            {                   
                MatrixXd LD = (L*(app->ANIM.VD[frame] - app->DANIM.VD[frame])).rowwise().norm();  
                MatrixXd sLD; //smoothed

                int smooth_iter = 1;
                for (int i = 0; i < smooth_iter; ++i) 
                {
                    tyro::smooth2(app->ANIM.VD[frame], app->ANIM.F, LD, 0.1, sLD);
                    LD = sLD.eval();
                }

                app->max_def_error = std::max((float)LD.maxCoeff(), app->max_def_error);
                app->m_error_deform.row(frame) = LD.col(0);
                RA_LOG_INFO("Max_deform_error %i %f ", frame, LD.maxCoeff());
            }

            //MatrixXd ones;
            //ones.resize(app->m_error_deform.rows(), app->m_error_deform.cols());
            //ones.setOnes();
            //app->m_error_deform = (app->m_error_deform + ones).array().log().matrix();
            //app->max_def_error = app->m_error_deform.sum()/app->m_error_deform.size(); 
            //app->max_def_error = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F);
             RA_LOG_INFO("Max_deform_error %f %f", app->m_error_deform.maxCoeff(), app->max_def_error);    
            //app->max_def_error = app->m_error_deform.maxCoeff();
        }

        void console_plot_error_deform2(App* app, const std::vector<std::string>& args)
        {   
            int num_vert = app->ANIM.AvgVD.rows();
            int num_frames = app->ANIM.VD.size();
            app->max_def_error = std::numeric_limits<float>::min();
            app->m_error_deform.resize(num_frames, num_vert);

            for (int frame=0; frame<num_frames; ++frame) 
            {                   
                //VectorXd lul = (app->ANIM.VD[frame] - app->DANIM.VD[frame]).rowwise().norm();  
                VectorXd lul = app->Lap[frame].rowwise().norm();
                app->max_def_error = std::max((float)lul.maxCoeff(), app->max_def_error);
                RA_LOG_INFO("Max_deform_error %i %f ", frame, lul.maxCoeff());
                app->m_error_deform.row(frame) = lul;
            }
            MatrixXd ones;
            ones.resize(app->m_error_deform.rows(), app->m_error_deform.cols());
            ones.setOnes();
            //app->m_error_deform = (app->m_error_deform + ones).array().log().matrix();
            app->max_def_error = app->m_error_deform.sum()/app->m_error_deform.size(); 
            //app->max_def_error = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F);
            RA_LOG_INFO("Max_deform_error %f %f", app->m_error_deform.maxCoeff(), app->max_def_error);    
            //app->max_def_error = app->m_error_deform.maxCoeff();
        }

        void console_plot_error_vel(App* app, const std::vector<std::string>& args) 
        {
            using namespace Eigen;
       
            int num_parts = app->SMOTION.size();
            int num_frames = app->ANIM.VD.size();
            int num_vert = app->ANIM.AvgVD.rows();
            app->m_error_velocity.resize(num_parts);
            app->max_vel_error = std::numeric_limits<float>::min();
            
            for (int p = 0; p < num_parts; ++p) 
            {   
                if (app->SMOTION[p].computed) 
                {
                    app->m_error_velocity[p].resize(num_frames, num_vert); 
                    VectorXd zero;
                    zero.resize(app->PIECES[p].VD[0].rows());
                    zero.setZero();

                    app->m_error_velocity[p].row(0) = zero;

                    for (int frame = 1; frame < num_frames; ++frame) 
                    {                   
                        //VC[frame].resize(FD.VD[0].rows());
                        VectorXd lul = ((app->PIECES[p].VD[frame] - app->PIECES[p].VD[frame-1]) - 
                                        (app->SMOTION[p].anim.VD[frame] - app->SMOTION[p].anim.VD[frame-1])).rowwise().squaredNorm();  
                        app->max_vel_error = std::max((float)lul.maxCoeff(), app->max_vel_error);
                        //app->max_error_velocity = std::max(app->max_error_velocity, max); 
                        app->m_error_velocity[p].row(frame) = lul;                          
                    }
                    //RA_LOG_INFO("ERROR VEL %f", app->m_error_velocity[p].maxCoeff());
                }

            }
            app->m_computed_vel_error =true;
            
        }


        void console_plot_error(App* app, const std::vector<std::string>& args) 
        {   
            using namespace Eigen;
            int num_vert = app->ANIM.AvgVD.rows();
            int num_parts = app->SMOTION.size();
            int num_frames = app->ANIM.VD.size();
            app->m_error.resize(num_parts);
            //float max_error = std::;
            app->max_error = std::numeric_limits<float>::min();
            MatrixXd Hi;
            Hi.resize(num_frames, num_vert); 
            Hi.setOnes();
            for (int p = 0; p < num_parts; ++p) 
            {   
                if (app->SMOTION[p].computed) 
                {
                    for (int frame=0; frame < num_frames; ++frame) 
                    {                   
                        //VC[frame].resize(FD.VD[0].rows());
                        RowVectorXd lul = (app->PIECES[p].VD[frame] - app->SMOTION[p].anim.VD[frame]).rowwise().squaredNorm().transpose();  
                        app->max_error = std::max((float)lul.maxCoeff(), app->max_error);
                        ///RA_LOG_INFO("%f", k);
                        //app->max_error = std::max(app->max_error, max); 
                        Hi.row(frame) = lul;                          
                    }
                    bool k = Hi.hasNaN();
                    RA_LOG_INFO("ERROR DATA %i %i", Hi.maxCoeff(), Hi.minCoeff());
                    app->m_error[p] = Hi;
                }
            }
            app->m_computed_error =true;
        }

        void console_offset_frame(App* app, const std::vector<std::string>& args) 
        {
            if (args.size() == 1) 
            {
                int start = std::stoi(args[0]);
//                int end = std::stoi(args[1]);
                app->m_frame_offset = start;
                app->render();
                glfwPostEmptyEvent();
            }
        }
        
        void console_laplacian_smoothing_vert(App* app, const std::vector<std::string>& args) 
        {   
            std::vector<std::vector<int>> A;
            igl::adjacency_list(app->ANIM.F, A);
            
            for (int frame = 0; frame < app->ANIM.VD.size(); ++frame) 
            {   
                for (int iter = 0; iter < 6; ++iter) 
                {   
                    MatrixXd VD = app->ANIM.VD[frame];
                    for (auto vid:app->vid_list) 
                    {   
                        Eigen::Vector3d avg;
                        avg.setZero();
                        
                        for (auto vid_n : A[vid]) 
                        {   
                            avg += app->ANIM.VD[frame].row(vid_n);
                        }
                        avg = 1.0/A[vid].size() * avg;
                        Eigen::Vector3d cur = VD.row(vid);
                        
                        if (iter%2 == 0)
                            VD.row(vid) = cur + 0.5 * (avg - cur);
                            //Voriginal.row(app->vid_list[i]) +=  0.5*Lv;
                        else
                            VD.row(vid) = cur - 0.5 * (avg - cur);

                        
                    }
                    app->ANIM.VD[frame] = VD;
                }
            }
        }

        void console_laplacian_smooth_along_edges(App* app, const std::vector<std::string>& args) 
        {
              using Eigen::Vector3d;
            //Taubian smoothing
            if (app->vid_list.size() > 0 ) 
            {   
                for (int iter = 0; iter < 1; ++iter) 
                {   
                    const auto& FDV = app->ANIM.AvgVD;
                    MatrixXd Voriginal = app->ANIM.AvgVD.eval();

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
                        
                        //Lv += app->ANIM.VD[app->m_frame].row(vid_n);
                        Lv = (1.0/(w1+w2)) * Lv - v;
                        //std::cout<< Lv;
                        Voriginal.row(app->vid_list[i]) +=  0.5*Lv;
                        
                    }
                    app->ANIM.AvgVD = Voriginal.eval();
                }
                
                app->render();
                glfwPostEmptyEvent;
            }
        }
        
        void console_set_selection_type(App* app, const std::vector<std::string>& args) 
        {
            if (args.size() != 1 ) return;

            if (args[0] == "select") 
            {
                app->m_selection_type = App::SelectionType::Select;
            }
            else  
            {       
                app->m_selection_type = App::SelectionType::Deselect;
            }
        }

        void console_taubin_smooth_along_edges(App* app, const std::vector<std::string>& args) 
        {   
            using Eigen::Vector3d;
            //Taubian smoothing
            if (args.size() != 1) 
            {
                return;
            }

            bool isClosed = std::stoi(args[0]);
            if (app->vid_list.size() > 0) 
            {   

                for (int iter = 0; iter < 6; ++iter) 
                {   
                    const auto& FDV = app->ANIM.AvgVD;
                    MatrixXd Voriginal = app->ANIM.AvgVD.eval();

                    for (int i = 0; i < app->vid_list.size(); ++i) 
                    {   
                        Vector3d Lv;
                        Lv.setZero();
                        
                        int left = i-1; 
                        int right = i+1; 
                        
                        //check if v is boundary, then dont smooth
                        if (i==0) 
                        {   
                            if (!isClosed) continue;
                            left = app->vid_list.size()-1;
                            right = 1;  
                        }
                        else if (i == app->vid_list.size()-1) 
                        {   
                            if (!isClosed) continue;
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
                        
                        //Lv += app->ANIM.VD[app->m_frame].row(vid_n);
                        Lv = (1.0/(w1+w2)) * Lv - v;
                        //std::cout<< Lv;
                        if (iter%2 == 0)
                            Voriginal.row(app->vid_list[i]) +=  0.5*Lv;
                        else
                            Voriginal.row(app->vid_list[i]) += -0.55*Lv;
                        
                    }
                    app->ANIM.AvgVD = Voriginal.eval();
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
                archive(app->ANIM);
            }
            else if (type == "deform") 
            {
                archive(app->DANIM);
            }
            else if (type == "label" || type == "labels") 
            {
                archive(app->LABELS);
            }
            else if (type == "split") 
            {
                archive(app->PIECES);
                archive(app->PIECES_IDX);
            }
            else if (type == "split_up") 
            {
                archive(app->PIECES[0]);
            }
            else if (type == "split_low") 
            {
                archive(app->PIECES[1]);
            }
            else if (type == "stop") 
            {
                archive(app->SMOTION);
            }
            else if (type == "stop_low") 
            {
                archive(app->SMOTION[1]);
            }
            else if (type == "stop_up") 
            {
                archive(app->SMOTION[0]);
            }
            else if (type == "video") 
            {
                archive(app->m_video); 
            }
            else if (type == "weights_low") 
            {
                archive(app->PIECES[1].VW);
            }
            else if (type == "weights_up") 
            {
                archive(app->PIECES[0].VW);
            }
            else if (type == "seeds") 
            {
                archive(app->fid_list2);
                archive(app->fid_list3);
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
                archive(app->ANIM);

                app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);

                //Compute radius of the bounding box of the model
                AxisAlignedBBox bbox;
                MatrixXd VT = app->ANIM.VD[0].transpose();
                bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
                app->m_model_offset = 0.8*2* bbox.GetRadius(); 

                app->m_update_camera = true;
                app->m_state = App::State::LoadedModel;
            } 
            else if (type == "label" || type == "labels") 
            {
                archive(app->LABELS);
                for (int i = 0; i < app->LABELS.size(); ++i) 
                {   
                    if (app->LABELS(i) == 0) 
                        app->ANIM.FC.row(i) = Eigen::Vector3d(0,1,0);
                    else
                        app->ANIM.FC.row(i) =  Eigen::Vector3d(1,0,0);
                }
            }
            else if (type == "deform") 
            {
                archive(app->DANIM);
                app->m_computed_deformation = true;
                app->m_update_camera = true;
            }
            else if (type == "split") 
            {
                archive(app->PIECES);
                archive(app->PIECES_IDX);

                auto clr = Eigen::Vector3d(255/255.0, 148/255.0, 135/255.0);
                auto clr2 = Eigen::Vector3d(180/255.0, 100/255.0, 179/255.0);

                tyro::color_matrix(app->PIECES[0].F.rows(), clr, app->PIECES[0].FC);

                tyro::color_matrix(app->PIECES[1].F.rows(), clr2, app->PIECES[1].FC);

                app->m_computed_parts = true;            
                app->m_update_camera = true;
            }
            else if (type == "stop") 
            {
                archive(app->SMOTION);
                for (int i = 0; i < app->SMOTION.size(); ++i)
                    if (app->SMOTION[i].anim.VD.size() > 0)
                        app->SMOTION[i].computed = true;
                
                app->m_computed_stop_motion = true;
                app->m_update_camera = true;

            }
            else if (type == "stop_low") 
            {   
                if (app->SMOTION.empty())
                    app->SMOTION.resize(2);
                
                archive(app->SMOTION[1]);
                app->SMOTION[1].computed = true;
                
                app->m_computed_stop_motion = true;
                app->m_update_camera = true;
                auto clr = Eigen::Vector3d(180/255.0, 100/255.0, 179/255.0);

                tyro::color_matrix(app->SMOTION[1].anim.F.rows(), clr, app->SMOTION[1].anim.FC);
            }
            else if (type == "stop_up") 
            {
                if (app->SMOTION.empty())
                    app->SMOTION.resize(2);
                
                archive(app->SMOTION[0]);
                app->SMOTION[0].computed = true;

                auto clr = Eigen::Vector3d(255/255.0, 148/255.0, 135/255.0);

                tyro::color_matrix(app->SMOTION[0].anim.F.rows(), clr, app->SMOTION[0].anim.FC);

                app->m_computed_stop_motion = true;
                app->m_update_camera = true;
            }
            else if (type == "weights_low") 
            {
                archive(app->PIECES[1].VW);
                for (int i = 0; i < app->PIECES[1].VW.size(); ++i) 
                {
                    if (app->PIECES[1].VW[i] > 1 ) 
                    {
                        app->addSphere(i, app->PIECES[1].VD[app->m_frame], Wm5::Vector4f(0,1,0,1), app->RENDER.part[1]->WorldTransform);
                        app->vid_list2.push_back(i);
                    }               
                }
            }
            else if (type == "weights_up") 
            {
                archive(app->PIECES[0].VW);
                for (int i = 0; i < app->PIECES[0].VW.size(); ++i) 
                {
                    if (app->PIECES[0].VW[i] > 1)
                    {
                        app->addSphere(i, app->PIECES[0].VD[app->m_frame], Wm5::Vector4f(0,1,0,1), app->RENDER.part[0]->WorldTransform);
                        app->vid_list2.push_back(i);
                    }   
                }
            }
            else if (type == "seeds") 
            {
                archive(app->fid_list2);
                archive(app->fid_list3);

                for (auto fid : app->fid_list2) 
                {
                    app->setFaceColor(fid, Eigen::Vector3d(1,0,0));
                }
                for (auto fid : app->fid_list3) 
                {
                    app->setFaceColor(fid, Eigen::Vector3d(0,1,0));
                }
            }
            else if (type == "add1") 
            {
                archive(app->addAnim1);
            }
            else if (type == "add2") 
            {
                archive(app->addAnim2);
            }


            app->render();
            glfwPostEmptyEvent();
        }

        

        void console_clear_weights(App* app, const std::vector<std::string>& args) 
        {   
            app->PIECES[0].VW.setOnes();
            app->PIECES[1].VW.setOnes();
        }

        void console_clear_selection(App* app, const std::vector<std::string>& args) 
        {
            app->vid_list.clear();
            app->vid_list2.clear();
            app->ball_list.clear();
            
            for (int fid = 0; fid < app->ANIM.F.rows(); ++fid) 
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
            app->vid_list2.clear();
            app->ball_list.clear();
            app->render();
        }

        void console_clear_face_selection(App* app, const std::vector<std::string>& args) 
        {
            for (int fid = 0; fid < app->ANIM.F.rows(); ++fid) 
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
            if (args.size() == 2) 
            {   
                float w = std::stof(args[0]);
                int part_id = std::stoi(args[1]); 
                if (app->PIECES[part_id].VW.size() == 0) 
                {
                    app->PIECES[part_id].VW.resize(app->PIECES[part_id].VD[0].rows());
                    app->PIECES[part_id].VW.setOnes();                    
                }

                for (auto vid : app->vid_list2) 
                {
                    app->PIECES[part_id].VW[vid] = w;
                        RA_LOG_INFO("Weights set %f", w);

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
                    //app->m_weights.FW(fid) = w;
                }
            }
        }

        void console_draw_vertex_weight_map(App* app, const std::vector<std::string>& args) 
        {   
            /*
            double maxC = app->m_weights.VW.maxCoeff();
            double minC = app->m_weights.VW.minCoeff(); //should be 1
            
            for (int vid = 0; vid < app->m_weights.VW.size(); ++vid) 
            {   
                double w = app->m_weights.VW(vid); 
                if (w > 1) 
                {   
                    double red = w / (maxC - minC); 
                    app->addSphere(vid,  ANIM.VD[m_frame], Wm5::Vector4f(red, 0, 0, 1));
                }
            }
            */
        }

        void console_draw_face_weight_map(App* app, const std::vector<std::string>& args) 
        {   
            /*
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
            */
        } 

        void console_find_boundary(App* app, const std::vector<std::string> & args) 
        {   
            
            if (args.size() != 1) 
                return;
            
            int vid_list_id  = std::stoi(args[0]);

            const auto & cE = app->ANIM.UE;
            const auto & cEMAP = app->ANIM.EMAP;
            MatrixXi EF, EI;
            igl::edge_flaps(app->ANIM.F, cE, cEMAP, EF, EI);
            std::vector<int> seam_edges;

            for (int i = 0; i < app->ANIM.UE.rows(); ++i) 
            {               
                int f1 = EF(i, 0);
                int f2 = EF(i, 1);
                
                auto it1 = std::find(app->fid_list.begin(), app->fid_list.end(), f1);
                auto it2 = std::find(app->fid_list.begin(), app->fid_list.end(), f2);
                
                if (it1 != app->fid_list.end() && it2 != app->fid_list.end()) 
                {   
                    int idx1 = it1 - app->fid_list.begin();
                    int idx2 = it2 - app->fid_list.begin();
                    if (app->LABELS(idx1) != app->LABELS(idx2)) 
                    {
                        seam_edges.push_back(i);
                    }
                }
            }

            MatrixXi EU;
            EU.resize(seam_edges.size(), 2);
            for (int i=0;i < seam_edges.size(); ++i) 
            {   
                int eid = seam_edges[i];
                EU.row(i) = app->ANIM.UE.row(eid);
            }
            VectorXi I1, J1, K1; 
            igl::edges_to_path(EU, I1, J1, K1);
            
            for (int i=0; i <J1.size(); ++i) {
                //RA_LOG_INFO("Edge %i %i", EU(J1(i),0), EU(J1(i),1));
            }

            int end = I1.size();
            if (I1(0) == I1(I1.size()-1)) {
                RA_LOG_INFO("BOUNDARY IS CLOSED LOOP");
                end -= 1;
            }
            //RA_LOG_INFO("Vertex %i", I1(i));
            
            if (vid_list_id == 0) 
            {
                app->vid_list.clear();           

                for (int i=0; i < end; ++i) 
                {      
                    app->add_vertex(I1(i));
                    app->addSphere(I1(i),app->ANIM.VD[app->m_frame]);
                }
            }
            else if (vid_list_id == 1) 
            {   
                app->vid_list_second.clear();           
                for (int i=0; i < end; ++i) 
                { 
                    app->vid_list_second.push_back(I1(i));
                    app->addSphere(I1(i),app->ANIM.VD[app->m_frame]);
                }
            }
            else if (vid_list_id == 2) 
            {   
                app->vid_list_third.clear();           
                for (int i=0; i < end; ++i) 
                { 
                    app->vid_list_third.push_back(I1(i));
                    app->addSphere(I1(i),app->ANIM.VD[app->m_frame]);
                }
            }
            
        
            
            //app->ANIM.UEC.row(i) = Eigen::Vector3d(0,0.0,0.8);

            //Eigen::Vector2i evec = FD.ue_data.row(i);
            //if direction is switched after a mapping directed to undirected
            
            
            //if (!(evec(0)== e1(0) && evec(1) == e1(1))) 
            //{
            //    DMAP(i) = 1;
            //}
        }

        void console_show_boundary_faces(App* app, const std::vector<std::string>& args) 
        {   
            VectorXi I; MatrixXi C;
            igl::on_boundary(app->ANIM.F, I, C);
            for (int i=0; i < I.size(); ++i) 
            {
                if (I(i)) app->setFaceColor(i, Eigen::Vector3d(1,0,0));
            }
        }

        void console_segmentation_full(App* app, const std::vector<std::string>& args) 
        {  
            if (!(app->fid_list2.size() > 0 &&
                  app->fid_list3.size() > 0 &&
                  args.size() == 1)) 
            { 
                return;   
            }   

            VectorXi L;
            double smooth_weight = (double)std::stof(args[0]);
            
            //const auto& FD = app->ANIM;

            VectorXi seeds2, seeds3; //foreground and background         
            seeds2.resize(app->fid_list2.size());
            seeds3.resize(app->fid_list3.size());

            for (int i = 0; i < app->fid_list2.size(); ++i)
            {
                seeds2(i) = app->fid_list2[i];
            }

            for (int i = 0; i < app->fid_list3.size(); ++i)
            {
                seeds3(i) = app->fid_list3[i];
            }

            double avg_length = 0.1*igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F);
            tyro::segmentation2(app->ANIM.VD, 
                                app->ANIM.F,
                                app->ANIM.AvgVD,
                                seeds2,
                                seeds3,
                                smooth_weight,
                                avg_length,
                                L);
            
            app->LABELS.resize(L.size());
            for (int i = 0; i < L.size(); ++i) 
            {   
                app->LABELS(i) = (double)L(i);
                if (L(i) == 1) 
                {
                    app->setFaceColor(i, Eigen::Vector3d(0,0.5,0));
                }
                else 
                {
                    app->setFaceColor(i, Eigen::Vector3d(0.5,0,0));                
                }
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

            double smooth_weight = (double)std::stof(args[0]);
            
            //const auto& FD = app->ANIM;

            VectorXi L;
            VectorXi flist1 = Eigen::Map<VectorXi>(app->fid_list.data(), app->fid_list.size());
            MatrixXi F;
            igl::slice(app->ANIM.F, flist1, 1, F); 

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
            for (int i = 0; i < app->ANIM.VD.size(); ++i) 
            {   
                MatrixXd NV;
                igl::remove_unreferenced(app->ANIM.VD[i], F, NV, NF, I);
                v_data.push_back(NV);
            }

            MatrixXd Vavg;
            igl::remove_unreferenced(app->ANIM.AvgVD, F, Vavg, NF, I);
            
            //copmute boundary faces
            //VectorXi I1;
            //MatrixXi C1;
            //igl::on_boundary(app->ANIM.F, I1, C1);
            VectorXi isBoundary;
            isBoundary.resize(flist1.size());
            isBoundary.setZero();
            //for (int i = 0; i < isBoundary.size(); ++i) 
            //{   int fid = flist1(i);
            //    if (I1(fid)) 
                    //isBoundary(i) = 1; 
            //}

            tyro::segmentation(v_data, 
                               NF,
                               Vavg,
                               seeds2,
                               seeds3,
                               smooth_weight,
                               L, 
                               isBoundary);
            
            app->LABELS.resize(L.size());
            for (int i = 0; i < L.size(); ++i) 
            {   
                app->LABELS(i) = (double)L(i);
                if (L(i) == 1) 
                {
                    app->setFaceColor(flist1[i], Eigen::Vector3d(0,0.5,0));
                }
                else 
                {
                    app->setFaceColor(flist1[i], Eigen::Vector3d(0.5,0,0));                
                }
            }

            //find boundary
            //console_find_boundary(app, args);           
        }

        
        void console_remove_duplicate_vertices(App* app, const std::vector<std::string> & args) 
        {   
            using namespace Eigen;
            MatrixXd SV; 
            MatrixXi SF;
            VectorXi SVI, SVJ;
            igl::remove_duplicate_vertices(app->ANIM.VD[0], app->ANIM.F, 0.00001, SV, SVI, SVJ, SF);
            RA_LOG_INFO("CHANGES %i %i ", app->ANIM.VD[0].rows(), SV.rows());

       
            app->ANIM.F = SF;
            for (int i=0;i<app->ANIM.VD.size();++i) 
            {   
                MatrixXd V;
                igl::slice(app->ANIM.VD[i], SVI, 1, V);
                app->ANIM.VD[i] =  V;
                igl::per_vertex_normals(app->ANIM.VD[i], app->ANIM.F, app->ANIM.ND[i]);
            }
            
            std::vector<std::vector<int>> uE2E;
            igl::unique_edge_map(app->ANIM.F, app->ANIM.E, app->ANIM.UE, app->ANIM.EMAP, uE2E);

            tyro::color_matrix(app->ANIM.F.rows(), Eigen::Vector3d(0.5,0.5,0.5), app->ANIM.FC);
            tyro::color_matrix(app->ANIM.UE.rows(), Eigen::Vector3d(0.2,0.2,0.2), app->ANIM.UEC);
            
            app->ANIM.VW.resize(app->ANIM.VD[0].rows());
            app->ANIM.VW.setOnes();

            app->compute_average();

            app->RENDER.mesh = nullptr;
            app->RENDER.mesh_wire = nullptr;
            
            app->RENDER.avg = nullptr;
            app->RENDER.avg_wire = nullptr;       
        }

        void console_upsample(App* app, const std::vector<std::string> & args) 
        {
            Eigen::SparseMatrix<double> S;
            Eigen::MatrixXi newF;
            igl::upsample(app->ANIM.VD[0].rows(), app->ANIM.F, S, newF);
            app->ANIM.F = newF;

            for (int i =0; i < app->ANIM.VD.size(); ++i) 
            {
                app->ANIM.VD[i] = S * app->ANIM.VD[i];
                
                Eigen::MatrixXd N;
                int num_face = app->ANIM.F.rows();
                igl::per_vertex_normals(app->ANIM.VD[i], app->ANIM.F, N); 
                app->ANIM.ND[i] = N;

                std::vector<std::vector<int> > uE2E;
                igl::unique_edge_map(app->ANIM.F,
                                     app->ANIM.E,
                                     app->ANIM.UE,
                                     app->ANIM.EMAP,
                                     uE2E);
                Eigen::Vector3d face_color(0.5,0.5,0.5);
                tyro::color_matrix(app->ANIM.F.rows(), face_color, app->ANIM.FC);
                tyro::color_black_matrix(app->ANIM.UE.rows(), app->ANIM.UEC);
            }

            app->compute_average(); 
            console_save_serialised_data(app, args);                      
        }

        void console_show_wireframe(App* app, const std::vector<std::string> & args) 
        {
            if (args.size() == 1) 
            {
                int show = std::stoi(args[0]);
                app->m_show_wire = show;
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
            /*
            RA_LOG_INFO("Converting vertex to edge selection");
            
            if (args.size() != 1)  return;

            MatrixXi eid_list;
            VectorXi EI, uEI, DMAP;
            convert_vertex_to_edge_selection(app->vid_list, 
                                             app->vid_list_second,
                                             app->vid_list_third,
                                             app->ANIM.E, 
                                             app->ANIM.UE, 
                                             app->ANIM.EMAP, 
                                             std::stoi(args[0]), eid_list, EI, 
                                             uEI, DMAP);

            app->m_eid_list2.clear();
            for (int i = 0; i < uEI.size(); ++i) 
            {   
                app->m_eid_list2.push_back(uEI(i));
                app->ANIM.UEC.row(uEI(i)) = Eigen::Vector3d(0, 0.8, 0);
            }
            
            //app->m_show_seam = true;
            //debug_show_faces_near_edge_selection(uEI, DMAP);       
                    
            app->render();
            glfwPostEmptyEvent();
            
            return;
            */
            VectorXi B;
            igl::is_boundary_edge(app->SMOTION[0].anim.UE, app->SMOTION[0].anim.F, B);
            app->m_eid_list2.clear();
            for (int i = 0; i < app->SMOTION[0].anim.UE.rows(); ++i) 
            {  
                RA_LOG_INFO("print %i", B(i));
                if (B(i) == 1) 
                {
                    app->m_eid_list2.push_back(i);
                    app->SMOTION[0].anim.UEC.row(i) = Eigen::Vector3d(0, 0.8, 0);
                }
            }
        }
    


        void console_find_largest_manifold(App* app, const std::vector<std::string> & args) 
        {
            //igl::facet_components()
        }

        void console_uniform(App* app, const std::vector<std::string> & args) 
        {
            app->SMOTION.resize(2);
            int part_id = 0;
            int num_frames = app->ANIM.VD.size();
            app->SMOTION[part_id].anim.VD.resize(num_frames);
            int uniform = num_frames/15;

            VectorXd error;
            error.resize(num_frames);
            int cur_idx = -1;
            for (int i=0; i<app->ANIM.VD.size(); ++i) 
            {   
                if (i%25 == 0) 
                {   
                     cur_idx = i;
                }
                app->SMOTION[part_id].anim.VD[i] = app->PIECES[part_id].VD[cur_idx];
            }
        }

        void console_split_mesh(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Splitting mesh");
            
            if (app->vid_list.size() == 0 || app->DANIM.VD.size() == 0) 
            {   
                RA_LOG_INFO("Not enough information");
                return;
            }

            if (args.size() != 1) 
            {
                RA_LOG_INFO("Need type of loop to cut");
                return;
            }
            
            /*
            if (!igl::is_edge_manifold(app->DANIM.F)) 
            {   
                Eigen::MatrixXi P;
                igl::extract_manifold_patches(app->DANIM.F, P);
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
            */
            MatrixXi eid_list;
            VectorXi EI, uEI, DMAP;
            bool isClosedSeam = std::stoi(args[0]);
            convert_vertex_to_edge_selection(app->vid_list, 
            app->vid_list_second,app->vid_list_third,
                                             app->DANIM.E, 
                                             app->DANIM.UE, 
                                             app->DANIM.EMAP,
                                             isClosedSeam,
                                             eid_list, 
                                             EI, 
                                             uEI, 
                                             DMAP);
        

            MatrixXi F1, F2;
            VectorXi F1idx, F2idx;
            tyro::mesh_split(app->DANIM.F,
                             uEI,
                             DMAP, 
                             F1idx, 
                             F2idx);


            igl::slice(app->DANIM.F, F1idx, 1, F1); 
            igl::slice(app->DANIM.F, F2idx, 1, F2); 

            
            app->PIECES.resize(2);
            auto& A1 = app->PIECES[0];
            auto& A2 = app->PIECES[1];
            A1.VD.resize(app->DANIM.VD.size());
            A2.VD.resize(app->DANIM.VD.size());
            A1.ND.resize(app->DANIM.VD.size());
            A2.ND.resize(app->DANIM.VD.size());
            
            A1.SIdx = app->ANIM.SIdx;
            A2.SIdx = app->ANIM.SIdx;
            
            for (int i = 0; i < app->ANIM.VD.size(); ++i) 
            {                   
                MatrixXi I1, I2;    
                igl::remove_unreferenced(app->DANIM.VD[i], 
                                         F1, 
                                         A1.VD[i], 
                                         A1.F, 
                                         I1);

                igl::per_vertex_normals(A1.VD[i], A1.F, A1.ND[i]);
                std::vector<std::vector<int> > uE2E1;
                igl::unique_edge_map(A1.F,A1.E,A1.UE,A1.EMAP,uE2E1);
                tyro::color_matrix(A1.F.rows(), Eigen::Vector3d(0.2,0.2,0.2), A1.FC);
                tyro::color_black_matrix(A1.UE.rows(), A1.UEC);

                igl::remove_unreferenced(app->DANIM.VD[i], 
                                         F2, 
                                         A2.VD[i], 
                                         A2.F, 
                                         I2);
                
                igl::per_vertex_normals(A2.VD[i], A2.F, A2.ND[i]);
                std::vector<std::vector<int> > uE2E2;
                igl::unique_edge_map(A2.F, A2.E, A2.UE, A2.EMAP,uE2E2);
                tyro::color_matrix(A2.F.rows(), Eigen::Vector3d(0.6,0.6,0.6), A2.FC);
                tyro::color_black_matrix(A2.E.rows(), A2.UEC);
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
                
                if (app->SMOTION.empty())
                    app->SMOTION.resize(app->PIECES.size());

                int i = part_id;
                double result_energy;
                auto& sm = app->SMOTION[i]; 
                auto& piece = app->PIECES[i];
                bool isKmeans = false;
                
                if (initmethod == "kmeans") 
                    isKmeans = true; 
                else if (initmethod == "random")
                    isKmeans = false;
                else 
                    return;

                if (piece.VW.size() == 0) 
                {
                    piece.VW.resize(app->PIECES[i].VD[0].rows());
                    piece.VW.setOnes();
                    RA_LOG_INFO("WEIGHTS ARE EMPTY, SETTING TO ZERO");
                }                

                Eigen::VectorXd VW = piece.VW.cwiseSqrt();

                tyro::stop_motion_vertex_distance(num_labels, 
                                                  smooth_weight,
                                                  isKmeans,
                                                  piece.VD, 
                                                  VW, 
                                                  app->ANIM.SIdx,
                                                  piece.F,
                                                  sm.D, //dictionary
                                                  sm.L, //labels  
                                                  result_energy);
                
                sm.anim.F = piece.F;
                sm.anim.E = piece.E;
                sm.anim.UE = piece.UE;
                sm.anim.EMAP = piece.EMAP;
                sm.anim.UEC = piece.UEC;
                sm.anim.FC = piece.FC;
                //precompute normals
                std::vector<MatrixXd> normals;
                normals.resize(sm.D.size());
                for (int j = 0; j < sm.D.size(); ++j) 
                {   
                    igl::per_vertex_normals(sm.D[j], sm.anim.F, normals[j]);
                }

                sm.anim.VD.clear();
                sm.anim.ND.clear();
                sm.anim.VD.resize(piece.VD.size());
                sm.anim.ND.resize(piece.ND.size());

                int run_idx =0;
                for (int j = 0; j < sm.L.size(); ++j) 
                {
                    int l_idx = sm.L(j);
                    sm.anim.VD[run_idx] =sm.D[l_idx];
                    sm.anim.ND[run_idx] = normals[l_idx];
                    run_idx++;
                }
                sm.computed = true;                   
                

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
                igl::slice(app->ANIM.F, slice_list, 1, newF);
                int start_frame = 0;

                //check that the slice is manifold
                //if (!igl::is_edge_manifold(newF)) 
                if (false)
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
                    for (int file = 0; file < app->ANIM.SIdx.size(); ++file) 
                    {   
                        auto folder = filesystem::path(app->FOLDERS[file])/wheretosave;

                        if (!folder.exists()) 
                        {
                            filesystem::create_directory(folder);
                        }
                        
                        auto objlist_path = folder/filesystem::path("objlist.txt");
                        int num_frames = app->ANIM.SIdx[file];
                        
                        ofstream objlist_file;
                        objlist_file.open (objlist_path.str());
                    
                        for (int frame = start_frame; frame < start_frame + num_frames; ++frame) 
                        {   
                            assert(frame < app->ANIM.VD.size());
                            MatrixXd temp_V;
                            MatrixXi temp_F;
                            VectorXi I;
                            igl::remove_unreferenced(app->ANIM.VD[frame], newF, temp_V, temp_F, I);
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
                    to_save.VD.resize(app->ANIM.VD.size());
                    to_save.ND.resize(app->ANIM.VD.size());
                    for (int frame = 0; frame < app->ANIM.VD.size(); ++frame) 
                    {   
                        MatrixXi temp_F;
                        VectorXi I;
                        igl::remove_unreferenced(app->ANIM.VD[frame], newF, to_save.VD[frame], temp_F, I);
                        igl::per_vertex_normals(to_save.VD[frame], temp_F, to_save.ND[frame]);
                        if (frame == 0) 
                        {
                            to_save.F = temp_F;
                        }
                    }  
                
                    std::vector<std::vector<int> > uE2E;
                    igl::unique_edge_map(to_save.F, 
                                         to_save.E,
                                         to_save.UE,
                                         to_save.EMAP,
                                         uE2E);
                    
                    //@TODO need this to update camera
                    Eigen::Vector3d face_color(0.5,0.5,0.5);
                    tyro::color_matrix(to_save.F.rows(), face_color, to_save.FC);
                    tyro::color_black_matrix(to_save.UE.rows(), to_save.UEC);

                    to_save.SIdx = app->ANIM.SIdx;

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

            if (args.size() == 2)
            {   
                auto filename = args[0];
                bool append = std::stoi(args[1]);

                RA_LOG_INFO("Load selected faces");
                auto path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/") + filename;

                if (append)
                    app->fid_list.clear();
                
                std::fstream myfile(path, std::ios_base::in);
                int fid;
                while (myfile >> fid)
                {
                    //printf("%i ", fid);
                    
                    auto it = std::find(app->fid_list.begin(), app->fid_list.end(), fid);
                    if (it == app->fid_list.end()) 
                    {
                        app->add_face(fid);
                        app->setFaceColor(fid, true);
                    }              
                }
                app->render();
                glfwPostEmptyEvent();        
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


            if (args.size() == 2)
            {   
                auto filename = args[0];
                bool append = std::stoi(args[1]);
                RA_LOG_INFO("Load selected verticies");
                auto path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/") + filename;

                if (append)
                    app->vid_list.clear();
                
                std::fstream myfile(path, std::ios_base::in);
                int vid;
                while (myfile >> vid)
                {
                    //printf("%i ", vid);
                    app->add_vertex(vid);
                    app->addSphere(vid, app->ANIM.VD[app->m_frame]);
                }
                app->render();
                glfwPostEmptyEvent();        
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

        void console_add_adjaent_boundary(App* app, const std::vector<std::string> & args)  
        {   
            std::vector<std::vector<int>> A;
            igl::adjacency_list(app->ANIM.F, A);

            std::vector<int> vcopy;
            for (auto k : app->vid_list) vcopy.push_back(k);
            for (auto vid : vcopy) 
            {
                auto & loop = A[vid];
                for (auto vid2 : loop) 
                {   
                    auto it = std::find(app->vid_list.begin(), app->vid_list.end(), vid2);
                    if (it == app->vid_list.end()) 
                    {
                        app->addSphere(vid2, app->ANIM.VD[app->m_frame]);
                    
                        app->add_vertex(vid2);
                    }
                }
            }
        }
        
       
        void console_fix_boundary_edges(App* app, const std::vector<std::string> & args) 
        {   
            std::vector<std::vector<int>> L;
            igl::boundary_loop(app->ANIM.F, L);

            for (int i = 0; i < L.size(); ++i) 
            {
                auto & loop = L[i];
                for (auto vid : loop)
                {               
                    auto it = std::find(app->vid_list.begin(), app->vid_list.end(), vid);
                    if (it == app->vid_list.end()) 
                    {
                        app->addSphere(vid, app->ANIM.VD[app->m_frame], Wm5::Vector4f(0,1,0,1));                        
                        app->add_vertex2(vid);
                    }
                }
            }
        }



        void console_load_agent(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("load agent");
            bool serialized = true;
            if (args.size() == 1) 
                serialized = std::stoi(args[0]);

            //Picked face_id 26093 vertex_id 13681 coord 11.950990 2.934150 16.501955
            //face_id 40217 vertex_id 10585 coord -1.194749 -0.823966 1.658947
            //Picked face_id 31779 vertex_id 12996 coord 0.012255 1.557947 0.744930
            int offset_vid = 638; // 1222;
            auto offset = Eigen::Vector3d(-0.000000, 1.379708, 0.009046) ; //Eigen::Vector3d(0.613322, 2.613381, 2.238946);
            
            app->FOLDERS = 
            {   
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/agent327/obj/04"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/agent327/obj/07"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/agent327/obj/08_05"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/agent327/obj/10_03"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/agent327/obj/11_02"),
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/agent327/obj/14_01")
            };

            if (serialized) 
            {
                auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
                //auto p = f/filesystem::path("agent_manifold");
                //auto p = f/filesystem::path("2018_agent_split_up"); 
               
                auto p = f/filesystem::path("2018_agent_frames_vertical"); 
               
                std::ifstream in = std::ifstream(p.str(), std::ios::binary);
                cereal::BinaryInputArchive archive_i(in);
                archive_i(app->ANIM);
            }
            else
            {   std::vector<std::string> obj_paths;
                for (int i = 0; i < app->FOLDERS.size(); ++i) 
                {
                    int num_files_read;
                    //Add smth
                    auto folder = app->FOLDERS[i];
                    //folder += std::string("face/");

                    RA_LOG_INFO("loading folder %s", folder.data());
                    tyro::obj_file_path_list(folder, "objlist.txt", obj_paths, num_files_read);
                    RA_LOG_INFO("frames read %i", num_files_read);
                    app->ANIM.SIdx.push_back(num_files_read);
                }
                app->load_mesh_sequence(obj_paths, false); //use IGL obj loader
            }
 
            if (!igl::is_edge_manifold(app->ANIM.F)) 
            {   
                RA_LOG_ERROR_ASSERT("not manifold");
                exit(0);
            }
            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);

            app->align_all_models(offset_vid, offset);
                
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = 0.7*2*bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            //app->m_weights.VW.resize(app->ANIM.VD[0].rows());
            //app->m_weights.VW.setOnes();

            //app->m_weights.FW.resize(app->ANIM.F.rows());
            //app->m_weights.FW.setOnes();
            
            app->compute_average();
            app->m_ball_size = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F)/5;

            std::vector<std::string> args1 = {"split", "2018_agent_split"};
            console_load_serialised_data(app,args1);

            std::vector<std::string> args2 = {"stop_low", "monka_stop_100_1_1_kmeans"};
            //console_load_serialised_data(app, args2);
            
            std::vector<std::string> args3 = {"stop_up", "monka_stop_50_1_0_kmeans"};
            //console_load_serialised_data(app, args3);

            app->render();
        }

                
        void console_load_bartender(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("load bart");
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
                std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/osipa/obj/01"),
            };

            if (serialized) 
            {
                auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
                //auto p = f/filesystem::path("bartender_ver_split_front_nodup");
                
                auto p = f/filesystem::path("new_bart");
                //auto p = f/filesystem::path("bart_frames_lily");
                
                
                std::ifstream in = std::ifstream(p.str(), std::ios::binary);
                cereal::BinaryInputArchive archive_i(in);
                archive_i(app->ANIM);
            }
            else
            {   
                std::vector<std::string> obj_paths;

                for (int i = 0; i < app->FOLDERS.size(); ++i) 
                {
                    int num_files_read;

                    //Add smth
                    auto folder = app->FOLDERS[i];
                    //folder += std::string("face/");

                    RA_LOG_INFO("loading folder %s", folder.data());
                    tyro::obj_file_path_list(folder, "objlist.txt", obj_paths, num_files_read);
                    RA_LOG_INFO("frames read %i", num_files_read);
                    app->ANIM.SIdx.push_back(num_files_read);
                }
                
                app->load_mesh_sequence(obj_paths, false); //use IGL obj loader

            }

            
            if (!igl::is_edge_manifold(app->ANIM.F)) 
            {   
                RA_LOG_ERROR_ASSERT("not manifold");
                exit(0);
            }
            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);
            
            bool shouldscale = true;
            double S;
            Eigen::RowVector3d tr;
            
            if (shouldscale)
                scale_one(app->ANIM, S, tr);
            //app->align_all_models(offset_vid, offset);
                
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = 0.9*2*bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
           
            //app->m_weights.VW.resize(app->ANIM.VD[0].rows());
            //app->m_weights.VW.setOnes();

            //app->m_weights.FW.resize(app->ANIM.F.rows());
            //app->m_weights.FW.setOnes();
            
            app->compute_average();
            app->m_ball_size = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F)/5;
            
           
            
            std::vector<std::string> args1 = {"deform", "new_bart_deform"};
            console_load_serialised_data(app, args1);

            std::vector<std::string> args2 = {"split", "new_bart_split"};
            console_load_serialised_data(app, args2);

            std::vector<std::string> args3 = {"stop_low", "bar_20_1_0"};
            //console_load_serialised_data(app, args3);
            
            std::vector<std::string> args4 = {"stop_up", "bar_12_1_1"};
            //console_load_serialised_data(app, args4);

             if (shouldscale) 
            {
                if (!app->PIECES.empty()) 
                {
                    scale_one_exe(app->PIECES[1], S, tr);
                }
            }

            if (shouldscale) 
            {
                if (!app->SMOTION.empty()) 
                {
                    //scale_one_exe(app->SMOTION[0].anim, S, tr);
                    //scale_one_exe(app->SMOTION[1].anim, S, tr);
                }
            }

            auto clr = Eigen::Vector3d(255/255.0, 148/255.0, 135/255.0);
            auto clr1 = Eigen::Vector3d(255/255.0, 255/255.0, 255/255.0);
            
            auto clr2 = Eigen::Vector3d(180/255.0, 100/255.0, 179/255.0);
            auto clr3 = Eigen::Vector3d(120/255.0, 151/255.0, 200/255.0);

            tyro::color_matrix(app->ANIM.F.rows(), clr, app->ANIM.FC);
            if (app->PIECES.size() > 0) {
                tyro::color_matrix(app->PIECES[0].F.rows(), clr2, app->PIECES[0].FC);
                tyro::color_matrix(app->PIECES[1].F.rows(), clr3, app->PIECES[1].FC);
            }
            
            if (app->SMOTION.size()>0)
            {
                tyro::color_matrix(app->SMOTION[0].anim.F.rows(), clr2, app->SMOTION[0].anim.FC);
                tyro::color_matrix(app->SMOTION[1].anim.F.rows(), clr3, app->SMOTION[1].anim.FC);
            }
            
            app->render();
        }

        void console_load_monka_print(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("load monka print");
         
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            //auto p = f/filesystem::path("monkey_sub_anim");
            //auto p = f/filesystem::path("monkey_temp_anim");
            auto p = f/filesystem::path("monka_split_sub_anim");
            
            std::ifstream in = std::ifstream(p.str(), std::ios::binary);
            cereal::BinaryInputArchive archive_i(in);
            archive_i(app->ANIM);
            
            
            if (!igl::is_edge_manifold(app->ANIM.F)) 
            {   
                RA_LOG_ERROR_ASSERT("monkey NOT MANIFOLD");
                exit(0);
            }
            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);

            bool shouldscale = true;
            double S;
            Eigen::RowVector3d tr;
            if (shouldscale)
                scale_one(app->ANIM, S, tr);
            //app->align_all_models(offset_vid, offset);
                
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = 0.8*2*bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            app->compute_average();
            app->m_ball_size = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F)/5;

            //app->m_weights.VW.resize(app->ANIM.VD[0].rows());
            //app->m_weights.VW.setOnes();

            //app->m_weights.FW.resize(app->ANIM.F.rows());
            //app->m_weights.FW.setOnes();
            
            app->compute_average();
            
            std::vector<std::string> args1 = {"split", "monka_split_sub_split"};
            console_load_serialised_data(app,args1);
            if (shouldscale)  
            {
               if (!app->PIECES.empty()) 
               {
                    scale_one_exe(app->PIECES[0], S, tr);
                    scale_one_exe(app->PIECES[1], S, tr);
               }
            }
            std::vector<std::string> args2 = {"deform",   "monka_split_sub_deform"};
            console_load_serialised_data(app, args2);
            if (shouldscale)  
            {
               if (!app->DANIM.VD.empty()) 
               {
                    scale_one_exe(app->DANIM, S, tr);
               }
            }
            std::vector<std::string> args3 = {"stop_up",  "monka_10_1_1_kmeans"};
            //console_load_serialised_data(app, args3);
            
            std::vector<std::string> args4 = {"stop_low", "monka_30_1_0_kmeans"};
            //console_load_serialised_data(app, args4);
            if (shouldscale) 
            {
               if (!app->SMOTION.empty()) 
               {
                    scale_one_exe(app->SMOTION[0].anim, S, tr);
                    scale_one_exe(app->SMOTION[1].anim, S, tr);
               }
            }
            app->ShowMonkaMovieStop();
            
            auto clr = Eigen::Vector3d(255/255.0, 148/255.0, 135/255.0);
//            auto clr1 = Eigen::Vector3d(255/255.0, 255/255.0, 255/255.0);
            
            auto clr2 = Eigen::Vector3d(180/255.0, 100/255.0, 179/255.0);
            auto clr3 = Eigen::Vector3d(120/255.0, 151/255.0, 200/255.0);

            app->ANIM.FC.setZero();
            tyro::color_matrix(app->ANIM.F.rows(), clr, app->ANIM.FC);
            tyro::color_matrix(app->PIECES[0].F.rows(), clr2, app->PIECES[0].FC);
            tyro::color_matrix(app->PIECES[1].F.rows(), clr3, app->PIECES[1].FC);
            
            if (app->SMOTION.size()>0)
            {
                tyro::color_matrix(app->SMOTION[0].anim.F.rows(), clr2, app->SMOTION[0].anim.FC);
                tyro::color_matrix(app->SMOTION[1].anim.F.rows(), clr3, app->SMOTION[1].anim.FC);
            }

            app->render();
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
                //auto p = f/filesystem::path("monka_frames_nodup_smooth");
                //auto p = f/filesystem::path("2018_monkey_split_up");
                //auto p = f/filesystem::path("monkey_new_frames");
                //auto p = f/filesystem::path("monka_smooth_front");
                auto p = f/filesystem::path("monka_split_frames");
                
                std::ifstream in = std::ifstream(p.str(), std::ios::binary);
                cereal::BinaryInputArchive archive_i(in);
                archive_i(app->ANIM);            
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
                    app->ANIM.SIdx.push_back(num_files_read);
                }
                
                app->load_mesh_sequence(obj_paths, true); //use IGL obj loader
            }

            
            if (!igl::is_edge_manifold(app->ANIM.F)) 
            {   
                RA_LOG_ERROR_ASSERT("monkey NOT MANIFOLD");
                exit(0);
            }
            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);

            //app->align_all_models(offset_vid, offset);
                
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
                    
            app->m_model_offset = 0.8*2*bbox.GetRadius(); 
            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            app->compute_average();
            app->m_ball_size = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F)/5;

            //app->m_weights.VW.resize(app->ANIM.VD[0].rows());
            //app->m_weights.VW.setOnes();

            //app->m_weights.FW.resize(app->ANIM.F.rows());
            //app->m_weights.FW.setOnes();
            
            app->compute_average();
            
            std::vector<std::string> args1 = {"split", "monka_split_split"};
            console_load_serialised_data(app,args1);

            std::vector<std::string> args2 = {"deform", "monka_split_deform"};
            console_load_serialised_data(app, args2);
            
            std::vector<std::string> args3 = {"stop_up", "monkey_stop_12_1_1_kmeans"};
            //console_load_serialised_data(app, args3);
            
//            app->ShowMonkaMovieStop();
            //auto clr = Eigen::Vector3d(255/255.0, 148/255.0, 135/255.0);
            //auto clr2 = Eigen::Vector3d(180/255.0, 100/255.0, 179/255.0);

            //tyro::color_matrix(app->ANIM.F.rows(), clr, app->ANIM.FC);
            

            auto clr = Eigen::Vector3d(255/255.0, 148/255.0, 135/255.0);
//            auto clr1 = Eigen::Vector3d(255/255.0, 255/255.0, 255/255.0);
            
            auto clr2 = Eigen::Vector3d(180/255.0, 100/255.0, 179/255.0);
            auto clr3 = Eigen::Vector3d(120/255.0, 151/255.0, 200/255.0);

            app->ANIM.FC.setZero();
            tyro::color_matrix(app->ANIM.F.rows(), clr, app->ANIM.FC);
            if (app->PIECES.size()>0) 
            {
            tyro::color_matrix(app->PIECES[0].F.rows(), clr2, app->PIECES[0].FC);
            tyro::color_matrix(app->PIECES[1].F.rows(), clr3, app->PIECES[1].FC);
            }
            if (app->SMOTION.size()>0)
            {
                tyro::color_matrix(app->SMOTION[0].anim.F.rows(), clr2, app->SMOTION[0].anim.FC);
                tyro::color_matrix(app->SMOTION[1].anim.F.rows(), clr3, app->SMOTION[1].anim.FC);
            }
            
            app->render();


        }

        void console_load_bunny(App* app, const std::vector<std::string> & args) 
        {   
            RA_LOG_INFO("load bunny serialized");
            int offset_vid = 1030; // 1222;
            auto offset = Eigen::Vector3d(0.268563, 3.142050, 2.504273) ; //Eigen::Vector3d(0.613322, 2.613381, 2.238946);
        
            app->FOLDERS = 
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

            bool serialized = true;
            if (serialized) 
            {   
                auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
                //auto p = f/filesystem::path("rabbit_ver_split_low");
                //auto p = f/filesystem::path("rabbit2");
                //auto p = f/filesystem::path("rabbit_ver_split_up");
                //auto p = f/filesystem::path("2018_bunny_frames");
                //auto p = f/filesystem::path("rabbit_hor_frames");            
                //auto p = f/filesystem::path("bunny_frames_upsampled");
                //auto p = f/filesystem::path("bunny_split_up_horizontal_no_dup");
                //auto p = f/filesystem::path("bunny_frames_upsampled_scaled");
                //auto p = f/filesystem::path("bunny_scaled_small_frames");
                //auto p = f/filesystem::path("subanimaton");
                //auto p = f/filesystem::path("rabbit_split_up_ver");
                //auto p = f/filesystem::path("rabbit_frames_hor");
                auto p = f/filesystem::path("bunny_frames_fuck");


                std::ifstream in = std::ifstream(p.str(), std::ios::binary);
                cereal::BinaryInputArchive archive_i(in);
                archive_i(app->ANIM);
                tyro::color_matrix(app->ANIM.UE.rows(), Eigen::Vector3d(0,0,0), app->ANIM.UEC);
            }
            else
            {   
                std::vector<std::string> obj_paths;
                for (auto folder : app->FOLDERS) 
                {
                    int num_files_read;

                    //Add smth
                    //folder += std::string("no_mouth/");
                    
                    RA_LOG_INFO("loading folder %s", folder.data());
                    tyro::obj_file_path_list(folder, "objlist2.txt", obj_paths, num_files_read);
                    RA_LOG_INFO("frames read %i", num_files_read);
                    app->ANIM.SIdx.push_back(num_files_read);
                }
                app->load_mesh_sequence(obj_paths, true); //use IGL obj loader
            }
            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);

            //align_all_models(offset_vid, offset);
            if(!igl::is_edge_manifold(app->ANIM.F)) 
            {
                RA_LOG_ERROR("Mesh is not edge manifold");
                exit(0);
            }

                
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = 2*bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            
            app->ANIM.VW.resize(app->ANIM.VD[0].rows());
            app->ANIM.VW.setOnes();

            app->compute_average();
            app->m_ball_size = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F)/5;

            //compute ambient_occlusion
        // Eigen::VectorXd AO;
        /*
            for (int i = 0; i < 0; ++i) 
            {               
                app->ANIM.AO.resize(ANIM.VD.size());
                igl::ambient_occlusion(ANIM.VD[i], 
                                    ANIM.F, 
                                    ANIM.VD[i], 
                                    ANIM.ND[i], 
                                    1000, 
                                    ANIM.AO[3114]);
                RA_LOG_INFO("Computed AO for frame %i", i);
                //RA_LOG_INFO("AO %f, %f",ANIM.AO[339].minCoeff(), ANIM.AO[339].maxCoeff());
            }
            */
            //std::vector<std::string> args2 = {"split", "2018_bunny_split_vertical"};

            //std::vector<std::string> args2 = {"split", "rabbit_hor_split"};
            std::vector<std::string> args1 = {"split", "bunny_split_fuck"};
            console_load_serialised_data(app, args1);


            std::vector<std::string> args2 = { "deform", "bunny_deform_fuck"};
            console_load_serialised_data(app, args2);
            

            std::vector<std::string> args3 = {"stop_low", "bunny_170_1_0_kmeans"};
            //console_load_serialised_data(this, args3);
            
            std::vector<std::string> args4 = {"stop_up", "bunny_80_1_1_"};
            //console_load_serialised_data(this, args4);
            

            //addAnim1.FC
            app->ParseImages();
            
            app->render();
        }
        

        void console_load_blobby(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("Loading blobby ");
            bool serialized = true;
            
            //Picked face_id 26093 vertex_id 13681 coord 11.950990 2.934150 16.501955
            //face_id 40217 vertex_id 10585 coord -1.194749 -0.823966 1.658947
            //Picked face_id 31779 vertex_id 12996 coord 0.012255 1.557947 0.744930
            int offset_vid = 12996; // 1222;
            auto offset = Eigen::Vector3d(0.012255, 1.557947, 0.744930) ; //Eigen::Vector3d(0.613322, 2.613381, 2.238946);
        
            app->FOLDERS 
            = 
            {   
                std::string("/home/rinat/GDrive/StopMotionProject/Claymation/data/hello_merged/NewFramesOBJ/20")
            };

            if (serialized) 
            {
                auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
                auto p = f/filesystem::path("bloby_frames");
                std::ifstream in = std::ifstream(p.str(), std::ios::binary);
                cereal::BinaryInputArchive archive_i(in);
                archive_i(app->ANIM);
            }
            else
            {
                std::vector<std::string> obj_paths;
                for (int i = 0; i < app->FOLDERS.size(); ++i) 
                {
                    int num_files_read;

                    //Add smth
                    auto folder = app->FOLDERS[i];
                    //folder += std::string("face/");

                    RA_LOG_INFO("loading folder %s", folder.data());
                    tyro::obj_file_path_list(folder, "objlist.txt", obj_paths, num_files_read);
                    RA_LOG_INFO("frames read %i", num_files_read);
                    app->ANIM.SIdx.push_back(num_files_read);
                }
                app->load_mesh_sequence(obj_paths, true); //use IGL obj loader

            }

            
            if (!igl::is_edge_manifold(app->ANIM.F)) 
            {   
                RA_LOG_ERROR_ASSERT("blobby not manifold");
                exit(0);    
            }
            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);
            app->isBLOBBY=false;
            //app->align_all_models(offset_vid, offset);
            
            double S;
            Eigen::RowVector3d tr;
            bool shouldscale = false;
            if (shouldscale)
                scale_one(app->ANIM, S, tr);   
            //Compute radius of the bounding box of the model
            
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = 0.7*2*bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            
            //app->m_weights.VW.resize(app->ANIM.VD[0].rows());
            //app->m_weights.VW.setOnes();

            //app->m_weights.FW.resize(app->ANIM.F.rows());
            //app->m_weights.FW.setOnes();
            
            app->compute_average();
            app->m_ball_size = igl::avg_edge_length(app->ANIM.AvgVD, app->ANIM.F)/5;

            std::vector<std::string> args1 = {"split", "blobby_split"};
            console_load_serialised_data(app, args1);

            if (shouldscale) 
            {
                if (!app->PIECES.empty()) 
                {
                    scale_one_exe(app->PIECES[0], S, tr);
                    scale_one_exe(app->PIECES[1], S, tr);
                }
            }
            std::vector<std::string> args2 = {"stop_low", "monka_stop_100_1_1_kmeans"};
            console_load_serialised_data(app, args2);
            
            std::vector<std::string> args3 = {"stop_up", "blobby_stop_20_1_1_kmeans"};
            console_load_serialised_data(app, args3);

            app->SMOTION[0].anim.VD.resize(app->ANIM.VD.size());
            app->SMOTION[0].anim.ND.resize(app->ANIM.ND.size());
            for (int i = 0; i < app->ANIM.VD.size(); ++i) 
            {
                app->SMOTION[0].anim.VD[i] = app->PIECES[0].VD[0];
                app->SMOTION[0].anim.ND[i] = app->PIECES[0].ND[0];
            }
            app->SMOTION[0].anim.F = app->PIECES[0].F;
            app->SMOTION[0].anim.FC = app->PIECES[0].FC;
            app->SMOTION[0].computed = true;
            

            if (shouldscale) 
            {
                if (!app->SMOTION.empty()) 
                {
                    scale_one_exe(app->SMOTION[0].anim, S, tr);
                    scale_one_exe(app->SMOTION[1].anim, S, tr);
                }
            } 
            auto clr = Eigen::Vector3d(255/255.0, 148/255.0, 135/255.0);
            auto clr1 = Eigen::Vector3d(255/255.0, 255/255.0, 255/255.0);
            
            auto clr2 = Eigen::Vector3d(180/255.0, 100/255.0, 179/255.0);

            tyro::color_matrix(app->ANIM.F.rows(), clr, app->ANIM.FC);
            
            if (app->PIECES.size()>0)
            {
                tyro::color_matrix(app->PIECES[0].F.rows(), clr, app->PIECES[0].FC);
                tyro::color_matrix(app->PIECES[1].F.rows(), clr2, app->PIECES[1].FC);
              
            }

             auto clr3 = Eigen::Vector3d(120/255.0, 151/255.0, 200/255.0);
            if (app->SMOTION.size()>0)
            {
                tyro::color_matrix(app->SMOTION[0].anim.F.rows(), clr3, app->SMOTION[0].anim.FC);
                tyro::color_matrix(app->SMOTION[1].anim.F.rows(), clr2, app->SMOTION[1].anim.FC);
            }
            app->render();
        }

        void console_compute_average(App* app, const std::vector<std::string> & args) 
        {
            RA_LOG_INFO("compute_average");
            app->compute_average();
        }

        void console_compute_deformation2(App* app, const std::vector<std::string> & args)
        {
            RA_LOG_INFO("computing deformation");

            if (app->vid_list.empty() || !app->m_computed_avg) 
            { 
                RA_LOG_WARN("Need vertex/face selection and average mesh to compute deformation")
                return;
            }

            if (!igl::is_edge_manifold(app->ANIM.F)) 
            { 
                RA_LOG_ERROR("NOTE EDGE MANIFOLD FOR DEFORM");
                return;
            }

            //app->DANIM.VD.clear();
          
            bool result = tyro::compute_deformation2(app->vid_list,
                                                     app->vid_list2, 
                                                     app->ANIM.VD,
                                                     app->ANIM.F,
                                                     app->ANIM.AvgVD,
                                                     app->DANIM.VD, 
                                                     app->Lap);
            assert(result);
            tyro::copy_animation(app->ANIM, app->DANIM, true, true, true);
            app->m_computed_deformation = true;
            app->m_update_camera = true;
            //app->render();
        }

        

        void console_compute_deformation(App* app, const std::vector<std::string> & args)
        {
            RA_LOG_INFO("computing deformation");

            if (app->vid_list.empty() || app->fid_list.empty() || !app->m_computed_avg) 
            { 
                RA_LOG_WARN("Need vertex/face selection and average mesh to compute deformation")
                return;
            }
            //app->DANIM.VD.clear();
            bool result = tyro::compute_deformation(app->vid_list, 
                                                    app->fid_list,
                                                    app->ANIM.VD,
                                                    app->ANIM.F,
                                                    app->ANIM.AvgVD,
                                                    app->DANIM.VD);
            assert(result);
            tyro::copy_animation(app->ANIM, app->DANIM, true, true, true);
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
    m_show_wire(true),
    add_seg_faces(false),
    m_video_texture(nullptr),
    m_frame_offset(0)
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

    using  std::string;
    string type2str(int type) 
    {
        string r;

        uchar depth = type & CV_MAT_DEPTH_MASK;
        uchar chans = 1 + (type >> CV_CN_SHIFT);

        switch ( depth ) {
            case CV_8U:  r = "8U"; break;
            case CV_8S:  r = "8S"; break;
            case CV_16U: r = "16U"; break;
            case CV_16S: r = "16S"; break;
            case CV_32S: r = "32S"; break;
            case CV_32F: r = "32F"; break;
            case CV_64F: r = "64F"; break;
            default:     r = "User"; break;
        }

        r += "C";
        r += (chans+'0');

        return r;
    }

    int App::VideoToImages() 
    {   
        using namespace cv;

        // Create a VideoCapture object and open the input file
        // If the input is the web camera, pass 0 instead of the video file name
        VideoCapture cap("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/movies/monka.mp4"); 
            
        // Check if camera opened successfully
        if(!cap.isOpened())
        {
            cout << "Error opening video stream or file" << endl;
            return -1;
        }
        
        int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        int num_frames = cap.get(CV_CAP_PROP_FRAME_COUNT);
        int frame = 0;
        int fcount = 0;
        while(1)
        {   

            cv::Mat frame_image;
            // Capture frame-by-frame
            cap >> frame_image;
            //cv::Mat frame_image_rgb;
            //cv::Mat frame_image;

            //cv::cvtColor(frame_image_grb, frame_image_rgb, CV_BGR2RGB);
            //cv::flip(frame_image_rgb, frame_image, 0);

            //auto tp = type2str(frame_image.type());
            // If the frame is empty, break immediately
            if (frame_image.empty())
                break;
        
            // Display the resulting frame
            //imshow( "Frame", frame );
            string folder = "images";
            
            if (frame%400 == 0) 
            {
                fcount++;
            }
            folder = folder + std::to_string(fcount);
            auto a = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/images");
            a = a/folder;
            if (!a.exists())
                filesystem::create_directory(a);
            auto final_p = a/filesystem::path(tyro::pad_zeros(frame, width = 7) + string(".jpeg"));
            imwrite(final_p.str(), frame_image);
            frame++;

        }
    }

    int App::Video() 
    {   
        using namespace cv;

         //setup windowOdpes
        m_tyro_window = new Window();
        m_tyro_window->Init();
                
        //setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
 
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(Wm5::APoint(0,0,0), 1.0, 1.0, 2, viewport, true);

        // Create a VideoCapture object and open the input file
        // If the input is the web camera, pass 0 instead of the video file name
        VideoCapture cap("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/Big Buck Bunny-720p.mp4"); 
            
        // Check if camera opened successfully
        if(!cap.isOpened())
        {
            cout << "Error opening video stream or file" << endl;
            return -1;
        }
        
        int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        int num_frames = cap.get(CV_CAP_PROP_FRAME_COUNT);
        cap.set(CV_CAP_PROP_CONVERT_RGB, 1);

        m_video_texture = ES2VideoTexture::Create(width, height, num_frames, Texture::TextureFormat::TF_R8G8B8);
        int frame = 0;
        //m_tyro_window->GetGLContext()->swapBuffers();

        m_gl_rend->SetClearColor(Wm5::Vector4f(0.0, 0, 0.5, 1.0));

        FontManager* fManager = FontManager::GetSingleton();
        float scale = 1;
        float ppi = 144;
        fManager->Setup(ppi, scale);

        ES2FontSPtr font = FontManager::GetSingleton()->GetSystemFontOfSize12();
        std::string strrr("Frame 0/9000");
        m_frame_overlay = ES2TextOverlay::Create(strrr, 
                                                 Wm5::Vector2f(0, 0), 
                                                 font, 
                                                 Wm5::Vector4f(0,0,1,1), 
                                                 viewport);
        m_frame_overlay->SetTranslate(Wm5::Vector2i(-viewport[2]/2 + 50,-viewport[3]/2 + 50));
   
        while(!m_tyro_window->ShouldClose())
        {   

            cv::Mat frame_image_grb;
            // Capture frame-by-frame
            cap >> frame_image_grb;
            cv::Mat frame_image_rgb;
            cv::Mat frame_image;

            cv::cvtColor(frame_image_grb, frame_image_rgb, CV_BGR2RGB);
            cv::flip(frame_image_rgb, frame_image, 0);

            auto tp = type2str(frame_image.type());
            // If the frame is empty, break immediately
            if (frame_image.empty())
                break;
        
            // Display the resulting frame
            //imshow( "Frame", frame );
            
            m_video_texture->showFrame(frame, frame_image);
            VisibleSet vis_set;
            vis_set.Insert(m_video_texture.get());
            vis_set.Insert(m_frame_overlay.get());
            m_gl_rend->RenderVisibleSet(&vis_set, m_camera);         
        
            RA_LOG_INFO("Drawn frame %i out of %i", frame, num_frames);
            frame++;
            
            m_tyro_window->GetGLContext()->swapBuffers();
            m_tyro_window->ProcessUserEvents();
        }

        
        
        // When everything done, release the video capture object
        cap.release();
        
        // Closes all the frames
        //destroyAllWindows();
            
        return 0;
    }

    int App::ShowMonkaMovieStop() 
    {   
        using namespace cv;
               
        int beginning = 632; //159 - 148;
        m_frame = beginning;
        int fileAnchor = 378;
        int openglAnchor = 426;
        //int offset = openglAnchor - fileAnchor;
        int offset = 0;

        bool serialised = true;
        if (serialised) 
        {
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/subvideo_frames_monka");
            std::ifstream in = std::ifstream(f.str(), std::ios::binary);
            cereal::BinaryInputArchive archive_i(in);
            archive_i(m_video);
        }
        else 
        {               
            int num_frames = 696; //TODO DEPENDS ON MOVIE 

            m_video.F.resize(num_frames);
            for (int f = 0; f < num_frames; ++f) 
            {
                m_video.F[f] = cv::Mat::zeros( cv::Size(1920, 1080), CV_8UC3 );
            }
        }

        int num_frames = m_video.F.size();
        m_video_texture = ES2VideoTexture::Create(1920, 1080, num_frames, Texture::TextureFormat::TF_R8G8B8); 

        std::vector<std::string> FOLDERS__ = 
        {   
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/images/monka/01"),
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/images/monka/02"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monkaa/images/monka/03")      
        };
    } 

    
    int App::PRINTEDStopAnimMovie() 
    {   
        using namespace cv;
        /*
        if (serialised) 
        {
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/subvideo_frames");
            std::ifstream in = std::ifstream(f.str(), std::ios::binary);
            cereal::BinaryInputArchive archive_i(in);
            archive_i(m_video);
        }
        */
       
        std::vector<std::string> obj_paths;
        int num_labels;
        tyro::obj_file_path_list(std::string("/home/rinat/Workspace/STOPMO"), "objlist.txt", obj_paths, num_labels);
        stop_video_cv.D.resize(num_labels);
        
        int num_frames = SMOTION[0].anim.VD.size();
        //m_video_texture2 = ES2VideoTexture::Create(2144, 1424, num_frames, Texture::TextureFormat::TF_R8G8B8); 

        int f_count = 0;
        for (auto& file : obj_paths) 
        {
            Mat LoadedImage, image2, image3;
            LoadedImage = cv::imread(file);
            //cv::cvtColor(LoadedImage, image2, CV_BGR2RGB);
            //cv::flip(image2, image3, 0);
            stop_video_cv.D[f_count] = LoadedImage;
            f_count++;
        }
        
        //create dic 
        std::ifstream in("/home/rinat/DIC.csv");
        std::string str;
        std::unordered_map<std::string, int> wordMap;
        
        int file_idx = 0;
        while (std::getline(in, str)) 
        {
            std::cout << str << std::endl;
            wordMap.insert({str,file_idx++});
        }

        std::ifstream fin("/home/rinat/FRAMES.csv");
        int frame = 0;
        stop_video_cv.F.resize(num_frames);
        while (std::getline(fin, str)) 
        {   
            int dic_idx = wordMap[str];
            
            auto fldr = filesystem::path("/home/rinat/GDrive/StopMotionProject/Results/stop");
            fldr = fldr/(std::to_string(frame++) + std::string(".JPG"));
     
            ifstream source(obj_paths[dic_idx], ios::binary);
            ofstream dest(fldr.str(), ios::binary);
            dest << source.rdbuf();

            source.close();
            dest.close();

            //stop_video_cv.F[frame++] = stop_video_cv.D[dic_idx];
            // cv::Mat::zeros(cv::Size(2144, 1424), CV_8UC3);
        }

        //////////////////////////////////////////////////
        /*
        cv::Size S = cv::Size(2144, 1424);

        cv::VideoWriter outputVideo;                                        // Open the output
        outputVideo.open(string("/home/rinat/GDrive/StopMotionProject/Results/") +                      string("fuck.avi"),
                            CV_FOURCC('M','J','P','G'), 
                            24,
                            S,
                            true);
        
        if (!outputVideo.isOpened())
        {
            cout  << "Could not open the output video for write" << endl;
            return 0;
        }

        for (int frame = 0; frame < stop_video_cv.F.size(); ++frame) 
        {   
            //cv::Mat image1(v_height, v_width, CV_8UC4, texture);
            //cv::Mat image2, image3;
            //cv::cvtColor(image1, image2, CV_RGBA2BGR);
            //cv::flip(image2, image3, 0);

            outputVideo.write(stop_video_cv.F[frame]);
            cout << "Saved frame " << frame  << endl;

        }
        
        cout << "Finished writing" << endl;
       

        for (int frame = 0; frame < stop_video_cv.F.size(); ++frame) 
        {   
            //cv::Mat image1(v_height, v_width, CV_8UC4, texture);
            //cv::Mat image2, image3;
            //cv::cvtColor(image1, image2, CV_RGBA2BGR);
            //cv::flip(image2, image3, 0);

           
            //outputVideo.write(stop_video_cv.F[frame]);
            auto fldr = filesystem::path("/home/rinat/GDrive/StopMotionProject/Results/stop");
            fldr = fldr/(std::to_string(frame) + std::string(".JPG"));

     
     
            ifstream source(obj_paths[frame], ios::binary);
            ofstream dest(fldr.str(), ios::binary);
            dest << source.rdbuf();

            source.close();
            dest.close();

        } */
           
    }
    
    int App::ShowBunnyMovieStop2() 
    {   
        using namespace cv;
        auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/bunny_sub_video2");
        std::ifstream in = std::ifstream(f.str(), std::ios::binary);
        cereal::BinaryInputArchive archive_i(in);
        archive_i(m_video);     
        int num_frames = m_video.F.size();
        m_video_texture = ES2VideoTexture::Create(1280, 720, num_frames, Texture::TextureFormat::TF_R8G8B8); 
    }

    int App::ShowBunnyMovieStop() 
    {   
        using namespace cv;
        auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/subvideo_frames");
        std::ifstream in = std::ifstream(f.str(), std::ios::binary);
        cereal::BinaryInputArchive archive_i(in);
        archive_i(m_video);
       int num_frames = m_video.F.size();
        m_video_texture = ES2VideoTexture::Create(1280, 720, num_frames, Texture::TextureFormat::TF_R8G8B8); 
    }


    int App::ParseImages() 
    {   
        using namespace cv;
        
       
        int beginning = 0;
        m_frame = beginning;
        int fileAnchor = 2666;
        int openglAnchor = 2699;
        int offset = -fileAnchor + openglAnchor ;
        offset = 0;

        bool serialised = true;
        if (serialised) 
        {
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj/video_frames");
            std::ifstream in = std::ifstream(f.str(), std::ios::binary);
            cereal::BinaryInputArchive archive_i(in);
            archive_i(m_video);
        }
        else 
        {               
            int num_frames = 5177; //TODO DEPENDS ON MOVIE 

            m_video.F.resize(num_frames);
            for (int f = 0; f < num_frames; ++f) 
            {
                m_video.F[f] = cv::Mat::zeros( cv::Size(1280, 720), CV_8UC3 );
            }
        }
        int num_frames = m_video.F.size();
        m_video_texture = ES2VideoTexture::Create(1280, 720, num_frames, Texture::TextureFormat::TF_R8G8B8); 

        std::vector<std::string> FOLDERS__ = 
        {   
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/02/01/"),
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/02/02/"),
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/02/03/"),
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/02/04/"),
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/02/05/"),
            
            
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/03/01/"),
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/03/02/"),
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/03/03/"),
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/03/05/"),            
            
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/03/06/"),

                        
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/04/01/"),
            
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/04/03/"),
            
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/05/01/"),
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/05/03/"),
            
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/05/05_07_09/"),
            /*
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/05/07/"),
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/05/09/"),            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/05/11/"),
            
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/07/01/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/07/02/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/07/05/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/07/06/"),


            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/01/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/02B/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/03/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/05/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/07/"),    
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/07B/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/09/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/10/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/11/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/13/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/14/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/16/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/18/"),            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/08/19/"),
            
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/09/05/"),

            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/11/10/"),

            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/12/09/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/12/21/"),

            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/13/03/"),
            std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/rinat/13/04/")
            */
        };
/*
        std::vector<std::string> obj_paths;
        for (auto& folder : FOLDERS__) 
        {
            int num_files_read;
            RA_LOG_INFO("loading folder %s", folder.data());
            tyro::obj_file_path_list(folder, "objlist.txt", obj_paths, num_files_read);
            RA_LOG_INFO("frames read %i", num_files_read);
        }       
        
        int f_count = beginning;
        for (auto& file : obj_paths) 
        {
            Mat LoadedImage, image2, image3;
            LoadedImage = cv::imread(file);
            cv::cvtColor(LoadedImage, image2, CV_BGR2RGB);
            cv::flip(image2, image3, 0);
            m_video.F[f_count + offset] = image3;
            f_count++;
        }
        */
    }

    int App::Launch()
    {   
        RA_LOG_INFO("Launching the app");

        //setup windowshapes
        m_tyro_window = new Window();
        m_tyro_window->Init();
                
        //setup renderer
        m_gl_rend = new ES2Renderer(m_tyro_window->GetGLContext());
        //m_gl_rend->SetClearColor(Wm5::Vector4f(0.0, 153.0/255.0, 153.0/255.0, 1.0));
        m_gl_rend->SetClearColor(Wm5::Vector4f(1, 1, 1, 1));

        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        m_camera = new iOSCamera(Wm5::APoint(0,0,0), 1.0, 1.0, 2, viewport, true);
        
        m_timeline = new Timeline(40, 300);
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
        
        register_console_function("load_jali", console_load_jali, "");
        register_console_function("save_avg", console_save_deform_average, "");
        register_console_function("write_labels", console_write_labels, "");
        register_console_function("compute_stop_frames", console_compute_stop_frames, "");
        register_console_function("uniform", console_uniform, "");
        register_console_function("save_few_deform_frames", console_save_few_deform_frames, "");
        register_console_function("save_first_frame", console_save_first_frame, "");
        register_console_function("compute_alec", console_compute_alec,"");

        register_console_function("compute_random_swith", console_figure_random, "");
        register_console_function("compute_data_vel_error", console_compute_data_vel_error,"");
        register_console_function("compute_errors", console_compute_errors,"");
        register_console_function("plot_error_deform", console_plot_error_deform, "");
        register_console_function("clear_weights", console_clear_weights, "");
        register_console_function("segmentation_full", console_segmentation_full, "");
        register_console_function("print_stats", console_print_stats, "");
        register_console_function("set_selection_type", console_set_selection_type, "");
        register_console_function("mesh_split_final", console_mesh_split_final, "");
        register_console_function("compute_deformation2", console_compute_deformation2, "");
        register_console_function("seam_from_selection", console_seam_from_selection, "");
        register_console_function("add_adjaent_boundary", console_add_adjaent_boundary, "");
        register_console_function("fix_boundary_edges", console_fix_boundary_edges, "");
        register_console_function("remove_duplicate_vertices", console_remove_duplicate_vertices, "");
        register_console_function("show_boundary_faces", console_show_boundary_faces, "");
        register_console_function("find_boundary", console_find_boundary, "");
        register_console_function("mesh_split_bndr", console_mesh_split_bndr, "");
        register_console_function("compute_vertex_bndr", console_compute_vertex_bndr, "");
        register_console_function("load_agent", console_load_agent, "");
        register_console_function("load_bartender", console_load_bartender, "");
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
        //register_console_function("split_mesh", console_split_mesh, "");
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
        register_console_function("load_monka_print", console_load_monka_print, "");
        
        register_console_function("set_vertex_weight", console_set_vertex_weight, "");
        register_console_function("set_face_weight", console_set_face_weight, "");
        
        register_console_function("draw_vertex_weight_map", console_draw_vertex_weight_map, "");
        register_console_function("draw_face_weight_map", console_draw_face_weight_map, "");
        register_console_function("upsample", console_upsample, "");
        register_console_function("taubin_smooth_along_edges", console_taubin_smooth_along_edges, "");
        register_console_function("laplacian_smooth_vert", console_laplacian_smoothing_vert, "");

        register_console_function("laplacian_smooth_along_edges", console_laplacian_smooth_along_edges, "");
        register_console_function("console_offset_frame", console_offset_frame, "");
        register_console_function("plot_error", console_plot_error, "");
        register_console_function("plot_error_vel", console_plot_error_vel, "");

        register_console_function("show_seam", console_show_seam, ""); 
        register_console_function("render_to_image", console_render_to_image, ""); 
        register_console_function("render_to_video", console_render_to_video, ""); 

        register_console_function("save_sub_split", console_save_sub_split, "");
        register_console_function("save_sub_animation", console_save_sub_animation, "");
        register_console_function("save_sub_deformation", console_save_sub_deformation, "");

        register_console_function("save_for_printing", console_save_for_printing, "");
        register_console_function("save_sub_video", console_save_sub_video, "");
        register_console_function("load_bunny_stop", console_load_bunny_stop, "");
        register_console_function("load_bunny_stop2", console_load_bunny_stop2, "");

        register_console_function("show_iso", console_show_iso, "");
        register_console_function("show_iso_2", console_show_iso_2, "");

        register_console_function("show_label", console_show_labels, "");
        register_console_function("scale", console_scale, "");
        register_console_function("collapse_small_triangles", console_collapse_small_triangles, "");


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
        FontManager* fManager = FontManager::GetSingleton();
        float scale = 1;
        float ppi = 144;
        fManager->Setup(ppi, scale);

        ES2FontSPtr font = FontManager::GetSingleton()->GetSystemFontOfSize12();
        std::string strrr("Frame 0/9000");
        m_frame_overlay = ES2TextOverlay::Create(strrr, 
                                                 Wm5::Vector2f(0, 0), 
                                                 font, 
                                                 Wm5::Vector4f(1,1,1,1), 
                                                 viewport);
        m_frame_overlay->SetTranslate(Wm5::Vector2i(-viewport[2]/2 + 50,-viewport[3]/2 + 50));
        
        //ParseImages();

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
        ///render any video first
        //play video
        {   
            /*
            if (m_video_texture == nullptr)
                m_video_texture = ES2VideoTexture::Create("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/Big Buck Bunny-720p.mp4");
            
            m_video_texture->showFrame(m_frame_offset + m_frame);
            vis_set.Insert(m_video_texture.get());
            */

            if (m_video.F.size() > 0 && m_video_texture != nullptr) 
            {
                m_video_texture->showFrame(m_frame, m_video.F[m_frame]);
                vis_set.Insert(m_video_texture.get());
            }
        }     

        #if 1
        //ORIGNAL MODEL
        if (RENDER.mesh == nullptr) 
            RENDER.mesh = IGLMesh::Create(ANIM.VD[m_frame], ANIM.F, ANIM.ND[m_frame], ANIM.FC); //ANIM.AO[3114]);
        else 
            RENDER.mesh->UpdateData(ANIM.VD[m_frame], ANIM.F, ANIM.ND[m_frame], ANIM.FC);

        RENDER.mesh->Update(true);
        vis_set.Insert(RENDER.mesh.get());
        
        if (m_show_wire)
        {   
            if (RENDER.mesh_wire == nullptr)
                RENDER.mesh_wire = IGLMeshWireframe::Create(ANIM.VD[m_frame], ANIM.UE, ANIM.UEC);
            else
                RENDER.mesh_wire->UpdateData(ANIM.VD[m_frame], ANIM.UE, ANIM.UEC);
            RENDER.mesh_wire->Update(true);
            vis_set.Insert(RENDER.mesh_wire.get());
        }

        if (m_eid_list2.size()>0) 
        {
            RENDER.seam_on_main = IGLMeshWireframe::Create(SMOTION[0].anim.VD[m_frame], SMOTION[0].anim.UE, SMOTION[0].anim.UEC, m_eid_list2);
            RENDER.seam_on_main->Update(true);
            vis_set.Insert(RENDER.seam_on_main.get());
        }

        if (m_computed_avg) 
        {   
            Wm5::Transform tr;
            tr.SetTranslate(Wm5::APoint(-4*m_model_offset, 0, 0));
            if (RENDER.avg == nullptr) 
            {
                RENDER.avg = IGLMesh::Create(ANIM.AvgVD, ANIM.F, ANIM.ND[0], Eigen::Vector3d(102/255.0, 153/255.0, 255/255.0));
                RENDER.avg->LocalTransform = tr * RENDER.avg->LocalTransform;
                RENDER.avg->Update(true);                            
            }
            else  
            {
                RENDER.avg->UpdateData(ANIM.AvgVD, ANIM.F, ANIM.ND[0], Eigen::Vector3d(102/255.0, 153/255.0, 255/255.0));
            }

           // vis_set.Insert(RENDER.avg.get());

            //create renderable for mesh wireframe
            if (m_show_wire)
            {
                if (RENDER.avg_wire == nullptr) 
                {
                    RENDER.avg_wire = IGLMeshWireframe::Create(ANIM.AvgVD, ANIM.UE, ANIM.UEC);
                    RENDER.avg_wire->LocalTransform = tr * RENDER.avg_wire->LocalTransform;
                    RENDER.avg_wire->Update(true);
                }
                else 
                    RENDER.avg_wire->UpdateData(ANIM.AvgVD, ANIM.UE, ANIM.UEC);
                
                //vis_set.Insert(RENDER.avg_wire.get());
            }                    
        }

        //DEFORMED MODEL
        if (m_computed_deformation)
        {   
            Wm5::Transform tr;
            tr.SetTranslate(Wm5::APoint(-2*m_model_offset, 0, 0));
                                    
            if (RENDER.dfm == nullptr) 
            {
                RENDER.dfm = IGLMesh::Create(DANIM.VD[m_frame], DANIM.F, ANIM.ND[m_frame], Eigen::Vector3d(0.5,0.5,0.5));
                RENDER.dfm->LocalTransform = tr * RENDER.dfm->LocalTransform;
                RENDER.dfm->Update(true);
            }
            else 
            {
                RENDER.dfm->UpdateData(DANIM.VD[m_frame], DANIM.F, ANIM.ND[m_frame], Eigen::Vector3d(0.5,0.5,0.5));                            
            }
            vis_set.Insert(RENDER.dfm.get());

            if (m_show_wire)
            {
                if (RENDER.dfm_wire==nullptr) 
                {
                    RENDER.dfm_wire = IGLMeshWireframe::Create(DANIM.VD[m_frame], DANIM.UE, ANIM.UEC);
                    RENDER.dfm_wire->LocalTransform = tr * RENDER.dfm_wire->LocalTransform;
                    RENDER.dfm_wire->Update(true);      
                }
                else 
                    RENDER.dfm_wire->UpdateData(DANIM.VD[m_frame], DANIM.UE, ANIM.UEC);
                                                
                vis_set.Insert(RENDER.dfm_wire.get());
            }
        }

        //Split Mesh
        if (m_computed_parts)
        {
            //Rendering stuff
            if (RENDER.part.size() == 0) 
            {
                RENDER.part = std::vector<IGLMeshSPtr>(PIECES.size(), nullptr);
                RENDER.part_wire = std::vector<IGLMeshWireframeSPtr>(PIECES.size(), nullptr);
            }

            std::random_device rd;     // only used once to initialise (seed) engine
            std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
            std::uniform_int_distribution<int> uni(0, ANIM.VD.size()-1); // guaranteed unbiased

    		int k = uni(rng);
    
            for (int i = 0; i < PIECES.size(); ++i) 
            {    
                Wm5::Transform tr;
                tr.SetTranslate(Wm5::APoint(-1*m_model_offset, 0  , 0));    


                if (RENDER.part[i] == nullptr) 
                {   if (false) 
                    {
                        RENDER.part[i] = IGLMesh::Create(PIECES[i].VD[0], PIECES[i].F, PIECES[i].ND[0], PIECES[i].FC);
                        RENDER.part[i]->LocalTransform = tr * RENDER.part[i]->LocalTransform;
                        RENDER.part[i]->Update(true);
                    }
                    else 
                    {
                        RENDER.part[i] = IGLMesh::Create(PIECES[i].VD[m_frame], PIECES[i].F, PIECES[i].ND[m_frame], PIECES[i].FC);
                        RENDER.part[i]->LocalTransform = tr * RENDER.part[i]->LocalTransform;
                        RENDER.part[i]->Update(true);
                    }
                }
                else
                {
                    //if (i == 0) 
                    //{   
                    //    if (m_frame%5 == 0)
                    k = m_frame;
                    if (false)
                        RENDER.part[i]->UpdateData(PIECES[i].VD[0], PIECES[i].F, PIECES[i].ND[0], PIECES[i].FC);
                    else
                        RENDER.part[i]->UpdateData(PIECES[i].VD[k], PIECES[i].F, PIECES[i].ND[k], PIECES[i].FC); 
                    //}
                }
                vis_set.Insert(RENDER.part[i].get());

                //create renderable for mesh wireframe
                if (m_show_wire)
                {   
                    if (RENDER.part_wire[i] == nullptr) 
                    {
                        RENDER.part_wire[i] = IGLMeshWireframe::Create(PIECES[i].VD[m_frame], PIECES[i].UE, PIECES[i].UEC);
                        RENDER.part_wire[i]->LocalTransform = tr * RENDER.part_wire[i]->LocalTransform;                                                        
                        RENDER.part_wire[i]->Update(true);
                    }
                    else 
                        RENDER.part_wire[i]->UpdateData(PIECES[i].VD[m_frame], PIECES[i].UE, PIECES[i].UEC);

                    vis_set.Insert(RENDER.part_wire[i].get());
                }
            } 
        }

        //Stop motion
        if (m_computed_stop_motion) 
        {   
            if (RENDER.stop.size() == 0) 
            {
                RENDER.stop = std::vector<IGLMeshSPtr>(SMOTION.size(), nullptr);
                RENDER.stop_wire = std::vector<IGLMeshWireframeSPtr>(SMOTION.size(), nullptr);
            }

            for (int i = 0; i < SMOTION.size(); ++i) 
            {   
                if (SMOTION[i].computed == false) 
                    continue;
                
                Wm5::Transform tr;
                tr.SetTranslate(Wm5::APoint(1*m_model_offset, 0, 0));

                if (RENDER.stop[i] == nullptr) 
                { 
                    RENDER.stop[i] = IGLMesh::Create(SMOTION[i].anim.VD[m_frame], 
                                                     SMOTION[i].anim.F, 
                                                     SMOTION[i].anim.ND[m_frame],
                                                     SMOTION[i].anim.FC); 
                    RENDER.stop[i]->LocalTransform = tr * RENDER.stop[i]->LocalTransform;
                    RENDER.stop[i]->Update(true);
                }
                else 
                {
                    RENDER.stop[i]->UpdateData(SMOTION[i].anim.VD[m_frame], 
                                               SMOTION[i].anim.F, 
                                               SMOTION[i].anim.ND[m_frame],
                                               SMOTION[i].anim.FC); 
                } 

                vis_set.Insert(RENDER.stop[i].get());

                //create renderable for mesh wireframe
                if (m_show_wire)
                {   
                    if (RENDER.stop_wire[i] == nullptr) 
                    {
                        RENDER.stop_wire[i]  = IGLMeshWireframe::Create(SMOTION[i].anim.VD[m_frame], 
                                                                        SMOTION[i].anim.UE,
                                                                        SMOTION[i].anim.UEC);
                        RENDER.stop_wire[i]->LocalTransform = tr * RENDER.stop_wire[i]->LocalTransform;                                                        
                        RENDER.stop_wire[i]->Update(true);
                    } 
                    else 
                    {
                        RENDER.stop_wire[i]->UpdateData(SMOTION[i].anim.VD[m_frame], 
                                                        SMOTION[i].anim.UE,
                                                        SMOTION[i].anim.UEC);
                    }

                    vis_set.Insert(RENDER.stop_wire[i].get());
                }

                if (m_eid_list.size() > 0) //should replace wiht boolen flag 
                {   
                    //if (i == 1)
                    //{
                        RENDER.seam[i] = IGLMeshWireframe::Create(SMOTION[i].anim.VD[m_frame], SMOTION[i].anim.UE, SMOTION[i].anim.UEC, m_eid_list[i]);
                        RENDER.seam[i]->LocalTransform = tr * RENDER.seam[i]->LocalTransform;                                                        
                        RENDER.seam[i]->Update(true);
                        vis_set.Insert(RENDER.seam[i].get());
                    //}
                }
            }
        }

        if (m_error_deform.size() > 0) 
        {
            Eigen::Vector3f maxColor = Eigen::Vector3f(255/255.0, 0/255.0, 0/255.0);
            Wm5::Transform tr;
            tr.SetTranslate(Wm5::APoint(-2*m_model_offset, m_model_offset, 0));

            Eigen::MatrixXd jetC;
            Eigen::VectorXd values = m_error_deform.row(m_frame);
            igl::jet(values, true, jetC);
            //jetC.setOnes();
            if (RENDER.errorDeform == nullptr) 
            {   
                RENDER.errorDeform = IGLMesh::Create(DANIM.VD[m_frame], 
                                                     DANIM.F, 
                                                     ANIM.ND[m_frame],  
                                                     jetC, 
                                                     true);                
                RENDER.errorDeform->LocalTransform = tr * RENDER.errorDeform->LocalTransform;
                RENDER.errorDeform->Update(true);
            }
            else 
            { 
                RENDER.errorDeform->UpdateData(DANIM.VD[m_frame], 
                                               DANIM.F, 
                                               ANIM.ND[m_frame],  
                                               jetC, 
                                               true);
            }

            vis_set.Insert(RENDER.errorDeform.get());
            
        }

        if (m_computed_error) 
        {
                //Rendering stuff
            if (RENDER.error.size() == 0) 
            {
                RENDER.error = std::vector<IGLMeshSPtr>(PIECES.size(), nullptr);
                // RENDER.part_wire = std::vector<IGLMeshWireframeSPtr>(PIECES.size(), nullptr);
            }

            for (int i = 0; i < RENDER.error.size(); ++i) 
            {   
                if (SMOTION[i].computed == false) continue;
                
                Eigen::Vector3f maxColor = Eigen::Vector3f(255/255.0, 0/255.0, 0/255.0);

                Wm5::Transform tr;
                //tr2.SetUniformScale(0.6);
                tr.SetTranslate(Wm5::APoint(2*m_model_offset, -0.6*m_model_offset, 0));
                if (RENDER.error[i] == nullptr) 
                {
                    RENDER.error[i] = IGLMesh::CreateColor(SMOTION[i].anim.VD[m_frame], 
                                                           SMOTION[i].anim.F, 
                                                           SMOTION[i].anim.ND[m_frame],  
                                                           m_error[i].row(m_frame), 
                                                           max_error, 
                                                           maxColor);
                    
                    RENDER.error[i]->LocalTransform = tr * RENDER.error[i]->LocalTransform;
                    RENDER.error[i]->Update(true);
                }
                else 
                { 
                    RENDER.error[i]->UpdateData(SMOTION[i].anim.VD[m_frame], 
                                                SMOTION[i].anim.F, 
                                                SMOTION[i].anim.ND[m_frame], 
                                                m_error[i].row(m_frame), 
                                                max_error,
                                                maxColor);
                }

                vis_set.Insert(RENDER.error[i].get());
            } 
        }

        if (m_computed_vel_error) 
        {
            //Rendering stuff
            if (RENDER.errorVel.size() == 0) 
            {
                RENDER.errorVel = std::vector<IGLMeshSPtr>(PIECES.size(), nullptr);
                // RENDER.part_wire = std::vector<IGLMeshWireframeSPtr>(PIECES.size(), nullptr);
            }

            for (int i = 0; i < RENDER.errorVel.size(); ++i) 
            {   
                if (SMOTION[i].computed == false) continue;
                Eigen::Vector3f maxColor = Eigen::Vector3f(255.0/255.0, 0/255.0, 0.0/255.0);
                Wm5::Transform tr;
                //tr.SetUniformScale(0.6);
                tr.SetTranslate(Wm5::APoint(2*m_model_offset, 0.6*m_model_offset, 0));
                if (RENDER.errorVel[i] == nullptr) 
                {
                    
                    RENDER.errorVel[i] = IGLMesh::CreateColor(SMOTION[i].anim.VD[m_frame], 
                                                              SMOTION[i].anim.F, 
                                                              SMOTION[i].anim.ND[m_frame],  
                                                              m_error_velocity[i].row(m_frame), 
                                                              //8.924972,
                                                              max_vel_error, 
                                                              maxColor);
                    
                    RENDER.errorVel[i]->LocalTransform = tr * RENDER.errorVel[i]->LocalTransform;
                    RENDER.errorVel[i]->Update(true);
                }
                else 
                { 
                    RENDER.errorVel[i]->UpdateData(SMOTION[i].anim.VD[m_frame], 
                                                   SMOTION[i].anim.F, 
                                                   SMOTION[i].anim.ND[m_frame],  
                                                   m_error_velocity[i].row(m_frame), 
                                                   //8.924972,
                                                   max_vel_error,
                                                   maxColor);
                }

                vis_set.Insert(RENDER.errorVel[i].get());
            }

        }
        
        if (m_computed_iso_color) 
        {
            Eigen::Vector3f maxColor = Eigen::Vector3f(255/255.0, 0/255.0, 0/255.0);

            Wm5::Transform tr;
            tr.SetTranslate(Wm5::APoint(3*m_model_offset, -m_model_offset/2, 0));
            if (RENDER.isoline == nullptr) 
            {
                RENDER.isoline = IGLMesh::CreateColor(ANIM.AvgVD, ANIM.F, ANIM.ND[0], ISOCOLORS, 1, maxColor);
                
                RENDER.isoline->LocalTransform = tr * RENDER.isoline->LocalTransform;
                RENDER.isoline->Update(true);
            }
            vis_set.Insert(RENDER.isoline.get());
        } 

        if (m_computed_iso_color) 
        {
            Wm5::Transform tr;
            tr.SetTranslate(Wm5::APoint(3*m_model_offset, m_model_offset/2, 0));
            if (RENDER.isolineSplit == nullptr) 
            {
                RENDER.isolineSplit = IGLMesh::Create(VSPLIT, FSPLIT, NSPLIT, FCSPLIT);
                RENDER.isolineSplit->LocalTransform = tr * RENDER.isolineSplit->LocalTransform;
                RENDER.isolineSplit->Update(true);

                RENDER.isolineSplitWire = IGLMeshWireframe::Create(VSPLIT, UESPLIT, UCSPLIT);
                RENDER.isolineSplitWire->LocalTransform = tr * RENDER.isolineSplitWire->LocalTransform;                                                        
                RENDER.isolineSplitWire->Update(true);
            }
            vis_set.Insert(RENDER.isolineSplit.get());
            vis_set.Insert(RENDER.isolineSplitWire.get());

        } 

        //Split Mesh
        if (addAnim1.VD.size()>0)
        {
            Wm5::Transform tr;
            tr.SetTranslate(Wm5::APoint(1*m_model_offset, 0, 0));

            if (RENDER.addShape1 == nullptr) 
            {
                RENDER.addShape1 = IGLMesh::Create(addAnim1.VD[0], addAnim1.F, addAnim1.ND[0], addAnim1.FC);
                RENDER.addShape1->LocalTransform = tr * RENDER.addShape1->LocalTransform;
                RENDER.addShape1->Update(true);

                //RENDER.addShape2 = IGLMesh::Create(addAnim1.VD[0], addAnim1.F, addAnim1.ND[0], addAnim1.FC);
                //RENDER.addShape2->LocalTransform = tr * RENDER.addShape2->LocalTransform;
                //RENDER.addShape2->Update(true);
            }
            else 
            {
                RENDER.addShape1->UpdateData(addAnim1.VD[0], addAnim1.F, addAnim1.ND[0], addAnim1.FC);
                //RENDER.addShape2->UpdateData(addAnim1.VD[0], addAnim1.F, addAnim1.ND[0], addAnim1.FC);
            }

            vis_set.Insert(RENDER.addShape1.get());
            vis_set.Insert(RENDER.addShape2.get());
        }

        for (auto object_sptr : ball_list) 
        {
            vis_set.Insert(object_sptr.get());
        }
        #endif

        //auto glambda = [](auto FILES, auto&& b) { return a < b; };
        int start_frame = 0;
        int scene_id; 

        for (int file = 0; file < ANIM.SIdx.size(); ++file) 
        {   
            int num_frames = ANIM.SIdx[file];
            start_frame += num_frames;
            if (m_frame < start_frame) 
            { 
                scene_id = file;
                break;
            }
        } 

        std::string fstr = std::string("Frame ") + std::to_string(m_frame) + std::string("/") + std::to_string(ANIM.VD.size());
        fstr =fstr+ std::string(" Scene ") + std::to_string(scene_id);
        m_frame_overlay->SetText(fstr);
        vis_set.Insert(m_frame_overlay.get());

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

    void App::invert_face_selection() 
    {
        std::vector<int> newlist;
        for (int fid = 0; fid < ANIM.F.rows(); ++fid) 
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
            add_face(fid);
            setFaceColor(fid, true);

        }
        render();
        glfwPostEmptyEvent();
    }
    
    

    void App::compute_average()
    {
        if (ANIM.VD.size()==0)
            RA_LOG_ERROR_ASSERT("cant compute average");
        
        ANIM.AvgVD.resize(ANIM.VD[0].rows(), 
                                       ANIM.VD[0].cols());
        ANIM.AvgVD.setZero();

        for (auto& mat : ANIM.VD) 
        {
            ANIM.AvgVD += mat;
        }
        ANIM.AvgVD = (1.0/ANIM.VD.size()) * ANIM.AvgVD;

        m_computed_avg = true;
        m_update_camera = true;
    }

    void App::update_camera() 
    {
        //setup camera
        AxisAlignedBBox WorldBoundBox = RENDER.mesh->WorldBoundBox;

        if (m_computed_avg) 
        {
           // WorldBoundBox.Merge(RENDER.avg->WorldBoundBox);
        }

        if (m_computed_deformation) 
        {
            WorldBoundBox.Merge(RENDER.dfm->WorldBoundBox);
        }

        if (m_computed_parts) 
        {
            for (auto& mesh : RENDER.part)
                if (mesh)
                    WorldBoundBox.Merge(mesh->WorldBoundBox);
        }

        if (m_computed_stop_motion) 
        {   
            for (int i=0; i < SMOTION.size(); ++i)
            {
                if (SMOTION[i].computed && RENDER.stop[i] != nullptr) 
                {
                    WorldBoundBox.Merge(RENDER.stop[i]->WorldBoundBox);
                }
            }
        }

        Wm5::APoint world_center = WorldBoundBox.GetCenter();
        float radius = std::abs(WorldBoundBox.GetRadius()*1.5);
        int v_width, v_height;
        m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
        Wm5::Vector4i viewport(0, 0, v_width, v_height);
        float aspect = (float)v_width/v_height;
        
        if (m_camera)
            delete m_camera;
        
        m_camera = new iOSCamera(world_center, radius, aspect, 2, viewport, true);
    }

    void App::addSphere(int vid, const Eigen::MatrixXd& V, Wm5::Vector4f color, Wm5::Transform world) 
    {   
        Eigen::RowVector3d new_c = V.row(vid);
        //float s =  (V.size() > 3000) ? 0.0005 : 0.007;
        //float s = 0.001;
        ES2SphereSPtr object = ES2Sphere::Create(10, 10, m_ball_size);
        Wm5::Transform tr;
        tr.SetTranslate(Wm5::APoint(new_c(0), new_c(1), new_c(2)));
        object->LocalTransform = world * tr * object->LocalTransform;
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
        ANIM.FC.row(fid) = clr;
    }

    void App::setFaceColor(int fid, bool selected) 
    {
        Eigen::Vector3d clr;
        if (selected) 
            clr = Eigen::Vector3d(0, 0, 0.5);
        else
            clr = Eigen::Vector3d(0.5,0.5,0.5);
        
        ANIM.FC.row(fid) = clr;
    }
   
    void App::load_mesh_sequence(const std::vector<std::string>& obj_list, bool use_igl_loader) 
    {   
        RA_LOG_INFO("LOAD MESH SEQUENCE")
        
        tyro::load_mesh_sequence(obj_list, 
                                 ANIM.VD, 
                                 ANIM.ND, 
                                 ANIM.F,
                                 ANIM.E,
                                 ANIM.UE,
                                 ANIM.EMAP,
                                 use_igl_loader);
        
        //@TODO need this to update camera
        tyro::color_matrix(ANIM.F.rows(), Eigen::Vector3d(0.5,0.5,0.5), ANIM.FC);
        tyro::color_matrix(ANIM.UE.rows(), Eigen::Vector3d(0.2,0.2,0.2), ANIM.UEC);
    }

    //load oldman (will you go to lunch)
    void App::load_oldman() 
    {   
        bool serialized = true;
        RA_LOG_INFO("load oldman");
        int offset_vid = 1030; // 1222;
        auto offset = Eigen::Vector3d(0.268563, 3.142050, 2.504273) ; //Eigen::Vector3d(0.613322, 2.613381, 2.238946);
       
        FOLDERS = 
        {   
            //std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj_export/just_face")
            std::string("/home/rinat/GDrive/StopMotionProject/Claymation/data/oldman/gotolunch/FramesOBJ/FullFace"),
        };

        if (serialized) 
        {   
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            //auto p = f/filesystem::path("oldman_frames_final");
            auto p = f/filesystem::path("2018_oldman_beck_split_low");
            
            std::ifstream in = std::ifstream(p.str(), std::ios::binary);
            cereal::BinaryInputArchive archive_i(in);
            archive_i(ANIM);
            tyro::color_matrix(ANIM.UE.rows(), Eigen::Vector3d(0,0,0), ANIM.UEC);
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
                tyro::obj_file_path_list(folder, "objlist.txt", obj_paths, num_files_read);
                RA_LOG_INFO("frames read %i", num_files_read);
                ANIM.SIdx.push_back(num_files_read);
            }
            load_mesh_sequence(obj_paths, true); //use IGL obj loader
        }
        m_timeline->SetFrameRange(ANIM.VD.size()-1);

        //align_all_models(offset_vid, offset);
        if(!igl::is_edge_manifold(ANIM.F)) 
        {
            RA_LOG_ERROR("Mesh is not edge manifold");
            //exit(0);
        }
        bool shouldscale = true;
        double S;
        Eigen::RowVector3d tr;
        if  (shouldscale)
            scale_one(ANIM, S, tr);
        //compute_average();
               
        //Compute radius of the bounding box of the model
        AxisAlignedBBox bbox;
        MatrixXd VT = ANIM.VD[0].transpose();
        bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
        m_model_offset = 0.7*2*bbox.GetRadius(); 


        m_update_camera = true;
        m_state = App::State::LoadedModel;
        
        ANIM.VW.resize(ANIM.VD[0].rows());
        ANIM.VW.setOnes();
        //m_weights.VW.resize(ANIM.VD[0].rows());
        //m_weights.VW.setOnes();

        //m_weights.FW.resize(ANIM.F.rows());
        //m_weights.FW.setOnes();

        compute_average();
        m_ball_size = igl::avg_edge_length(ANIM.AvgVD, ANIM.F)/5;
        
        //compute ambient_occlusion
       // Eigen::VectorXd AO;
        for (int i = 0; i < 0; ++i) 
        {               
            ANIM.AO.resize(ANIM.VD.size());
            igl::ambient_occlusion(ANIM.VD[i], 
                                   ANIM.F, 
                                   ANIM.VD[i], 
                                   ANIM.ND[i], 
                                   1000, 
                                   ANIM.AO[3114]);
            RA_LOG_INFO("Computed AO for frame %i", i);
            //RA_LOG_INFO("AO %f, %f",ANIM.AO[339].minCoeff(), ANIM.AO[339].maxCoeff());
        }


        std::vector<std::string> args2 = {"split", "2018_oldman_split_ver"};
        console_load_serialised_data(this, args2);
        if (shouldscale) 
        {
            if (!PIECES.empty()) 
            {
                scale_one_exe(PIECES[0], S, tr);
                scale_one_exe(PIECES[1], S, tr);
            }
        }
        std::vector<std::string> args1 = { "weights_low", "bunny_W"};
        //console_load_serialised_data(this,args1);
        
        
        std::vector<std::string> args3 = {"stop_up", "oldman_20_1_1"};
        console_load_serialised_data(this, args3);
        
        std::vector<std::string> args4 = {"stop_low", "oldman_15_1_0"};
        console_load_serialised_data(this, args4);
        
        if (shouldscale) 
        {
            if (!SMOTION.empty()) 
            {
                // scale_one_exe(SMOTION[0].anim, S, tr);
                // scale_one_exe(SMOTION[1].anim, S, tr);
            }
        }
    
        auto clr = Eigen::Vector3d(255/255.0, 148/255.0, 135/255.0);
        auto clr1 = Eigen::Vector3d(255/255.0, 255/255.0, 255/255.0);
        auto clr2 = Eigen::Vector3d(180/255.0, 100/255.0, 179/255.0);

        tyro::color_matrix(ANIM.F.rows(), clr, ANIM.FC);
                   auto clr3  = Eigen::Vector3d(120/255.0, 151/255.0, 200/255.0);

        if (PIECES.size()>0)
        {
            tyro::color_matrix(PIECES[1].F.rows(), clr3, PIECES[1].FC);
            tyro::color_matrix(PIECES[0].F.rows(), clr2, PIECES[0].FC);
        }

            if (SMOTION.size()>0)
            {
                tyro::color_matrix(SMOTION[0].anim.F.rows(), clr2, SMOTION[0].anim.FC);
                tyro::color_matrix(SMOTION[1].anim.F.rows(), clr3, SMOTION[1].anim.FC);
            }
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
        outFile.close();
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
        outFile.close();
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
        for (int frame = 0; frame < ANIM.VD.size(); ++frame) 
        {
            Eigen::Vector3d new_vec = ANIM.VD[frame].row(vid);
            Eigen::Vector3d d = ref_vec - new_vec;
            RowVector3d diff(d(0), d(1), d(2)); 
            ANIM.VD[frame].rowwise() += diff;
        }
        render();
    }
        

    void App::align_all_models() 
    {
        //last selected vid 
        if (vid_list.size()==0) return;
        int vid = vid_list.back();
        
        Eigen::Vector3d ref_vec;
        for (int frame = 0; frame < ANIM.VD.size(); ++frame) 
        {
            if (frame ==0 ) 
            {
                ref_vec = ANIM.VD[frame].row(vid);
            }
            else 
            {
                Eigen::Vector3d new_vec = ANIM.VD[frame].row(vid);
                Eigen::Vector3d d = ref_vec - new_vec;
                Eigen::RowVector3d diff(d(0), d(1), d(2)); 
                ANIM.VD[frame].rowwise() += diff;
            }
        }

        render();
        glfwPostEmptyEvent();
    }

    void App::selectVertexPart(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier, int which_part) 
    {   
        int fid;
        Eigen::Vector3f bc;
        Wm5::HMatrix modelViewMatrix = m_camera->GetViewMatrix() * RENDER.part[which_part]->WorldTransform.Matrix();
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
                                     PIECES[which_part].VD[m_frame],
                                     PIECES[which_part].F,
                                     fid,
                                     bc))
        {
            if (m_sel_primitive == App::SelectionPrimitive::Vertex) 
            {
                long c;
                bc.maxCoeff(&c);
                int vid = PIECES[which_part].F(fid, c);
                auto it = std::find(vid_list2.begin(), vid_list2.end(), vid);
                if (it == vid_list2.end()) 
                {       
                    Eigen::Vector3d vec = PIECES[which_part].VD[m_frame].row(vid);
                        RA_LOG_INFO("Picked part face_id %i vertex_id %i coord %f %f %f", fid, vid, vec(0), vec(1), vec(2));
                        addSphere(vid, PIECES[which_part].VD[m_frame], Wm5::Vector4f(0,1,0,1), RENDER.part[which_part]->WorldTransform);
                        vid_list2.push_back(vid);
                }  
                else
                {   
                    RA_LOG_INFO("remove vertex %i %i", fid, vid);
                    auto index = std::distance(vid_list2.begin(), it);
                    ball_list.erase(ball_list.begin() + index);
                    vid_list2.erase(vid_list2.begin() + index);
                }
                render();
            }
        }   
    }

    void App::selectVertex(Eigen::Vector2f& mouse_pos, int mouse_button, int modifier) 
    {   
        if (modifier == TYRO_MOD_ALT) 
        { 
            if (mouse_button == 0)
                selectVertexPart(mouse_pos, mouse_button, modifier, 0);
            else
                selectVertexPart(mouse_pos, mouse_button, modifier, 1); 
            return;
        }

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

        if (igl::unproject_onto_mesh(mouse_pos, 
                                     e1.transpose(),
                                     e2.transpose(),
                                     e3,
                                     ANIM.VD[m_frame],
                                     ANIM.F,
                                     fid,
                                     bc))
        {
            if (m_sel_primitive == App::SelectionPrimitive::Vertex) 
            {   
                if (modifier == TYRO_MOD_ALT) 
                {
                    long c;
                    bc.maxCoeff(&c);
                    int vid = ANIM.F(fid, c);
                    auto it = std::find(vid_list2.begin(), vid_list2.end(), vid);
                    if (it == vid_list2.end()) 
                    {       
                        Eigen::Vector3d vec = ANIM.VD[m_frame].row(vid);
                        RA_LOG_INFO("Picked face_id %i vertex_id %i coord %f %f %f", fid, vid, vec(0), vec(1), vec(2));
                        addSphere(vid, ANIM.VD[m_frame], Wm5::Vector4f(0,1,0,1));
                        add_vertex2(vid);
                    }  
                    else
                    {   
                        RA_LOG_INFO("remove vertex %i %i", fid, vid);
                        auto index = std::distance(vid_list2.begin(), it);
                        ball_list.erase(ball_list.begin() + index);
                        vid_list2.erase(vid_list2.begin() + index);
                    }
                    render(); 
                }
                else 
                {
                    long c;
                    bc.maxCoeff(&c);
                    int vid = ANIM.F(fid, c);
                    auto it = std::find(vid_list.begin(), vid_list.end(), vid);
                    if (it == vid_list.end()) 
                    {       
                        Eigen::Vector3d vec = ANIM.VD[m_frame].row(vid);
                            RA_LOG_INFO("Picked face_id %i vertex_id %i coord %f %f %f", fid, vid, vec(0), vec(1), vec(2));
                            addSphere(vid,  ANIM.VD[m_frame]);
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
            } 
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
        }   
    }
    
    
    void App::debug_show_faces_near_edge_selection(const Eigen::VectorXi& uEI, const Eigen::VectorXi& DMAP) 
    {   
        const auto & cE = ANIM.UE;
        const auto & cEMAP = ANIM.EMAP;
        MatrixXi EF, EI;

        igl::edge_flaps(ANIM.F, 
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

   

    void App::add_face(int fid)
    {   
        auto it = std::find(fid_list.begin(), fid_list.end(), fid);
        if (it == fid_list.end())
        {
            fid_list.push_back(fid);
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

    void App::add_vertex2(int vid)
    {   
        auto it = std::find(vid_list2.begin(), vid_list2.end(), vid);
        if (it == vid_list2.end())
        {
            vid_list2.push_back(vid);
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

            Wm5::HMatrix modelViewMatrix = m_camera->GetViewMatrix() * RENDER.mesh->WorldTransform.Matrix();
            Wm5::HMatrix projectMatrix = m_camera->GetProjectionMatrix();
            Eigen::Matrix4f e1 = Eigen::Map<Eigen::Matrix4f>(modelViewMatrix.mEntry);
            Eigen::Matrix4f e2 = Eigen::Map<Eigen::Matrix4f>(projectMatrix.mEntry);
            Eigen::Vector4f e3 = Eigen::Vector4f(m_camera->GetViewport()[0],
                                                m_camera->GetViewport()[1],
                                                m_camera->GetViewport()[2],
                                                m_camera->GetViewport()[3]);


            MatrixXd P;
            igl::project(ANIM.VD[m_frame],
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
                    if (m_selection_type == Select) 
                    {
                        addSphere(vid,  ANIM.VD[m_frame]);
                        add_vertex(vid);
                    }
                    else
                    {   
                        
                        auto it = std::find(vid_list.begin(), vid_list.end(), vid);
                        if (it != vid_list.end()) 
                        {
                            RA_LOG_INFO("remove vertex %i ", vid);
                            auto index = std::distance(vid_list.begin(), it);
                            ball_list.erase(ball_list.begin() + index);
                            vid_list.erase(vid_list.begin() + index);
                        }
                    }
                }
            }
            else if (m_sel_primitive == App::SelectionPrimitive::Faces) 
            {   
                //std::vector<int> fid_selected;
                for (int fid = 0; fid < ANIM.F.rows(); ++fid) 
                {
                    Eigen::Vector3i vids = ANIM.F.row(fid);

                    auto it = std::find(vid_selected.begin(), vid_selected.end(), vids(0));
                    if (it != vid_selected.end())  
                    {
                        add_face(fid); 
                        continue;
                    } 
                    
                    it = std::find(vid_selected.begin(), vid_selected.end(), vids(1));
                    if (it != vid_selected.end()) 
                    { 
                        add_face(fid);
                        continue;
                    }

                    it = std::find(vid_selected.begin(), vid_selected.end(), vids(2));
                    if (it != vid_selected.end()) 
                    {
                        add_face(fid); 
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