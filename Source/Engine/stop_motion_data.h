#pragma once

#include <vector>
#include <Eigen/Core>

namespace stop 
{
    bool load_mesh_sequence(const std::string& obj_list_file,
                            std::vector<Eigen::MatrixXd>& v_data, 
                            std::vector<Eigen::MatrixXd>& n_data,  
                            Eigen::MatrixXi& f_data);

    bool load_mesh_sequence(const std::string& obj_list_file,
                            Eigen::MatrixXd& v_data, 
                            Eigen::MatrixXd& n_data,  
                            Eigen::MatrixXi& f_data);
}

