#include "TyroApp.h"

#include "RAEnginePrerequisites.h"
#include "RAES2TextOverlay.h"
#include "RAFont.h"
#include "RAVisibleSet.h"
#include "RAES2StandardMesh.h"
#include "RAAxisAlignedBBox.h"
#include "ES2VideoTexture.h"
#include <stdio.h>

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
#include <igl/ambient_occlusion.h>
#include <igl/boundary_loop.h>
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
        num_edges = vid_list.size(); //assumes its a closed loop
        eid_list.resize(num_edges, 2);
        for (int i = 0; i < vid_list.size(); ++i) 
        {   
            if (i == vid_list.size() - 1) //last edge
                eid_list.row(i) = Eigen::Vector2i(vid_list[i], vid_list[0]);
            else
                eid_list.row(i) = Eigen::Vector2i(vid_list[i], vid_list[i+1]);
        }
    }
    else 
    {
 
        num_edges = vid_list.size() - 1; //assumes its a closed loop
        eid_list.resize(num_edges, 2);
        for (int i = 0; i < vid_list.size(); ++i) 
        {   
            if (i < vid_list.size() - 1) //last edge
                eid_list.row(i) = Eigen::Vector2i(vid_list[i], vid_list[i+1]);
        }
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
{       void console_load_serialised_data(App* app, const std::vector<std::string>& args); 

        void console_load_bunny_stop(App* app, const std::vector<std::string>& args) 
        {
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path("subanimaton");

            std::ifstream in = std::ifstream(p.str(), std::ios::binary);
            cereal::BinaryInputArchive archive_i(in);
            archive_i(app->ANIM);
            tyro::color_matrix(app->ANIM.UE.rows(), Eigen::Vector3d(0,0,0), app->ANIM.UEC);

            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);

                
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = 0.55*2*bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            
            app->ANIM.VW.setOnes();
            //app->m_weights.VW.resize(app->ANIM.VD[0].rows());
            //app->m_weights.VW.setOnes();

            //app->m_weights.FW.resize(app->ANIM.F.rows());
            //app->m_weights.FW.setOnes();

            app->compute_average();

            std::vector<std::string> args1 = {"split", "SUBPIECES"};
            console_load_serialised_data(app, args1);
            
            std::vector<std::string> args2 = {"stop_low", "bunny_stop_200_2_1_kmeans"};
            //console_load_serialised_data(this, args2);

            std::vector<std::string> args3 = {"stop_up", "bunny_stop_100_1_0_kmeans"};
            //console_load_serialised_data(this, args3);

            app->render();
        }

        void console_save_sub_video(App* app, const std::vector<std::string>& args) 
        {
            int frame[] = {1042,1597,1738,1947};  
            App::VideoCV video;

             //main animation
            int total_size = 0;
            for (int s  = 0; s < 4; s+=2) 
            { 
                total_size += frame[s+1] - frame[s] + 1;
            } 

            video.F.resize(total_size);
            
            int cur_frame = 0;
            for (int s  = 0; s < 4; s+=2) 
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
            auto p = f/filesystem::path(std::string("subvideo_frames"));
            std::ofstream os(p.str(), std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(video);

        }
        void copy_sub(App::MAnimation& subanimaton, int *frame, App::MAnimation& sourceAnim) 
        {
            using namespace Eigen;
            using namespace std;
            
            //main animation
            int total_size = 0;
            for (int s  = 0; s < 4; s+=2) 
            { 
                total_size += frame[s+1] - frame[s] + 1;
            } 

            //main animation
            subanimaton.VD.resize(total_size);
            subanimaton.ND.resize(total_size);

            int cur_frame = 0;
            for (int s  = 0; s < 4; s+=2) 
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
            int frame[] = {1042,1597,1738,1947};    

            std::vector<App::MAnimation> SUBPIECES; // Break deformed mesh into pieces along seam(s).
            SUBPIECES.resize(app->PIECES.size());
           
            for (int i=0; i<SUBPIECES.size(); ++i) 
            {   
                copy_sub(SUBPIECES[i], frame, app->PIECES[i]);
            }
            

            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path(std::string("SUBPIECES"));
            std::ofstream os(p.str(), std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(SUBPIECES);
        }

        void console_save_sub_animation(App* app, const std::vector<std::string>& args) 
        {
    
            int frame[] = {1042,1597,1738,1947};    
            
            //main animation
            App::MAnimation subanimaton;
            copy_sub(subanimaton, frame, app->ANIM);

            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path(std::string("subanimaton"));
            std::ofstream os(p.str(), std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(subanimaton);
        }

        void console_save_for_printing(App* app, const std::vector<std::string>& args) 
        {
            auto path = std::string("/home/rinat/Workspace/Tyro/Source/tmp/suckmydick");
            //std::ofstream outFile(path);
            //for (int i = 0; i < app->PIECES[1].V.size(); ++i)
            //{ 
                //outFile << app->PIECES[1].VF[0] << "\n";
            //}
            
            auto& part = app->PIECES[1];
            auto f = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj");
            auto p = f/filesystem::path(std::string("alec_btm.obj"));
            for (int i = 0; i < part.VD.size(); ++i) 
            {
                MatrixXd N;
                MatrixXd TC;
                MatrixXi FTC;
    
                N.resize(part.VD[app->m_frame].rows(), 3);
                N.setOnes();
                igl::writeOBJ(
                p.str(), 
                part.VD[app->m_frame], 
                part.F);
                break;
            }


        }

        void console_render_to_video(App* app, const std::vector<std::string>& args) 
        {   
            int v_width, v_height;
            app->m_tyro_window->GetGLContext()->getFramebufferSize(&v_width, &v_height);
            
            cv::Size S = cv::Size(v_width, v_height);

            cv::VideoWriter outputVideo;                                        // Open the output
            outputVideo.open("/home/rinat/bunny.avi",
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
            app->m_frame = 0;
            
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
            
            cv::imwrite( "/home/rinat/fuck.jpg", image3);
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

        void console_plot_error_vel(App* app, const std::vector<std::string>& args) 
        {
            using namespace Eigen;
       
            int num_parts = app->SMOTION.size();
            int num_frames = app->ANIM.VD.size();
            app->m_error_velocity.resize(num_parts);
            app->max_error_velocity = std::numeric_limits<float>::min();
            
            for (int p = 0; p < num_parts; ++p) 
            {   
                if (app->SMOTION[p].computed) 
                {
                    app->m_error_velocity[p].resize(num_frames); 
                    VectorXd zero;
                    zero.resize(app->PIECES[p].VD[0].rows());
                    zero.setZero();

                    app->m_error_velocity[p][0] = zero;

                    for (int frame = 1; frame < num_frames; ++frame) 
                    {                   
                        //VC[frame].resize(FD.VD[0].rows());
                        VectorXd lul = ((app->PIECES[p].VD[frame] - app->PIECES[p].VD[frame-1]) - 
                                        (app->SMOTION[p].anim.VD[frame] - app->SMOTION[p].anim.VD[frame-1])).rowwise().squaredNorm();  
                        float max = lul.maxCoeff();
                        app->max_error_velocity = std::max(app->max_error_velocity, max); 
                        app->m_error_velocity[p][frame] = lul;                          
                    }
                }
            }
            app->m_computed_vel_error =true;
        }


        void console_plot_error(App* app, const std::vector<std::string>& args) 
        {   
            using namespace Eigen;
            
            int num_parts = app->SMOTION.size();
            int num_frames = app->ANIM.VD.size();
            app->m_error.resize(num_parts);
            app->max_error = std::numeric_limits<float>::min();
            for (int p = 0; p < num_parts; ++p) 
            {   
                if (app->SMOTION[p].computed) 
                {
                    app->m_error[p].resize(num_frames); 
                    for (int frame=0; frame < num_frames; ++frame) 
                    {                   
                        //VC[frame].resize(FD.VD[0].rows());
                        VectorXd lul = (app->PIECES[p].VD[frame] - app->SMOTION[p].anim.VD[frame]).rowwise().squaredNorm();  
                        float max = lul.maxCoeff();
                        app->max_error = std::max(app->max_error, max); 
                        app->m_error[p][frame] = lul;                          
                    }
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
            else if (type == "split") 
            {
                archive(app->PIECES);
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
            else if (type == "deform") 
            {
                archive(app->DANIM);
                app->m_computed_deformation = true;
                app->m_update_camera = true;
            }
            else if (type == "split") 
            {
                archive(app->PIECES);
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
            }
            else if (type == "stop_up") 
            {
                if (app->SMOTION.empty())
                    app->SMOTION.resize(2);
                
                archive(app->SMOTION[0]);
                app->SMOTION[0].computed = true;
                
                app->m_computed_stop_motion = true;
                app->m_update_camera = true;
            }
            else if (type == "weights_low") 
            {
                archive(app->PIECES[1].VW);
                for (int i = 0; i < app->PIECES[1].VW.size(); ++i) 
                {
                    if (app->PIECES[1].VW[i] > 1)
                        app->addSphere(i, app->PIECES[1].VD[app->m_frame], Wm5::Vector4f(0,1,0,1), app->RENDER.part[1]->WorldTransform);
                }                
            }


            app->render();
            glfwPostEmptyEvent();
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
            
            //const auto& FD = app->ANIM;

            VectorXi L;
            VectorXi flist1 = Eigen::Map<VectorXi>(app->fid_list.data(), 
                                                   app->fid_list.size());
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
                        app->ANIM.UEC.row(i) = Eigen::Vector3d(0,0.0,0.8);

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
            RA_LOG_INFO("Converting vertex to edge selection");
            
            if (args.size() != 1)  return;

            MatrixXi eid_list;
            VectorXi EI, uEI, DMAP;
            convert_vertex_to_edge_selection(app->vid_list, app->ANIM.E, app->ANIM.UE, 
                                             app->ANIM.EMAP, std::stoi(args[0]), eid_list, EI, 
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
                                             app->DANIM.E, 
                                             app->DANIM.UE, 
                                             app->DANIM.EMAP,
                                             isClosedSeam,
                                             eid_list, 
                                             EI, 
                                             uEI, 
                                             DMAP);
        

            MatrixXi F1, F2;
            tyro::mesh_split(app->DANIM.F,
                             uEI,
                             DMAP, 
                             F1, 
                             F2);
            
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
                //app->SMOTION.clear();
                    app->SMOTION.resize(app->PIECES.size());

                int start, end;
                if (part_id == -1) 
                {
                    start = 0;
                    end = app->SMOTION.size();
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
                    auto& sm = app->SMOTION[i]; 
                    auto& piece = app->PIECES[i];
                    bool kmeans = false;
                    if (initmethod == "kmeans") kmeans = true; 

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

                    for (int j = 0; j < sm.L.size(); ++j) 
                    {
                        int l_idx = sm.L(j);
                        sm.anim.VD.push_back(sm.D[l_idx]);
                        sm.anim.ND.push_back(normals[l_idx]);
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
                igl::slice(app->ANIM.F, slice_list, 1, newF);
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
                        app->fid_list.push_back(fid);
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
                    app->vid_list.push_back(vid);
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
                    app->load_mesh_sequence(obj_paths, true); //use IGL obj loader
                }
            }

            
            if (!igl::is_edge_manifold(app->ANIM.F)) 
            {   
                RA_LOG_ERROR_ASSERT("not manifold");
                return;
            }
            app->m_timeline->SetFrameRange(app->ANIM.VD.size()-1);

            //app->align_all_models(offset_vid, offset);
                
            //Compute radius of the bounding box of the model
            AxisAlignedBBox bbox;
            MatrixXd VT = app->ANIM.VD[0].transpose();
            bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
            app->m_model_offset = 0.7*2*bbox.GetRadius(); 

            app->m_update_camera = true;
            app->m_state = App::State::LoadedModel;
            app->compute_average();

            //app->m_weights.VW.resize(app->ANIM.VD[0].rows());
            //app->m_weights.VW.setOnes();

            //app->m_weights.FW.resize(app->ANIM.F.rows());
            //app->m_weights.FW.setOnes();
            
            app->compute_average();

            std::vector<std::string> args1 = {"split", "monka_split"};
            console_load_serialised_data(app,args1);

            std::vector<std::string> args2 = {"stop_low", "monka_stop_100_1_1_kmeans"};
            console_load_serialised_data(app, args2);
            
            std::vector<std::string> args3 = {"stop_up", "monka_stop_50_1_0_kmeans"};
            console_load_serialised_data(app, args3);

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
        VideoCapture cap("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monka/monka.mp4"); 
            
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
            auto a = filesystem::path("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/monka/monka_images");
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
                                                 Wm5::Vector4f(1,1,1,1), 
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

        std::vector<std::string> obj_paths;
        for (auto& folder : FOLDERS__) 
        {
            int num_files_read;
            RA_LOG_INFO("loading folder %s", folder.data());
            tyro::obj_file_path_list(folder, "objlist.txt", obj_paths, num_files_read);
            RA_LOG_INFO("frames read %i", num_files_read);
        }       
        /*
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
        m_gl_rend->SetClearColor(Wm5::Vector4f(0.1, 0.1, 0.1, 1.0));

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
        register_console_function("console_offset_frame", console_offset_frame, "");
        register_console_function("plot_error", console_plot_error, "");
        register_console_function("plot_error_vel", console_plot_error_vel, "");

        register_console_function("show_seam", console_show_seam, ""); 
        register_console_function("render_to_image", console_render_to_image, ""); 
        register_console_function("render_to_video", console_render_to_video, ""); 

        register_console_function("save_sub_split", console_save_sub_split, "");
        register_console_function("save_sub_animation", console_save_sub_animation, "");
        register_console_function("save_for_printing", console_save_for_printing, "");
        register_console_function("save_sub_video", console_save_sub_video, "");
        register_console_function("load_bunny_stop", console_load_bunny_stop, "");


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
        
       // ParseImages();

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
            RENDER.seam_on_main = IGLMeshWireframe::Create(ANIM.VD[m_frame], ANIM.UE, ANIM.UEC, m_eid_list2);
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

            vis_set.Insert(RENDER.avg.get());

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
                
                vis_set.Insert(RENDER.avg_wire.get());
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

            for (int i = 0; i < PIECES.size(); ++i) 
            {    
                Wm5::Transform tr;
                tr.SetTranslate(Wm5::APoint(-3*m_model_offset, 0, 0));

                if (RENDER.part[i] == nullptr) 
                {
                    RENDER.part[i] = IGLMesh::Create(PIECES[i].VD[m_frame], PIECES[i].F, PIECES[i].ND[m_frame], PIECES[i].FC);
                    RENDER.part[i]->LocalTransform = tr * RENDER.part[i]->LocalTransform;
                    RENDER.part[i]->Update(true);
                }
                else
                    RENDER.part[i]->UpdateData(PIECES[i].VD[m_frame], PIECES[i].F, PIECES[i].ND[m_frame], PIECES[i].FC);

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
                tr.SetTranslate(Wm5::APoint(2*m_model_offset, -m_model_offset/2, 0));
                if (RENDER.error[i] == nullptr) 
                {
                    RENDER.error[i] = IGLMesh::CreateColor(SMOTION[i].anim.VD[m_frame], SMOTION[i].anim.F, 
                                                           SMOTION[i].anim.ND[m_frame], SMOTION[i].anim.FC, 
                                                           m_error[i][m_frame], max_error,maxColor);
                    
                    RENDER.error[i]->LocalTransform = tr * RENDER.error[i]->LocalTransform;
                    RENDER.error[i]->Update(true);
                }
                else 
                { 
                    RENDER.error[i]->UpdateData(SMOTION[i].anim.VD[m_frame], SMOTION[i].anim.F, 
                                                SMOTION[i].anim.ND[m_frame], SMOTION[i].anim.FC, 
                                                m_error[i][m_frame], max_error,maxColor);
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
                Eigen::Vector3f maxColor = Eigen::Vector3f(104/255.0, 34/255.0, 139/255.0);
                Wm5::Transform tr;
                tr.SetTranslate(Wm5::APoint(2*m_model_offset, m_model_offset/2, 0));
                if (RENDER.errorVel[i] == nullptr) 
                {
                    
                    RENDER.errorVel[i] = IGLMesh::CreateColor(SMOTION[i].anim.VD[m_frame], SMOTION[i].anim.F, 
                                                              SMOTION[i].anim.ND[m_frame], SMOTION[i].anim.FC, 
                                                              m_error_velocity[i][m_frame], max_error_velocity, 
                                                              maxColor);
                    
                    RENDER.errorVel[i]->LocalTransform = tr * RENDER.errorVel[i]->LocalTransform;
                    RENDER.errorVel[i]->Update(true);
                }
                else 
                { 
                    RENDER.errorVel[i]->UpdateData(SMOTION[i].anim.VD[m_frame], SMOTION[i].anim.F, 
                                                   SMOTION[i].anim.ND[m_frame], SMOTION[i].anim.FC, 
                                                   m_error_velocity[i][m_frame], max_error_velocity,
                                                   maxColor);
                }

                vis_set.Insert(RENDER.errorVel[i].get());
            } 
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
            fid_list.push_back(fid);
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
            WorldBoundBox.Merge(RENDER.avg->WorldBoundBox);
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
        float radius = std::abs(WorldBoundBox.GetRadius()*2.5);
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
        float s = 0.001;
        ES2SphereSPtr object = ES2Sphere::Create(10, 10, s);
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
            //auto p = f/filesystem::path("bunny_frames_front");
            
            //auto p = f/filesystem::path("subanimaton");


            std::ifstream in = std::ifstream(p.str(), std::ios::binary);
            cereal::BinaryInputArchive archive_i(in);
            archive_i(ANIM);
            tyro::color_matrix(ANIM.UE.rows(), Eigen::Vector3d(0,0,0), ANIM.UEC);

            //TODO REMOVE FOR FULL MOVIE
           // ANIM.SIdx = {106, 450, 210};
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
                ANIM.SIdx.push_back(num_files_read);
            }
            load_mesh_sequence(obj_paths, true); //use IGL obj loader
        }
        m_timeline->SetFrameRange(ANIM.VD.size()-1);

        //align_all_models(offset_vid, offset);

               
        //Compute radius of the bounding box of the model
        AxisAlignedBBox bbox;
        MatrixXd VT = ANIM.VD[0].transpose();
        bbox.ComputeExtremesd(VT.cols(), 3*sizeof(double), VT.data());
        m_model_offset = 0.55*2*bbox.GetRadius(); 

        m_update_camera = true;
        m_state = App::State::LoadedModel;
        
        ANIM.VW.setOnes();
        //m_weights.VW.resize(ANIM.VD[0].rows());
        //m_weights.VW.setOnes();

        //m_weights.FW.resize(ANIM.F.rows());
        //m_weights.FW.setOnes();

        compute_average();


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

        //ParseImages();
        std::vector<std::string> args1 = {"split", "bunny_split"};
        //console_load_serialised_data(this,args1);
        
        std::vector<std::string> args2 = {"stop_low", "bunny_stop_200_2_1_kmeans"};
        //console_load_serialised_data(this, args2);

        std::vector<std::string> args3 = {"stop_up", "bunny_stop_100_1_0_kmeans"};
        //console_load_serialised_data(this, args3);

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
                long c;
                bc.maxCoeff(&c);
                int vid = ANIM.F(fid, c);
                auto it = std::find(vid_list.begin(), vid_list.end(), vid);
                if (it == vid_list.end()) 
                {       
                    Eigen::Vector3d vec = ANIM.VD[m_frame].row(vid);
                        RA_LOG_INFO("Picked face_id %i vertex_id %i coord %f %f %f", fid, vid, vec(0), vec(1), vec(2));
                        addSphere(vid,  ANIM.VD[m_frame]);
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
                    addSphere(vid,  ANIM.VD[m_frame]);
                    vid_list.push_back(vid);
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