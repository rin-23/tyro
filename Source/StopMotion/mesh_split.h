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
void mesh_cut(const Eigen::MatrixXi& F, 
              const Eigen::VectorXi& E_seam,
              Eigen::MatrixXi& F1, 
              Eigen::MatrixXi& F2);
}