#include "save_mesh_sequence.h"

void tyro::save_mesh_sequence_with_selected_faces(const std::string& folder, 
                                                  const std::string& filename,
                                                  const std::vector<int> fid_list,
                                                  ) 
    {   
        if (fid_list.size() > 0) 
        {   
            auto path = std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj_export/03_apple/") + folder + std::string("/") + filename;
            auto tmp_path = std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj_export/03_apple/") + folder + std::string("/") + "tmp";
            auto objlist_path = std::string("/home/rinat/GDrive/StopMotionProject/BlenderOpenMovies/bunny rinat/production/obj_export/03_apple/") + folder + std::string("/") + std::string("objlist.txt");
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