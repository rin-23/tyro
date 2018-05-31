#include <Eigen/Core>
#include <vector>
namespace tyro
{
void segmentation(const std::vector<Eigen::MatrixXd>& v_data, //vertex data
                  const Eigen::MatrixXi& FO, //original face data
                  const Eigen::MatrixXd& Vavg, //average of faces
				  const Eigen::VectorXi& S1, //seeds(indicies of faces that must belong to a label)
                  const Eigen::VectorXi& S2,
                  double smooth_weight,
                  Eigen::VectorXi& L, //labeling of faces
                  const Eigen::VectorXi& isBoundary);   
}