#include "stop_motion.h"
#include <algorithm>
#include <iostream>
#include "labelingStep.h"
#include "updateStep.h"
#include <chrono>
#include "utils.h"
#include "kmeans.h"
#include <limits>
#include <opencv2/opencv.hpp>
//#include <opencv2/viz/vizcore.hpp>

//#include "KMeansRexCore.cpp"

using namespace Eigen;
using namespace std;
using namespace std::chrono;

namespace tyro 
{

/*
template<typename _Tp, int _rows, int _cols, int _options, int _maxRows, int _maxCols>
void eigen2cv(const Eigen::Matrix<_Tp, _rows, _cols, _options, _maxRows, _maxCols>& src, cv::Mat& dst)
{
    if (!(src.Flags & Eigen::RowMajorBit))
    {
        cv::Mat _src(src.cols(), src.rows(), cv::DataType<_Tp>::type,
            (void*)src.data(), src.stride() * sizeof(_Tp));
        cv::transpose(_src, dst);
    }
    else
    {
        cv::Mat _src(src.rows(), src.cols(), cv::DataType<_Tp>::type,
            (void*)src.data(), src.stride() * sizeof(_Tp));
        _src.copyTo(dst);
    }
}
*/


/*
void build_dictionary_kmeans(MatrixXd& F, MatrixXd& L, int numLabels)
{	
	
	std::vector<Point> points;
	//KMeans::loadPoints(fpath, &points);
	
	int numFrames = F.cols();
	for (int i = 0; i < numFrames; ++i) 
	{
		VectorXd mat = F.col(i);
		vector<double> vec(mat.data(), mat.data() + mat.size());
		Point p(vec);
		points.push_back(p);
	}
	
	KMeans kmeans(numLabels);
	kmeans.init(points);
	kmeans.run();
	kmeans.printMeans();

	
	int numVerticies= F.rows();
	L.resize(numVerticies, numLabels);
	L.setZero();

	for (int i = 0; i < numLabels; ++i)
	{
		Point cluster = kmeans.means_[i];
		double* cluster_data = cluster.data_.data();
		Map<VectorXd> mapped(cluster_data, cluster.dimensions_);
		L.col(i) = mapped;
	}
}
*/

void build_dictionary_random(const MatrixXd& F, MatrixXd& L, int numLabels) 
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

void flatten_weights(const Eigen::VectorXd& VW_per_vertex, Eigen::VectorXd& VW) 
{
	VW.resize(VW_per_vertex.size() * 3);
	for (int i = 0; i < VW.size(); ++i) 
	{
		VW(i) = VW_per_vertex(i%3);
	}
}

int stop_motion_vertex_distance(int num_labels,
								double w_s,
								bool kmeans,
                            	const std::vector<Eigen::MatrixXd>& v_data,
								Eigen::VectorXd& VW_per_vertex,
								std::vector<int>& sequenceIdx,
                            	const Eigen::MatrixXi& f_data,
								std::vector<Eigen::MatrixXd>& d_data,
								Eigen::VectorXi& s_data,  
                            	double& result_energy)
{
	//double w_s = 2.0f; //smooth weight
	int num_steps = 30;// 150;
	double tolerance = 0.0001;
	int n_init = 5; // number of times the clustering algorithm will be run

	MatrixXd F; //,  SAVED_FACES; //frame data
	flatten_frames(v_data, F);
	VectorXd VW;
	flatten_weights(VW_per_vertex, VW);
	std::vector<Eigen::MatrixXd> n_init_D;
	std::vector<Eigen::VectorXi> n_init_S;
	std::vector<double> n_init_energy;

	std::cout << "Energie for " << num_labels << " labels\n";
	std::cout << "Smooth weight " << w_s << "\n";

//#define D_USE_KMEANS_INITALIZATION 1
	MatrixXd KMEANS;
	if (kmeans) 
	{
		VectorXi lbvec;
		int num_iter = 100;
		tyro::kmeans(F, num_labels, num_iter, KMEANS, lbvec);
	} 

	for (int j = 0; j < n_init; ++j)
	{
		Eigen::MatrixXd D; //label blendshape data

		if (kmeans) 
		{	
			n_init  = 1;
			D = KMEANS;
			std::cout << "Using kmeans initialization" << w_s << "\n";
		}else {
			build_dictionary_random(F, D, num_labels);
			std::cout << "Using random intialization" << w_s << "\n";
		}

		VectorXi S_vec;
		double oldEnergy = 0; 
		double newEnergy = 0;
		double graphEnergy = 0;

		std::cout << "######################################################\n";
		std::cout << "######################## Trial " << j << " #######################\n";
		std::cout << "######################################################\n";

		//VectorXd nrg;
		//nrg.resize(num_steps);
		//nrg.setConstant(std::numeric_limits<double>::max());
		MatrixXd lastD;
		VectorXi lastS_vec;
		double lastEnergy = std::numeric_limits<double>::max();
		for (int i = 0; i < num_steps; ++i)
		{
			std::cout << "Iteration " << i << "\n";

			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			labelFacesTRUEVertex(F, VW, D, S_vec, sequenceIdx, w_s, graphEnergy);
			high_resolution_clock::time_point t2 = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(t2 - t1).count();
			cout << "label time " << duration << "\n";

			if (kmeans) 
			{ 
				lastD = D;
				lastS_vec = S_vec;
				lastEnergy = graphEnergy;
			}
			else 
			{
				//if (graphEnergy > lastEnergy)
			//	{	
			//		std::cout << "Energy after graph cuts " << graphEnergy << " is higher than previous energy " << lastEnergy << "\n\n";
			//		D = lastD;
			//		S_vec = lastS_vec;
			//		break;
			//	}
			//	else 
				{
					lastD = D;
					lastS_vec = S_vec;
					lastEnergy = graphEnergy;
				}
			}

			t1 = high_resolution_clock::now();
			updateStepTRUEVertex(F, VW, D, S_vec, sequenceIdx, w_s, oldEnergy, newEnergy);
			t2 = high_resolution_clock::now();
			duration = duration_cast<milliseconds>(t2 - t1).count();
			cout << "update time " << duration << "\n";
			
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
		n_init_energy.push_back(graphEnergy);
	}

	//Choose best energy
	std::vector<double>::iterator result = std::min_element(std::begin(n_init_energy), std::end(n_init_energy));
	int min_idx = std::distance(std::begin(n_init_energy), result);

	MatrixXd D_flat = n_init_D.at(min_idx);
	unflatten_frames(D_flat, d_data);
	//saveDictionary(labelOBJ, D, SAVED_FACES);
	
	s_data = n_init_S.at(min_idx);
	//VectorXi S_vec = n_init_S.at(min_idx);
	//for (int i = 0; i < S_vec.size(); ++i) 
	//{
	//	s_data.push_back(S_vec(i));
	//}
	//saveMatrix(labelPath, S_vec, true);

	result_energy = n_init_energy.at(min_idx);

	return 1;
}
}