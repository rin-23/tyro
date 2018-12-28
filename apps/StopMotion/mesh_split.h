#pragma once

#include <Eigen/Core>
#include <vector>

namespace tyro 
{
//Cut mesh F,V along edges in E_seam.
// in F
// in V
// in E_seam
// out F1 indicies into V of the first part
// out F2 indicies into V of the second part
void mesh_split(const Eigen::MatrixXi& F, 
                const Eigen::VectorXi& uEI, // indicies of seam edges into unique edges matrix
                const Eigen::VectorXi& DMAP, // checks which directions where switched HACKY
                Eigen::VectorXi& F1idx, 
                Eigen::VectorXi& F2idx);
}