#include <Eigen/Dense>

namespace tyro 
{
    void kmeans(const Eigen::MatrixXd& F, //data. Every column is a feature 
                const int num_labels, // number of clusters
                const int num_iter, // number of iterations
                Eigen::MatrixXd& D, // dictionary of clusters (every column is a cluster)
                Eigen::VectorXi& labels); // map D to F.
}