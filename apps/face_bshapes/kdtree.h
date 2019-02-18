#pragma once
//#include <Eigen/Dense>
#include "nanoflann.hpp"
#include <vector>
#include "KDTreeVectorOfVectorsAdaptor.h"

namespace tyro 
{
    class KDTree 
    {
        public:
            KDTree();
            ~KDTree();

            void InitWithData(const std::vector<std::vector<double>>& data);
            void FindClosest(const std::vector<double>& query_pt, double& c_dist);
            void Serialize(const std::string& path);
            void Deserialize(const std::string& path);

        private:

            int mNumPoints;
            int mDim;
            //typedef nanoflann::KDTreeEigenMatrixAdaptor<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>> my_kd_tree_t;
            typedef std::vector<std::vector<double> > my_vector_of_vectors_t;
            typedef KDTreeVectorOfVectorsAdaptor< my_vector_of_vectors_t, double >  my_kd_tree_t;
            my_kd_tree_t* mat_index;
    };
}