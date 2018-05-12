#pragma once

#include "prerequsites.h"

//template <typename DerivedV>
bool saveMatrix(std::string filename, Eigen::VectorXi& matrix, bool overwrite=true);

bool saveMatrix(std::string filename, Eigen::VectorXd& matrix, bool overwrite = true);

//void loadMatricies(Eigen::MatrixXd*& verticies, Eigen::MatrixXi& F, int num_files, char** files);

bool loadMatrix(std::string filename, Eigen::MatrixXd& m);

bool saveMatrix(std::string filename, Eigen::MatrixXd& matrix, bool overwrite = true);

void printMatrix(const Eigen::MatrixXd& mat, std::string& header);

int generateRandomNumbers(int** numbers, int nSimulations, int numFrames);

bool loadBlendshapeTargets(const std::string& objPath, const std::string& neut_path, Eigen::MatrixXd& B);

bool loadFrames(const std::string& objPath, Eigen::MatrixXd& FRAMES, Eigen::MatrixXd& SAVED_FACES);

bool saveDictionary(const std::string& labelOBJ, Eigen::MatrixXd& D, Eigen::MatrixXd& SAVED_FACES);

int convertOFFtoOBJ(std::string& objPath);

template <class MatT>
Eigen::Matrix<typename MatT::Scalar, MatT::ColsAtCompileTime, MatT::RowsAtCompileTime>
pseudoinverse(const MatT &mat, typename MatT::Scalar tolerance = typename MatT::Scalar{ 1e-4 }) // choose appropriately
{
	typedef typename MatT::Scalar Scalar;
	auto svd = mat.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);
	const auto &singularValues = svd.singularValues();
	Eigen::Matrix<Scalar, MatT::ColsAtCompileTime, MatT::RowsAtCompileTime> singularValuesInv(mat.cols(), mat.rows());
	singularValuesInv.setZero();
	for (unsigned int i = 0; i < singularValues.size(); ++i) {
		if (singularValues(i) > tolerance)
		{
			singularValuesInv(i, i) = Scalar{ 1 } / singularValues(i);
		}
		else
		{
			singularValuesInv(i, i) = Scalar{ 0 };
		}
	}
	return svd.matrixV() * singularValuesInv * svd.matrixU().adjoint();
}