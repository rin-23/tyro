#pragma once

#include <Eigen/Core>


// Normalize matrix values to be between the range 0 and 1.
// same as gptoolbox

namespace tyro 
{
void matrixnomalize(const Eigen::MatrixXd& M, MatrixXd& N) 
{
    //N = (M-min(M(:)))./(max(M(:))-min(M(:)));
    double minCoeff = M.minCoeff();
    double maxCoeff = M.maxCoeff();
    N = (M - minCoeff) / (maxCoeff - minCoeff);

}
}