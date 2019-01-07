#pragma once

#include <vector>
#include <Eigen/Core>

namespace tyro 
{   
    //Custom mesh loader for stop motion project
    bool load_mesh_sequence(const std::vector<std::string>& obj_paths,
                            std::vector<Eigen::MatrixXd>& v_data, 
                            std::vector<Eigen::MatrixXd>& n_data,  
                            Eigen::MatrixXi& f_data,
                            Eigen::MatrixXi& e_data,
                            Eigen::MatrixXi& ue_data,
                            Eigen::VectorXi& EMAP,                             
                            bool use_igl_loader = true);

    
    //Load a mesh from obj file plus edge data
    bool load_mesh(const std::string& obj_path,
                    Eigen::MatrixXd& V, 
                    Eigen::MatrixXd& N,  
                    Eigen::MatrixXi& F,
                    Eigen::MatrixXi& E,
                    Eigen::MatrixXi& UE,
                    Eigen::VectorXi& EMAP);

    //Load a mesh from obj file
    bool load_mesh(const std::string& obj_path,
                    Eigen::MatrixXd& v_data, 
                    Eigen::MatrixXd& n_data,  
                    Eigen::MatrixXi& f_data);
    
    //Load a multiple meshes from the list of obj files.
    //Assume they all have same topology so only on Face data matrix is returned
    bool load_meshes(const std::vector<std::string>& obj_paths,
                     std::vector<Eigen::MatrixXd>& v_data, 
                     std::vector<Eigen::MatrixXd>& n_data,  
                     Eigen::MatrixXi& f_data);

}

