#include "stop_motion.h"
#include <algorithm>
#include <iostream>
#include "labelingStep.h"
#include "updateStep.h"
#include <chrono>
#include "utils.h"

using namespace Eigen;
using namespace std;
using namespace std::chrono;
namespace tyro 
{
void build_L_Matrix_Vertex(const MatrixXd& F, MatrixXd& L, int numLabels) 
{
	int numFrames = F.cols();
	int numVerticies = F.rows();
	L.resize(numVerticies, numLabels);
	L.setZero();

	int* numbers = nullptr;
	generateRandomNumbers(&numbers, numLabels, numFrames);

	for (int i = 0; i < numLabels; ++i)
	{
		int rndNum = numbers[i];
		L.col(i) = F.col(rndNum);
	}

	delete numbers;
}

void unflatten_frames(const MatrixXd& F, std::vector<Eigen::MatrixXd>& v_data) 
{	
	for (int col = 0; col < F.cols(); ++col) 
	{
		VectorXd V = F.col(col);
		int rows = F.rows()/3;
		int cols = 3;
		MatrixXd A = Map<Matrix<double, Dynamic, Dynamic, RowMajor>>(V.data(), rows, cols);
		v_data.push_back(A);
	}
}

void flatten_frames(const std::vector<Eigen::MatrixXd>& v_data, MatrixXd& F) 
{
	int numFrames= v_data.size();
	int numVerticies = v_data[0].rows() * v_data[0].cols();
	
	F.resize(numVerticies, numFrames);

	int cindex = 0;
	for (auto& V : v_data)
	{
		Matrix<double, Dynamic, Dynamic, RowMajor> M2(V);
		Map<VectorXd> v2(M2.data(), M2.size());
		VectorXd b2 = v2;
		F.col(cindex++) = b2;
	}
}

int stop_motion_vertex_distance(int num_labels, 
                            	const std::vector<Eigen::MatrixXd>& v_data,
                            	const Eigen::MatrixXi& f_data,
								std::vector<Eigen::MatrixXd>& D, //dictionary
								Eigen::VectorXi& S_vec, //labels 
                            	double& result_energy)
{
	MatrixXd F; //,  SAVED_FACES; //frame data
	flatten_frames(v_data, F);
	
	double w_s = 1.0f; //smooth weight
	int num_steps = 10;// 150;
	double tolerance = 0.0001;
	int n_init = 1; // number of times the clustering algorithm will be run

	std::vector<Eigen::MatrixXd> n_init_D;
	std::vector<Eigen::VectorXi> n_init_S;
	std::vector<double> n_init_energy;

	std::cout << "Energie for " << num_labels << " labels\n";

	for (int j = 0; j < n_init; ++j)
	{
		Eigen::MatrixXd D; //label blendshape data
		
#if D_USE_KMEANS_INITALIZATION
		build_L_Matrix_Vertex_Kmeans(F, D, num_labels);
#else
		build_L_Matrix_Vertex(F, D, num_labels);
#endif

		VectorXi S_vec;
		double oldEnergy = 0; 
		double newEnergy = 0;
		double graphEnergy = 0;

		std::cout << "#### Trial " << j << " ####\n";

		for (int i = 0; i < num_steps; ++i)
		{
			std::cout << "Iteration " << i << "\n";

			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			labelFacesTRUEVertex(F, D, S_vec, w_s, graphEnergy);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(t2 - t1).count();
			cout << "label time " << duration << "\n";
			
			t1 = high_resolution_clock::now();
			updateStepTRUEVertex(F, D, S_vec, w_s, oldEnergy, newEnergy);
			t2 = high_resolution_clock::now();
			duration = duration_cast<microseconds>(t2 - t1).count();
			cout << "updte time " << duration << "\n";
			
			std::cout << "Energy after graph cuts " << graphEnergy << "\nEnergy before update step " << oldEnergy << "\nEnergy after update step " << newEnergy << "\n\n";
			double diff = abs(newEnergy - oldEnergy);
			if (oldEnergy >= newEnergy && diff < tolerance)
			{
				std::cout << "Difference diff " << diff << " between old and new energies is lower than tolerance of " << tolerance << "\n\n";
				break;
			}

		}
		n_init_D.push_back(D);
		n_init_S.push_back(S_vec);
		n_init_energy.push_back(oldEnergy);
	}

	//Choose best energy
	std::vector<double>::iterator result = std::min_element(std::begin(n_init_energy), std::end(n_init_energy));
	int min_idx = std::distance(std::begin(n_init_energy), result);

	MatrixXd D_flat = n_init_D.at(min_idx);
	unflatten_frames(D_flat, D);
	//saveDictionary(labelOBJ, D, SAVED_FACES);
	
	S_vec = n_init_S.at(min_idx);
	//saveMatrix(labelPath, S_vec, true);

	result_energy = n_init_energy.at(min_idx);

	return 1;
}
}