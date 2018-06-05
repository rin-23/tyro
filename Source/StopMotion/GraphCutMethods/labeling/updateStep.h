#pragma once

#include "prerequsites.h"
#include <Eigen/Sparse>

void updateStepVertex(const Eigen::DiagonalMatrix<double, Eigen::Dynamic>& M, const Eigen::MatrixXd& W_T, const Eigen::MatrixXd& B, Eigen::MatrixXd& D_T, const Eigen::VectorXi& S_vec, double w_s,  double& oldEnergy, double& newEnergy);

void updateStepBShape(const Eigen::MatrixXd& F_T, const Eigen::MatrixXd& M, const Eigen::VectorXi& S_vec, double w_s, Eigen::MatrixXd& L_T, double& oldEnergy, double& newEnergy);

void updateStepTRUEVertex(const Eigen::MatrixXd& F, Eigen::VectorXd& VW, Eigen::MatrixXd& D, const Eigen::VectorXi& S_vec, std::vector<int>& sequenceIdx, double w_s, double& oldEnergy, double& newEnergy, Eigen::SparseMatrix<double> &G, Eigen::MatrixXd& K_prime);
