#pragma once

#include <Eigen/Core>
#include <vector>

using Eigen::VectorXd;

namespace tyro
{   

    //Matlab like all function
    // V = matrix
    // condition - bool test function
    // dim - 0 column, 1 row
    // indicies into V that pass condition
    //TODO need to figure out templates
    template <typename DerivedX>
    void all(const Eigen::DenseBase<DerivedX>& X, 
             std::function<bool(const Eigen::VectorXi&)> condition, 
             int dim,
             Eigen::VectorXi& I) 
    {
        std::vector<int> indicies;
        if (dim == 0)  //column
        {
            for (int i = 0; i < X.cols(); ++i) 
            {
                auto v = X.col(i);
                if (condition(v)) 
                {
                    indicies.push_back(i);
                } 
            }
        }
        else 
        {
            assert(dim == 1);
            for (int i = 0; i < X.rows(); ++i) 
            {
                VectorXd v = X.row(i);
                if (condition(v)) 
                {
                    indicies.push_back(i);
                } 
            }
        }

        I.resize(indicies.size());
        for (int i =0; i < indicies.size(); ++i) 
        {
            I(i) = indicies[i];
        }
    }
}