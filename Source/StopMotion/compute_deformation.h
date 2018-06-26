#pragma once
#include <Eigen/Core>
#include <vector>

namespace tyro 
{
bool compute_deformation(const std::vector<int>& vid_list, 
                         const std::vector<int>& fid_list, 
                         const std::vector<Eigen::MatrixXd>& v_data,
                         const Eigen::MatrixXi& F,
                         const Eigen::MatrixXd& AV, //average                       
                        std::vector<Eigen::MatrixXd>& rv_data);

bool compute_deformation2( const std::vector<int>& vid_list_avg,
                            const std::vector<int>& vid_list_frame, 
                            const std::vector<Eigen::MatrixXd>& v_data,
                            const Eigen::MatrixXi& F,
                            const Eigen::MatrixXd& AV, //average                       
                            std::vector<Eigen::MatrixXd>& rv_data,
                            std::vector<Eigen::MatrixXd>& Laplacian);
}
