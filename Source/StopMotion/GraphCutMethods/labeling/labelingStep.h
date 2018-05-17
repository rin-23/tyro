#pragma once

#include "prerequsites.h"

//void labelFacesPositions(std::vector<Eigen::MatrixXd>& dataObj, std::vector<Eigen::MatrixXd>& labelObj, const Eigen::MatrixXd& L, Eigen::VectorXi& S_vec, double w_s);

void labelFacesVertex(Eigen::DiagonalMatrix<double, Eigen::Dynamic>& M, Eigen::MatrixXd& W, Eigen::MatrixXd& B, Eigen::MatrixXd& D, Eigen::VectorXi& S_vec, double w_s, double& energy);
void labelFacesBShape(Eigen::MatrixXd& F, Eigen::MatrixXd& L, Eigen::MatrixXd& M, Eigen::VectorXi& S_vec, double w_s, double& energy);
void labelFacesTRUEVertex(Eigen::MatrixXd& F, Eigen::MatrixXd& D, Eigen::VectorXi& S_vec, std::vector<int>& sequenceIdx, double w_s, double& energy);
