#pragma once

#include <vector>
#include <Eigen/Core>

namespace tyro
{

int stop_motion_vertex_distance(int num_labels, 
                            	const std::vector<Eigen::MatrixXd>& v_data,
                            	const Eigen::MatrixXi& f_data,
								std::vector<Eigen::MatrixXd>& D, //dictionary
								Eigen::VectorXi& S_vec, //labels 
                            	double& result_energy);
}