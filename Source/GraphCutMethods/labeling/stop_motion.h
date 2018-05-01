#pragma once

#include <vector>
#include <Eigen/Core>

namespace tyro
{

int stop_motion_vertex_distance(int num_labels,  // how many faces should dictionary contain.
                            	const std::vector<Eigen::MatrixXd>& v_data, // array (size num_frames) of vertex data for every frame 
                            	const Eigen::MatrixXi& f_data, // face data
								std::vector<Eigen::MatrixXd>& d_data,// array(size num_labels) of vertex data. 
								std::vector<int>& s_data,  // array (size num_frames) of indicies into D for every frame.  
                            	double& result_energy); // final energy after minimization.
}