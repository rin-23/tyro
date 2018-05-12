#include <Eigen/Core>
#include <vector>
namespace tyro
{
void segmentation(const std::vector<Eigen::MatrixXd>& v_data, //vertex data
                  const Eigen::MatrixXi& F, //face data
                  const Eigen::MatrixXd& Vavg, //average of faces
                  const Eigen::MatrixXi& inEF, //edge flaps 
                  const Eigen::MatrixXi& inuE, //unique edges
				  const Eigen::VectorXi& seeds, //seeds(indicies of faces that must belong to a label)
                  Eigen::VectorXi& L);  //labeling of faces 
}